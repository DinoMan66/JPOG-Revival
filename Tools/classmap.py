#!/usr/bin/env python3
"""
classmap.py - recover classes from JPOG binaries: vtables, slot names, constructors/destructors, base classes.
Ported from ZooTycoonDecompiled\\Tools\\classmap.py and adapted to the Toshi DLLs (exports, import thunks).

Usage:
  python classmap.py Original/DLLS/TRenderInterface_inst.dll [more binaries ...] --out Notes/classes.csv [--md Notes/classes.md]
  python classmap.py Original/DLLS/D3D/*.dll Original/DLLS/TRenderD3DInterface_inst.dll --slots TSkinShader

vtables   : exported ??_7 vtables, plus tables a function stores into an object ("mov dword ptr [reg+off], imm"
            where imm points at a run of code pointers) - the second kind finds unexported classes (SimJP.exe).
slot names: the exported (decorated) name of each slot function; import thunks resolve to dll!name.
writers   : functions storing the vptr (constructors and destructors), with their exported names.
class name: 1) the ??_7 export; 2) an exported ??0/??1 writer; 3) majority vote of 'Class::method' strings that
            the writers and slot functions reference (SimJP.exe has 147 such strings and no exports).
base      : a writer that calls another class's constructor (??0Base, direct or through an import) -> Base.
status    : per slot from Notes/function_tracker.csv (exported functions only).
--slots X : print the slot table of every vtable whose class or base class contains X. Use it to name the
            `_SlotNN` placeholders: slot N of a derived class's vtable is the same virtual function as slot N
            of its base (same index), and the D3D DLLs export the derived implementations by name.
Needs GNU objdump (same as dis.py).
"""
import argparse, collections, csv, os, re, struct, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import compare as C, dis as DI

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
INS = re.compile(r"\s*([0-9a-f]+):\s*(.*)")


def undname(n):
    """short readable form of a decorated name: Class::method (no arguments)"""
    m = re.match(r"\?(\?_[0-9A-Z]|\?[0-9A-Z])?([^@]*)@((?:[^@]+@)*)@", n)
    if not m:
        return n
    scopes = [s for s in m.group(3).split("@") if s and s != "Toshi"]
    op = m.group(1)
    if op == "?0": fn = scopes[0] if scopes else m.group(2)
    elif op == "?1": fn = "~" + (scopes[0] if scopes else m.group(2))
    elif op == "?_7": return "vftable " + "::".join(reversed([m.group(2)] + scopes))
    elif op: fn = "operator" + op
    else: fn = m.group(2)
    if op in ("?0", "?1"): scopes = [m.group(2)] + scopes
    return "::".join(list(reversed([s for s in scopes if s]))[-2:] + [fn]) if scopes else fn


def ctor_class(n):
    m = re.match(r"\?\?[01]([^@]+)@", n or "")
    return m.group(1) if m else None


class Binary:
    def __init__(self, path):
        self.path = path
        self.d = open(path, "rb").read()
        self.base, self.secs, self.r2o, self.imp = DI.imports(self.d)
        _, _, exports, _ = C.load_dll(path) if self._has_exports() else (None, None, {}, None)
        self.names = {self.base + r: n for n, r in exports.items()}
        self.text = [(self.base + va, self.base + va + max(vs, rs)) for nm, vs, va, rs, ra in self.secs
                     if nm.rstrip(b"\0") in (b".text", b"CODE") or nm.startswith(b".text")]
        self.dll = os.path.basename(path).replace("_inst", "")
        self._disasm()

    def _has_exports(self):
        pe = struct.unpack_from("<I", self.d, 0x3c)[0]
        return struct.unpack_from("<I", self.d, pe + 0x78)[0] != 0

    def in_text(self, v): return any(s <= v < e for s, e in self.text)

    def u32(self, v):
        o = self.r2o(v - self.base)
        return struct.unpack_from("<I", self.d, o)[0] if o is not None and o + 4 <= len(self.d) else None

    def string(self, v):
        o = self.r2o(v - self.base)
        if o is None: return None
        s = self.d[o:o + 96].split(b"\0")[0]
        return s.decode("latin1") if len(s) >= 4 and all(32 <= c < 127 for c in s) else None

    def thunk(self, v):
        """import name if v is 'jmp dword ptr [IAT]'"""
        o = self.r2o(v - self.base)
        if o is not None and self.d[o:o + 2] == b"\xff\x25":
            return self.imp.get(struct.unpack_from("<I", self.d, o + 2)[0])
        return None

    def name(self, v):
        return self.names.get(v) or (("-> " + self.thunk(v)) if self.thunk(v) else None)

    def _disasm(self):
        txt = subprocess.run(["objdump", "-d", "-M", "intel", "--no-show-raw-insn", self.path],
                             capture_output=True, text=True).stdout
        self.ins = []
        for ln in txt.splitlines():
            m = INS.match(ln)
            if m: self.ins.append((int(m.group(1), 16), m.group(2)))


def analyse(b):
    starts = set(v for v in b.names if b.in_text(v))
    stores = collections.defaultdict(set)          # vtable -> instruction addresses storing it
    calls, strs = [], []
    for a, s in b.ins:
        m = re.match(r"mov\s+DWORD PTR \[e\w\w(?:[+-]0x[0-9a-f]+)?\],0x([0-9a-f]+)$", s)
        if m:
            imm = int(m.group(1), 16)
            if not b.in_text(imm) and b.in_text(b.u32(imm) or 0):
                stores[imm].add(a)
        m = re.match(r"call\s+(?:0x)?([0-9a-f]+)\b", s)
        if m:
            t = int(m.group(1), 16); calls.append((a, t)); starts.add(t)
        m = re.match(r"call\s+DWORD PTR ds:0x([0-9a-f]+)", s)
        if m:
            calls.append((a, ("imp", int(m.group(1), 16))))
        for h in re.findall(r"0x([0-9a-f]{6,8})", s):
            st = b.string(int(h, 16))
            if st: strs.append((a, st))
    vts = set(stores) | {v for v, n in b.names.items() if n.startswith("??_7")}
    slots = {}
    for t in sorted(vts):
        out, v = [], t
        while True:
            f = b.u32(v)
            if f is None or not b.in_text(f): break
            if v != t and (v in vts): break
            out.append(f); starts.add(f); v += 4
        slots[t] = out
    order = sorted(starts)
    import bisect
    def owner(a):
        i = bisect.bisect_right(order, a) - 1
        return order[i] if i >= 0 else None
    writers = {t: sorted({owner(a) for a in stores.get(t, ())} - {None}) for t in slots}
    fcalls = collections.defaultdict(list)            # function -> call targets in address order
    for a, t in calls:
        if t not in fcalls[owner(a)]: fcalls[owner(a)].append(t)
    writer_of = {w: t for t in slots for w in writers[t]}
    fstr = collections.defaultdict(collections.Counter)
    for a, st in strs:
        m = re.match(r"(?:Toshi::)?([A-Z][A-Za-z0-9_]+)::~?[A-Za-z_]", st)
        if m: fstr[owner(a)][m.group(1)] += 1
    rows = []
    for t, sl in slots.items():
        nm = b.names.get(t)
        cls = None
        if nm and nm.startswith("??_7"):
            cls = undname(nm).replace("vftable ", "")
        for w in writers[t]:
            c = ctor_class(b.names.get(w))
            if not cls and c: cls = c
        if not cls:
            vote = collections.Counter()
            for f in sl + writers[t]: vote.update(fstr.get(f, {}))
            if vote: cls = vote.most_common(1)[0][0] + " (string vote)"
        # base: the first constructor a vptr writer calls - a local function writing another vtable, or an
        # imported/exported ??0Base (members are constructed after the base, so the first one wins)
        base = None
        for w in writers[t]:
            for tgt in fcalls.get(w, ()):
                if not isinstance(tgt, tuple) and tgt in writer_of and writer_of[tgt] != t:
                    base = ("vtable %#x" % (writer_of[tgt] - b.base)); break
                n = (b.imp.get(tgt[1]) if isinstance(tgt, tuple) else b.name(tgt)) or ""
                c = ctor_class(n.split("!")[-1])
                if c and (not cls or c != cls.split(" ")[0]): base = c; break
            if base: break
        rows.append(dict(binary=b.dll, vtable=f"{t - b.base:#x}", cls=cls or "", base=base or "",
                         writers=" ".join(f"{w - b.base:#x}" + (f"={undname(b.names[w])}" if w in b.names else "")
                                          for w in writers[t]),
                         slots=[(f - b.base, b.name(f)) for f in sl]))
    byvt = {r["vtable"]: r for r in rows}
    for r in rows:
        if r["base"].startswith("vtable "):
            br = byvt.get(r["base"][7:])
            if br:
                if not br["cls"]: br["cls"] = "(unexported base of %s)" % (r["cls"] or r["vtable"])
                r["base"] = br["cls"] if not br["cls"].startswith("(") else "vtable " + br["vtable"]
    return rows


def tracker():
    st = {}
    p = os.path.join(ROOT, "Notes", "function_tracker.csv")
    if os.path.exists(p):
        for r in csv.DictReader(open(p, newline="", encoding="utf-8", errors="replace")):
            st[(r["dll"].lower(), int(r["rva"], 16))] = r["status"].strip()
    return st


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("binaries", nargs="+")
    ap.add_argument("--out"); ap.add_argument("--md"); ap.add_argument("--slots")
    a = ap.parse_args()
    st = tracker()
    rows = []
    for p in a.binaries:
        rows += analyse(Binary(p))
    if a.slots:
        for r in rows:
            if a.slots.lower() in (r["cls"] + " " + r["base"]).lower():
                print(f"== {r['binary']} vtable {r['vtable']}  {r['cls']}" + (f"  (base {r['base']})" if r["base"] else ""))
                for i, (rva, n) in enumerate(r["slots"]):
                    print(f"  slot {i:2} +0x{4 * i:02x}  {rva:#07x}  {undname(n) if n and not n.startswith('->') else (n or '?')}")
    if a.out:
        with open(a.out, "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["binary", "vtable", "class", "base", "slots", "slots_done", "vptr_writers", "slot_functions"])
            for r in rows:
                done = sum(1 for rva, _ in r["slots"] if st.get((r["binary"].lower(), rva)) in ("matched", "limited"))
                w.writerow([r["binary"], r["vtable"], r["cls"], r["base"], len(r["slots"]), done, r["writers"],
                            " ".join(f"{rva:#x}" + (f"={undname(n)}" if n and not n.startswith("->") else "") for rva, n in r["slots"])])
        print(f"{len(rows)} vtables, {sum(1 for r in rows if r['cls'])} named, "
              f"{sum(len(r['slots']) for r in rows)} slots -> {a.out}")
    if a.md:
        L = ["# Classes recovered from vtables (Tools/classmap.py)", "",
             "| binary | vtable | class | base | slots | constructors/destructors |", "|---|---|---|---|---|---|"]
        for r in rows:
            L.append(f"| {r['binary']} | {r['vtable']} | {r['cls']} | {r['base']} | {len(r['slots'])} | {r['writers'][:80]} |")
        open(a.md, "w").write("\n".join(L) + "\n")


if __name__ == "__main__":
    main()
