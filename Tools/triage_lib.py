#!/usr/bin/env python3
"""
triage_lib.py - shared helpers for verdict.py and trivial.py (same file in both projects).

  disasm(code, addr)        -> [(addr, bytes, text)]  (capstone if installed, else GNU objdump)
  classify(ours, orig)      -> (category, short explanation)   for the first differing instruction
  demangle(name)            -> readable C++ signature (llvm-undname / undname if available)
  pretty(name)              -> Class::method short form (no external tool needed)

Categories returned by classify():
  encoding    same instruction, different encoding ([esp] vs [esp+0])           -> compiler-limited
  regalloc    same instruction shape, different registers                      -> usually compiler-limited
  sched       the same instructions in a different order                       -> usually compiler-limited
  shrink-wrap callee-saved register saved in a different place                  -> usually compiler-limited
  inline      the original keeps a call where ours inlined the code             -> compiler-limited (JPOG #23)
  fp          x87 constant / operand-size / fadd-vs-fmul differences            -> usually compiler-limited
  outlined    ours calls something the original inlined (missing `inline`?)     -> SOURCE problem
  stack       same instruction on another stack slot (local declaration order)  -> SOURCE problem
  branch      inverted condition at the same place (if/else body order)         -> SOURCE problem
  source      anything else (different logic, branches, constants, sizes)       -> SOURCE problem
"""
import os, re, shutil, subprocess, tempfile

LIMITED_KINDS = {"encoding", "regalloc", "sched", "shrink-wrap", "inline", "fp"}
REGS = ("eax ebx ecx edx esi edi ebp esp ax bx cx dx si di bp sp al ah bl bh cl ch dl dh").split()
REG_RE = re.compile(r"\b(" + "|".join(REGS) + r")\b")
CALLEE_SAVED = {"ebx", "esi", "edi", "ebp"}

# ------------------------------------------------------------------ disassembly
_cs = None
try:
    import capstone as _capstone
    _cs = _capstone.Cs(_capstone.CS_ARCH_X86, _capstone.CS_MODE_32)
except Exception:
    _cs = None

def disasm(code, addr=0):
    """Linear sweep. Returns [(addr, bytes, text)]; text is lower-case Intel syntax."""
    if not code:
        return []
    if _cs is not None:
        return [(i.address, bytes(i.bytes), f"{i.mnemonic} {i.op_str}".strip().lower())
                for i in _cs.disasm(code, addr)]
    if not shutil.which("objdump"):
        raise SystemExit("need either 'pip install capstone' or GNU objdump")
    fd, tmp = tempfile.mkstemp(suffix=".bin")
    os.write(fd, code); os.close(fd)
    try:
        txt = subprocess.run(["objdump", "-D", "-b", "binary", "-m", "i386", "-M", "intel", "-w",
                              f"--adjust-vma={addr:#x}", tmp], capture_output=True, text=True).stdout
    finally:
        os.unlink(tmp)
    out = []
    for ln in txt.splitlines():
        m = re.match(r"^\s*([0-9a-f]+):\t((?:[0-9a-f]{2} )+)\s*\t?(.*)$", ln)
        if m:
            b = bytes(int(x, 16) for x in m.group(2).split())
            out.append((int(m.group(1), 16), b, m.group(3).strip().lower()))
    return out

def norm(text):
    """Normalise away pure encoding differences and objdump/capstone spelling differences."""
    t = text.lower()
    t = re.sub(r"\s+", " ", t)
    t = re.sub(r"\+0x0\]", "]", t)
    t = t.replace(" ptr ", " ").replace("ds:", "")
    t = re.sub(r"(byte|word|dword|qword|tbyte) ", r"\1 ", t)
    return t.strip()

def skeleton(text):
    """Instruction with registers replaced by R and numbers by N (for 'same shape' tests)."""
    t = REG_RE.sub("R", norm(text))
    return re.sub(r"0x[0-9a-f]+|\b\d+\b", "N", t)

def regs_of(text):
    return set(REG_RE.findall(norm(text)))

def mnem(text):
    return norm(text).split(" ")[0] if text else ""

# ------------------------------------------------------------------ classification
def classify(ours, orig, index_in_func=0):
    """ours / orig: lists of (addr, bytes, text) starting AT the first differing instruction.
    index_in_func: how many instructions precede it in the function (prologue detection)."""
    if not ours or not orig:
        return "source", "one side ends early"
    o, g = ours[0][2], orig[0][2]
    if norm(o) == norm(g):
        return "encoding", f"same instruction, other encoding ({ours[0][1].hex(' ')} vs {orig[0][1].hex(' ')})"
    mo, mg = mnem(o), mnem(g)
    # inlining: the original calls, we do not (or the other way round)
    if mg == "call" and mo != "call" and not any(mnem(t) == "call" for _, _, t in ours[:2]):
        return "inline", "the original keeps a call here; ours inlined it"
    if mo == "call" and mg != "call" and not any(mnem(t) == "call" for _, _, t in orig[:2]):
        return "outlined", "ours calls here; the original has the code inline (missing inline?)"
    # mov bx,[m] vs movzx ebx,[m]: widening choice of the register allocator
    if {mo, mg} <= {"mov", "movzx", "movsx"} and mo != mg and o.split(",")[-1].strip() == g.split(",")[-1].strip():
        return "regalloc", "mov vs movzx of the same value (register width choice)"
    # same instruction against another stack slot: locals laid out differently
    so_, sg_ = re.sub(r"\[esp(\+0x[0-9a-f]+)?\]", "[esp+S]", norm(o)), re.sub(r"\[esp(\+0x[0-9a-f]+)?\]", "[esp+S]", norm(g))
    if so_ == sg_ and "[esp" in so_:
        return "stack", "same instruction, other stack slot - try reordering local declarations (slots follow declaration order)"
    if mo == mg == "sub" and "esp" in o and "esp" in g:
        return "stack", "different stack frame size - a local too many/few, or a temporary the original keeps"
    # inverted conditional branch at the same place: block order
    if mo.startswith("j") and mg.startswith("j") and mo != "jmp" and mg != "jmp":
        return "branch", "condition inverted - swap the if/else bodies (write the fall-through path first)"
    # register choice: same shape, other registers
    if skeleton(o) == skeleton(g):
        if mo == "push" and mg == "push":
            return "regalloc", "different callee-saved register pushed"
        return "regalloc", "same instruction, different registers"
    # shrink-wrap / prologue differences: a callee-saved push on one side only
    def is_cs_push(t):
        return mnem(t) == "push" and regs_of(t) & CALLEE_SAVED and len(regs_of(t)) == 1
    if (is_cs_push(o) != is_cs_push(g)) and index_in_func <= 4:
        return "shrink-wrap", "callee-saved register saved in a different place"
    # scheduling: same multiset of instruction shapes in the next few instructions
    for k in (3, 4, 5, 6):
        so = sorted(skeleton(t) for _, _, t in ours[:k])
        sg = sorted(skeleton(t) for _, _, t in orig[:k])
        if len(so) == k and so == sg:
            return "sched", f"same {k} instructions, different order/registers"
    if mo.startswith("f") and mg.startswith("f"):
        return "fp", "x87 difference (constant size / fadd vs fmul / spill)"
    if is_cs_push(o) != is_cs_push(g):
        return "shrink-wrap", "callee-saved register saved in a different place"
    return "source", "different code - needs a source change"

# ------------------------------------------------------------------ names
def pretty(name):
    """Readable short form of a decorated name: Class::function."""
    for pre, what in (("??0", "ctor"), ("??1", "dtor"), ("??4", "operator="), ("??_E", "vector-deleting-dtor"),
                      ("??_G", "scalar-deleting-dtor")):
        m = re.match(re.escape(pre) + r"([A-Za-z_]\w*)@", name)
        if m: return m.group(1) + "::" + what
    if name.startswith("??2"): return "operator new"
    if name.startswith("??3"): return "operator delete"
    m = re.match(r"\?([A-Za-z_]\w*)@([A-Za-z_]\w*)@", name)
    if m: return m.group(2) + "::" + m.group(1)
    m = re.match(r"\?([A-Za-z_]\w*)@@", name)
    if m: return m.group(1)
    return name

_undname = shutil.which("llvm-undname") or shutil.which("undname")
_dm_cache = {}
def demangle(name):
    """Full C++ signature, or None when no demangler is installed (llvm-undname on Linux,
    undname.exe from Visual Studio on Windows)."""
    if not _undname or not name.startswith("?"):
        return None
    if name not in _dm_cache:
        try:
            out = subprocess.run([_undname, name], capture_output=True, text=True, timeout=10).stdout
        except Exception:
            out = ""
        res = None
        for ln in out.splitlines():
            ln = ln.strip()
            if ln.startswith("is :-"):                      # MS undname: is :- "..."
                res = ln.split('"', 1)[1].rsplit('"', 1)[0]
            elif ln and ln != name and not ln.startswith(("Microsoft", "Undecoration", "Copyright")):
                res = ln
        _dm_cache[name] = res
    return _dm_cache[name]

def demangle_many(names):
    """Batch version (one process for many names) - much faster for thousands of names."""
    names = [n for n in names if n.startswith("?") and n not in _dm_cache]
    if not names or not _undname or "llvm" not in os.path.basename(_undname):
        for n in names: demangle(n)
        return
    out = subprocess.run([_undname], input="\n".join(names) + "\n", capture_output=True, text=True).stdout
    lines = [l for l in out.splitlines()]
    # llvm-undname echoes each input line followed by its result
    i = 0
    while i < len(lines) - 1:
        if lines[i] in names:
            _dm_cache[lines[i]] = lines[i + 1].strip() or None; i += 2
        else:
            i += 1
