#!/usr/bin/env python3
"""Score every build (.txt dump, with its .obj next to it) separately, per function.

usage: varcmp.py DLL MAP "Results/*.txt" [name-substring ...]

For each function that appears in the dumps (optionally filtered by substrings), prints one line
per dump file: status, number of differing (non-relocation) bytes, and size difference.
Lower score = closer. Use it to pick the best source variant out of Var*_X.cpp builds.
"""
import sys, os, glob
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import compare as C

def main():
    dll, mapf, pattern = sys.argv[1:4]
    filters = sys.argv[4:]
    d, r2o, exports, code = C.load_dll(dll)
    m = {}
    if os.path.exists(mapf):
        for ln in open(mapf):
            ln = ln.split("#")[0].strip()
            if "=" in ln:
                k, v = ln.split("=", 1)
                m[k.strip()] = int(v.strip(), 16)
    def lookup(name):
        if name in exports: return exports[name]
        for k, v in m.items():
            if k in name: return v
        return None
    table = {}
    for path in sorted(glob.glob(pattern)):
        rel = C.obj_relocs(os.path.splitext(path)[0] + ".obj")
        for name, instrs in C.parse_dumpbin(path).items():
            if not instrs or (filters and not any(f in name for f in filters)):
                continue
            rva = lookup(name)
            if rva is None:
                continue
            status, ours, theirs, first = C.compare(name, instrs, d, r2o(rva), rel.get(name))
            ndiff = 0
            if not status.startswith("MATCH"):
                tol = set()
                for r in rel.get(name, ()):
                    tol.update(range(r, r + 4))
                ndiff = sum(1 for i in range(len(ours)) if ours[i] != theirs[i] and i not in tol)
            table.setdefault(name, []).append((ndiff, os.path.basename(path), status, first))
    for name in sorted(table, key=C.pretty):
        rows = sorted(table[name])
        print(C.pretty(name), " ", name)
        for ndiff, f, st, first in rows:
            mark = "  <== MATCH" if st.startswith("MATCH") else ""
            fd = "" if first is None else "first diff +0x%x" % first
            print("    %-36s %4d bytes differ  %s%s" % (f, ndiff, fd, mark))

if __name__ == "__main__":
    main()
