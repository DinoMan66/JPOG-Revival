#!/usr/bin/env python3
"""Print every vtable (??_7 export) of a DLL with slot names; resolves import thunks.
usage: vtables.py DLL"""
import sys, struct, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import compare as C, dis as DI
dll = sys.argv[1]
d = open(dll, "rb").read()
base, secs, r2o, imp = DI.imports(d)
_, _, exports, code = C.load_dll(dll)
byaddr = {base + r: n for n, r in exports.items()}
lo, hi = base + code[0][0], base + code[0][0] + code[0][1]
def name(v):
    if v in byaddr: return byaddr[v]
    o = r2o(v - base)
    if o is not None and d[o:o+2] == b"\xff\x25":
        return "thunk -> " + imp.get(struct.unpack_from("<I", d, o + 2)[0], "?")
    return "?"
starts = sorted((r, n) for n, r in exports.items() if n.startswith("??_7"))
for r, n in starts:
    print("==", n, hex(r))
    o = r2o(r); i = 0
    while True:
        v = struct.unpack_from("<I", d, o + 4 * i)[0]
        if not (lo <= v < hi) or (i and base + r + 4 * i - base in [x for x, _ in starts]):
            break
        print("  %2d +0x%02x  %s" % (i, 4 * i, name(v)))
        i += 1
