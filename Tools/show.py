#!/usr/bin/env python3
"""
show.py - everything about one function in one command: tracker/attempts rows, Ghidra's C and the annotated
disassembly (calls, imports, strings resolved). Ported from ZooTycoonDecompiled\\Tools\\show.py.

Usage:  python show.py <Dll> <name-substring | rva> [--asm-only | --c-only]
        python show.py TRenderInterface ?Create@TMaterialLibrary
        python show.py SimJP 0x2f6aa0
<Dll> is the file name without _inst/.dll (TRenderInterface, TSkinShaderD3D, SimJP). Several matches -> a list.
Run from anywhere; reads Original\\, Notes\\function_tracker.csv, Notes\\attempts.csv, decompiled\\<file>.c.
Needs GNU objdump (same as dis.py).
"""
import bisect, csv, glob, os, re, subprocess, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import compare as C, dis as DI

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def find_binary(name):
    for p in glob.glob(os.path.join(ROOT, "Original", "**", "*.*"), recursive=True):
        b = os.path.basename(p)
        if b.lower().endswith((".dll", ".exe")) and \
                os.path.splitext(b.replace("_inst", ""))[0].lower() == name.lower():
            return p
    sys.exit(f"no binary called {name} under Original\\")


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    dll, what, opts = sys.argv[1], sys.argv[2], sys.argv[3:]
    path = find_binary(dll)
    d = open(path, "rb").read()
    base, secs, r2o, imp = DI.imports(d)
    _, _, exports, code = C.load_dll(path)
    byrva = {r: n for n, r in exports.items()}
    if re.match(r"^(0x)?[0-9a-fA-F]+$", what):
        rva = int(what, 16)
        rva = rva - base if rva >= base else rva
    else:
        hits = sorted((r, n) for n, r in exports.items() if what.lower() in n.lower())
        if not hits:
            sys.exit(f"no export of {os.path.basename(path)} contains '{what}'")
        if len(hits) > 1:
            print(f"{len(hits)} matches - give more of the name or the rva:")
            for r, n in hits[:30]: print(f"  {r:#07x}  {n}")
            return
        rva = hits[0][0]
    name = byrva.get(rva, f"sub_{base + rva:x}")
    print(f"== {os.path.basename(path)}  rva {rva:#x}  va {base + rva:#x}  {name}")
    key = os.path.basename(path).replace("_inst", "").lower()
    size = None
    for f, cols in (("function_tracker.csv", None), ("attempts.csv", None)):
        p = os.path.join(ROOT, "Notes", f)
        if os.path.exists(p):
            for r in csv.DictReader(open(p, newline="", encoding="utf-8", errors="replace")):
                if (r.get("decorated_name") == name or r.get("name") == name) and \
                        (not r.get("dll") or r["dll"].lower() == key):
                    print(f"{f}: " + ", ".join(f"{k}={v}" for k, v in r.items() if v))
                    if r.get("max_size_bytes"): size = int(r["max_size_bytes"])
    if "--asm-only" not in opts:
        cf = os.path.join(ROOT, "decompiled", os.path.basename(path) + ".c")
        if os.path.exists(cf):
            txt = open(cf, encoding="utf-8", errors="replace").read()
            m = re.search(r"\n// ==== 0*%x  .*\n" % (base + rva), txt, re.I)
            if m:
                end = txt.find("\n// ==== ", m.end())
                print(txt[m.start():end if end > 0 else None].rstrip())
            else:
                print(f"(no Ghidra function at {base + rva:#x} in decompiled\\{os.path.basename(cf)})")
        else:
            print(f"(no decompiled\\{os.path.basename(path)}.c - run Tools\\export_decompiled.bat)")
    if "--c-only" in opts:
        return
    starts = sorted(set(r for r in exports.values()))
    i = bisect.bisect_right(starts, rva)
    stop = starts[i] if i < len(starts) else rva + 0x1000
    stop = min(stop, rva + (size or 0x1000))
    def strat(a):
        o = r2o(a - base)
        if o is None: return None
        s = d[o:o + 80].split(b"\0")[0]
        return '"%s"' % s.decode() if len(s) >= 3 and all(32 <= c < 127 for c in s) else None
    names = {base + r: n for n, r in exports.items()}
    out = subprocess.run(["objdump", "-d", "-M", "intel", "--no-show-raw-insn",
                          f"--start-address={base + rva:#x}", f"--stop-address={base + stop:#x}", path],
                         capture_output=True, text=True).stdout
    for ln in out.splitlines():
        m = re.match(r"\s*([0-9a-f]+):\s*(.*)", ln)
        if not m: continue
        a, ins = int(m.group(1), 16), m.group(2)
        notes = []
        for h in re.findall(r"0x([0-9a-f]{6,8})", ins):
            v = int(h, 16)
            if v in imp: notes.append(imp[v])
            elif v in names: notes.append(names[v])
            else:
                o = r2o(v - base)
                if o is not None and d[o:o + 2] == b"\xff\x25":
                    notes.append("-> " + imp.get(int.from_bytes(d[o + 2:o + 6], "little"), "?"))
                else:
                    s = strat(v)
                    if s: notes.append(s)
        print(f"{a - base:08x}: {ins}" + (("    ; " + ", ".join(notes)) if notes else ""))


if __name__ == "__main__":
    main()
