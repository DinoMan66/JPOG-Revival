#!/usr/bin/env python3
"""
trivial.py - do the no-judgment part of the work without AI.

1. TRIVIAL FUNCTIONS: recognises tiny bodies in the original binary (empty, return constant,
   return member, return &member, return this, set member, return global, tail-call thunk) and
   writes ready-to-paste C++ for each one.
2. CLUSTERS: groups not-yet-written functions whose code is identical apart from addresses
   (calls, globals, strings). Write ONE of them and the rest follow the same source. A cluster
   that already contains a matched function is a free win: copy its source.

JPOG (names from the export table + Notes/function_tracker.csv):
  python trivial.py jpog
Zoo Tycoon (no names; addresses from Notes/inventory.csv, written functions from Notes/zoo.map):
  python trivial.py zoo

Output (in Notes/generated/):
  trivial_<module>.cpp   C++ for every trivial function still to do, grouped by class
  clusters.md            identical-code groups, biggest payoff first
  summary.txt            the counts
The C++ uses placeholder member names (m_unk08 = offset 0x8). Rename them to the real members from
Src/include when pasting; the offset tells you which one. Generated code still has to go through the
build + verdict.py like anything else - it is a first draft, not a guaranteed match.
"""
import argparse, collections, csv, glob, hashlib, os, re, struct, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import triage_lib as T

# ------------------------------------------------------------------ PE + whole-section disassembly
class Image:
    def __init__(self, path):
        self.path = path
        d = self.d = open(path, "rb").read()
        pe = struct.unpack_from("<I", d, 0x3c)[0]
        nsec = struct.unpack_from("<H", d, pe + 6)[0]
        optsz = struct.unpack_from("<H", d, pe + 20)[0]
        self.base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
        self.size = struct.unpack_from("<I", d, pe + 24 + 56)[0]
        so = pe + 24 + optsz
        self.secs = []
        for i in range(nsec):
            name, vs, va, rs, rp = struct.unpack_from("<8sIIII", d, so + 40 * i)
            ch = struct.unpack_from("<I", d, so + 40 * i + 36)[0]
            self.secs.append((va, vs, rs, rp, ch))
        self.exports = {}
        erva = struct.unpack_from("<I", d, pe + 24 + 96)[0]
        if erva:
            eo = self.r2o(erva)
            nn, af, an, ao = struct.unpack_from("<IIII", d, eo + 24)
            for i in range(nn):
                n = struct.unpack_from("<I", d, self.r2o(an) + 4 * i)[0]
                s = d[self.r2o(n):self.r2o(n) + 600].split(b"\0")[0].decode(errors="replace")
                od = struct.unpack_from("<H", d, self.r2o(ao) + 2 * i)[0]
                self.exports[s] = struct.unpack_from("<I", d, self.r2o(af) + 4 * od)[0]
        self.by_rva = {}
        for k, v in self.exports.items():
            self.by_rva.setdefault(v, k)
        self.ins = {}
        for va, vs, rs, rp, ch in self.secs:
            if ch & 0x20000000:                         # code section: one linear sweep
                for a, b, t in T.disasm(d[rp:rp + min(vs, rs)], self.base + va):
                    self.ins[a] = (b, t)
    def r2o(self, rva):
        for va, vs, rs, rp, ch in self.secs:
            if va <= rva < va + max(vs, rs):
                return rva - va + rp
        return None
    def in_image(self, v):
        return self.base <= v < self.base + self.size

# ------------------------------------------------------------------ control-flow walk (layout independent)
JCC = re.compile(r"^(j[a-z]+|loop\w*)\s+0x([0-9a-f]+)")
def walk(img, entry, limit=400):
    """Instructions of a function in a canonical order (fall-through first), following jmps,
    so BBT-moved blocks compare equal. Returns (list of (addr, bytes, text), ok)."""
    out, seen, work = [], set(), [entry]
    while work and len(out) < limit:
        a = work.pop()
        while a not in seen and len(out) < limit:
            x = img.ins.get(a)
            if x is None:
                return out, False
            b, t = x
            m = re.match(r"^jmp\s+0x([0-9a-f]+)$", t)
            if m and b[0] in (0xEB, 0xE9):              # direct jmp: transparent
                seen.add(a); a = int(m.group(1), 16); continue
            seen.add(a); out.append((a, b, t))
            mn = t.split(" ")[0]
            if mn in ("ret", "retn") or mn == "jmp" or mn == "int3":
                break
            m = JCC.match(t)
            if m and mn != "jmp":
                work.append(int(m.group(2), 16))
            a += len(b)
    return out, len(out) < limit

def fingerprint(img, insns):
    """Hash of the code with addresses masked (call targets, globals, strings, branch targets)."""
    toks = []
    for a, b, t in insns:
        s = t
        s = re.sub(r"^(call|j[a-z]+|loop\w*)\s+0x[0-9a-f]+", r"\1 L", s)
        def mask(m):
            v = int(m.group(0), 16)
            return "A" if img.in_image(v) else m.group(0)
        s = re.sub(r"0x[0-9a-f]{6,8}", mask, s)
        toks.append(s)
    return hashlib.sha1("\n".join(toks).encode()).hexdigest()[:16], len(toks)

# ------------------------------------------------------------------ trivial-body recogniser
def mem_off(op, reg):
    m = re.fullmatch(r"(?:(byte|word|dword|qword) (?:ptr )?)?\[" + reg + r"(?:\+0x([0-9a-f]+))?\]", op.strip())
    if not m: return None
    return (m.group(1) or "", int(m.group(2) or "0", 16))

TYPE_OF = {"byte": "char", "word": "short", "dword": "int", "qword": "double"}
def recognise(img, insns):
    """-> (kind, data) or None. insns: canonical instructions of the function."""
    if not insns or len(insns) > 4:
        return None
    texts = [T.norm(t) for _, _, t in insns]
    last = texts[-1]
    m = re.match(r"^(ret|retn)(?: 0x([0-9a-f]+))?$", last)
    tail = re.match(r"^jmp 0x([0-9a-f]+)$", last)
    if not m and not (tail and len(texts) == 1):
        return None
    if tail:
        tgt = int(tail.group(1), 16)
        return ("thunk", img.by_rva.get(tgt - img.base) or f"sub_{tgt:x}")
    body = texts[:-1]
    if not body:
        return ("empty", None)
    if len(body) == 1:
        s = body[0]
        if s in ("mov al,0x1",): return ("const", "true")
        if s in ("xor al,al",): return ("const", "false")
        if s in ("xor eax,eax", "sub eax,eax"): return ("const", "0")
        if s in ("or eax,0xffffffff",): return ("const", "-1")
        if s == "fldz": return ("const", "0.0f")
        if s == "fld1": return ("const", "1.0f")
        if s == "mov eax,ecx": return ("this", None)
        mm = re.match(r"^mov eax,0x([0-9a-f]+)$", s)
        if mm:
            v = int(mm.group(1), 16)
            if img.in_image(v):
                return ("gaddr", v)
            return ("const", str(v) if v < 0x10000 else f"{v:#x}")
        mm = re.match(r"^(?:mov|movzx|movsx) (eax|ax|al),(.*)$", s)
        if mm:
            o = mem_off(mm.group(2), "ecx")
            if o: return ("get", (o[0] or {"eax": "dword", "ax": "word", "al": "byte"}[mm.group(1)], o[1]))
            g = re.fullmatch(r"(?:(byte|word|dword) )?\[?(?:ds:)?0x([0-9a-f]+)\]?", mm.group(2).strip())
            if g and img.in_image(int(g.group(2), 16)):
                return ("global", int(g.group(2), 16))
        mm = re.match(r"^fld (dword|qword) \[ecx(?:\+0x([0-9a-f]+))?\]$", s)
        if mm: return ("get", ({"dword": "float", "qword": "double"}[mm.group(1)], int(mm.group(2) or "0", 16)))
        mm = re.match(r"^lea eax,\[ecx\+0x([0-9a-f]+)\]$", s)
        if mm: return ("addr", int(mm.group(1), 16))
        mm = re.match(r"^mov (byte|word|dword) \[ecx(?:\+0x([0-9a-f]+))?\],0x([0-9a-f]+)$", s)
        if mm: return ("setconst", (mm.group(1), int(mm.group(2) or "0", 16), int(mm.group(3), 16)))
    if len(body) == 2:
        mm = re.match(r"^(?:mov|movzx) (eax|ax|al),(?:(?:byte|word|dword) )?\[esp\+0x4\]$", body[0])
        m2 = re.match(r"^mov (?:(byte|word|dword) )?\[ecx(?:\+0x([0-9a-f]+))?\],(eax|ax|al)$", body[1])
        if mm and m2:
            w = m2.group(1) or {"eax": "dword", "ax": "word", "al": "byte"}[m2.group(3)]
            return ("set", (w, int(m2.group(2) or "0", 16)))
        mm = re.match(r"^fld dword \[esp\+0x4\]$", body[0])
        m2 = re.match(r"^fstp dword \[ecx(?:\+0x([0-9a-f]+))?\]$", body[1])
        if mm and m2:
            return ("set", ("float", int(m2.group(1) or "0", 16)))
    return None

# ------------------------------------------------------------------ C++ emission
def split_params(p):
    out, depth, cur = [], 0, ""
    for ch in p:
        if ch in "<(": depth += 1
        if ch in ">)": depth -= 1
        if ch == "," and depth == 0:
            out.append(cur.strip()); cur = ""
        else:
            cur += ch
    if cur.strip(): out.append(cur.strip())
    return [] if out == ["void"] else out

def signature(decorated):
    """'bool Toshi::TFoo::IsX(int a1) const' from the decorated name, or None."""
    d = T.demangle(decorated)
    if not d or "(" not in d:
        return None
    d = re.sub(r"^(public|protected|private): ", "", d)
    d = re.sub(r"\b(virtual|static) ", "", d)
    d = re.sub(r"\b__(thiscall|stdcall|cdecl|fastcall) ", "", d)
    d = d.replace("class ", "").replace("struct ", "").replace("enum ", "")
    head, rest = d.split("(", 1)
    params, tail = rest.rsplit(")", 1)
    ps = split_params(params)
    named = ", ".join(f"{p} a{i + 1}" for i, p in enumerate(ps))
    return f"{head.strip()}({named}){tail}", head.strip(), ps

def class_of(name):
    m = re.match(r"\?\?(?:_[0-9A-Z]|[0-9A-Z])([A-Za-z_]\w*)@", name)      # operators, ctors, dtors
    if m: return m.group(1)
    p = T.pretty(name)
    return p.split("::")[0] if "::" in p else "(free functions)"

def field(off):
    return f"m_unk{off:02X}"

def body_for(kind, data, ret_type, name=""):
    ref = bool(ret_type) and ret_type.rstrip().endswith("&")
    if kind == "this" and name.startswith("??0"):
        return "{}"                                    # constructor with nothing to do
    if kind == "empty":
        return "{}"
    if kind == "const":
        v = data
        if ret_type and ret_type.startswith("bool") and v in ("0", "1", "-1"): v = "false" if v == "0" else "true"
        if ret_type and "*" in ret_type and v == "0": v = "NULL"
        return "{ return " + v + "; }"
    if kind == "this":
        return ("{ return *this; }" if ref else "{ return this; }") + "   // or the member / base class at offset 0 (e.g. a char[] name)"
    if kind == "get":
        return "{ return " + ("*" if ref else "") + field(data[1]) + "; }" + f"   // +{data[1]:#x} ({data[0]})"
    if kind == "addr":
        return "{ return &" + field(data) + "; }" + f"   // +{data:#x} (or a base-class/embedded-object cast)"
    if kind == "set":
        return "{ " + field(data[1]) + " = a1; }" + f"   // +{data[1]:#x} ({data[0]})"
    if kind == "setconst":
        return "{ " + field(data[1]) + f" = {data[2]:#x};" + " }" + f"   // +{data[1]:#x} ({data[0]})"
    if kind == "gaddr":
        return "{ return &" + f"g_{data:x}" + "; }" + f"   // address of a global/static at {data:#x} (maybe *staticPtr)"
    if kind == "global":
        return "{ return " + f"g_{data:x}" + "; }" + f"   // global at {data:#x}"
    if kind == "thunk":
        return "{ /* tail call */ return " + T.pretty(data) + "(...); }" + f"   // {data}"
    return "{ /* ? */ }"

# ------------------------------------------------------------------ project front ends
def jpog_functions(a):
    tracker = os.path.join(ROOT, "Notes", "function_tracker.csv")
    rows = list(csv.DictReader(open(tracker, newline="", encoding="utf-8", errors="replace")))
    dllpaths = {}
    for p in glob.glob(os.path.join(ROOT, "Original", "**", "*.dll"), recursive=True) + \
             glob.glob(os.path.join(ROOT, "Original", "*.exe")):           # SimJP.exe (tracker row 'SimJP.exe')
        b = os.path.basename(p).replace("_inst", "")
        if b not in dllpaths or "_inst" in p:
            dllpaths[b] = p
    T.demangle_many([r["decorated_name"] for r in rows if r["status"] == "todo"])
    for dll in sorted({r["dll"] for r in rows}):
        if a.only and not any(o.lower() in dll.lower() for o in a.only):
            continue
        if dll not in dllpaths:
            print(f"  {dll}: original not found under Original/ - skipped"); continue
        img = Image(dllpaths[dll])
        for r in rows:
            if r["dll"] == dll:
                yield img, dll, img.base + int(r["rva"], 16), r["decorated_name"], r["status"]

def zoo_functions(a):
    img = Image(os.path.join(ROOT, "Original", "zoo.exe"))
    written = {}
    mp = os.path.join(ROOT, "Notes", "zoo.map")
    if os.path.exists(mp):
        for ln in open(mp, errors="replace"):
            ln = ln.split("#", 1)[0].strip()
            if "=" in ln:
                k, v = ln.rsplit("=", 1); v = int(v.strip(), 0)
                written[v if v >= img.base else v + img.base] = k.strip()
    st = {}
    sp = os.path.join(ROOT, "Notes", "attempts.csv")
    if os.path.exists(sp):
        for r in csv.DictReader(open(sp, newline="", encoding="utf-8")):
            st[int(r["addr"], 16)] = r["status"]
    for r in csv.DictReader(open(os.path.join(ROOT, "Notes", "inventory.csv"), newline="")):
        va = int(r["va"], 16)
        name = written.get(va, f"FUN_{va:08x}")
        status = st.get(va, "written" if va in written else "todo")
        yield img, "zoo.exe", va, name, status

# ------------------------------------------------------------------ main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("project", choices=["jpog", "zoo"])
    ap.add_argument("--only", action="append", default=[], help="JPOG: only DLLs whose name contains this")
    ap.add_argument("--out", default=os.path.join(ROOT, "Notes", "generated"))
    ap.add_argument("--min-cluster-bytes", type=int, default=12)
    a = ap.parse_args()
    os.makedirs(a.out, exist_ok=True)
    gen = jpog_functions(a) if a.project == "jpog" else zoo_functions(a)

    trivial = collections.defaultdict(list)          # module -> [(cls, name, va, sig, body)]
    clusters = collections.defaultdict(list)         # fp -> [(module, va, name, status, n)]
    kinds = collections.Counter()
    todo_total = 0
    for img, module, va, name, status in gen:
        insns, ok = walk(img, va)
        if not insns:
            continue
        is_todo = status in ("todo",)
        todo_total += is_todo
        rec = recognise(img, insns) if ok else None
        if rec and is_todo:
            kinds[rec[0]] += 1
            sig = signature(name) if name.startswith("?") else None
            ret_type = None
            if sig:
                full, head, ps = sig
                ret_type = head.rsplit(" ", 1)[0] if " " in head else ""
                if rec[0] == "set" and len(ps) != 1:
                    rec = None
            if rec:
                cls = class_of(name)
                decl = sig[0] if sig else (f"{name}()" if not name.startswith("?") else T.pretty(name) + "(...)")
                trivial[module].append((cls, name, va, decl, body_for(rec[0], rec[1], ret_type, name)))
                continue
        nbytes = sum(len(b) for _, b, _ in insns)
        if ok and nbytes >= a.min_cluster_bytes:
            fp, n = fingerprint(img, insns)
            clusters[fp].append((module, va, name, status, nbytes))

    # ---- trivial_<module>.cpp
    total_triv = 0
    for module, items in trivial.items():
        total_triv += len(items)
        path = os.path.join(a.out, f"trivial_{module.replace('.dll', '').replace('.exe', '')}.cpp")
        L = [f"// Generated by Tools/trivial.py - {len(items)} trivial functions still to do in {module}.",
             "// Paste each block into the .cpp that owns the class, rename m_unkXX to the real member",
             "// (the comment gives the offset), then build + verdict.py as usual.", ""]
        by_cls = collections.defaultdict(list)
        for it in items: by_cls[it[0]].append(it)
        for cls in sorted(by_cls):
            L.append(f"// ---- {cls}")
            for _, name, va, decl, body in sorted(by_cls[cls], key=lambda x: x[2]):
                L.append(f"// {va:#x}  {name}")
                L.append(f"{decl} {body}")
            L.append("")
        open(path, "w", encoding="utf-8").write("\n".join(L) + "\n")

    # ---- clusters.md
    groups = [g for g in clusters.values() if len(g) >= 2 and any(x[3] == "todo" for x in g)]
    def payoff(g):
        todo = [x for x in g if x[3] == "todo"]
        done = any(x[3] in ("matched", "written") for x in g)
        return (done, (len(todo) - (0 if done else 1)) * todo[0][4])
    groups.sort(key=payoff, reverse=True)
    free = [g for g in groups if any(x[3] in ("matched", "written") for x in g)]
    saved = sum(len([x for x in g if x[3] == "todo"]) - (0 if g in free else 1) for g in groups)
    L = ["# Identical-code clusters (generated by Tools/trivial.py)", "",
         "Functions in one cluster have the same code apart from addresses (call targets, globals, strings).",
         "Write one, then reuse the source for the rest (usually the same template / macro / inline helper",
         "with different names or constants).", "",
         f"- clusters with at least one function still to do: {len(groups)}",
         f"- **free wins** (a member is already matched/written - copy its source): {len(free)} clusters, "
         f"{sum(len([x for x in g if x[3] == 'todo']) for g in free)} functions",
         f"- functions that need no separate work: {saved}", ""]
    for i, g in enumerate(groups[:300]):
        todo = [x for x in g if x[3] == "todo"]
        done = [x for x in g if x[3] in ("matched", "written")]
        tag = "FREE WIN - copy from " + T.pretty(done[0][2]) if done else "write one, reuse for the rest"
        L.append(f"## {i + 1}. {len(g)} functions, {g[0][4]} bytes each - {tag}")
        for x in sorted(g, key=lambda x: (x[3] != "matched", x[0], x[1]))[:25]:
            L.append(f"- `{x[0]}` {x[1]:#x} {T.pretty(x[2])} [{x[3]}]")
        if len(g) > 25: L.append(f"- ... and {len(g) - 25} more")
        L.append("")
    if len(groups) > 300: L.append(f"(and {len(groups) - 300} smaller clusters)")
    open(os.path.join(a.out, "clusters.md"), "w", encoding="utf-8").write("\n".join(L) + "\n")

    summ = [f"functions still to do: {todo_total}",
            f"trivial (C++ generated): {total_triv}  " + ", ".join(f"{k} {v}" for k, v in kinds.most_common()),
            f"identical-code clusters: {len(groups)} ({len(free)} free wins); functions covered by another cluster member: {saved}",
            f"left for real work: about {todo_total - total_triv - saved}"]
    open(os.path.join(a.out, "summary.txt"), "w").write("\n".join(summ) + "\n")
    print("\n".join(summ))
    print(f"written to {os.path.relpath(a.out, ROOT)}/")

if __name__ == "__main__":
    main()
