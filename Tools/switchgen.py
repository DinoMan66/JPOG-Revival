#!/usr/bin/env python3
"""Reconstruct a 'switch(x) { case K: return CONST; }' function from its jump table.

usage: switchgen.py DLL FUNC_RVA [--order addr|value|revaddr]
Prints C++ case lines. Handles 'lea eax,[edx-BASE]; cmp eax,N; ja default; jmp [edx*4+T]'
and 'cmp eax,N; ja; jmp [eax*4+T]' (BASE 0)."""
import sys, re, struct, subprocess, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import dis as DI
dll, rva = sys.argv[1], int(sys.argv[2], 16)
order = sys.argv[sys.argv.index("--order") + 1] if "--order" in sys.argv else "addr"
d = open(dll, "rb").read()
base, secs, r2o, imp = DI.imports(d)
def dis(a, n=0x40):
    t = subprocess.run(["objdump", "-d", "-M", "intel", "--no-show-raw-insn", "--start-address=%#x" % (base + a),
                        "--stop-address=%#x" % (base + a + n), dll], capture_output=True, text=True).stdout
    return [l for l in t.splitlines() if re.match(r"\s*[0-9a-f]+:", l)]
head = dis(rva, 0x30)
low, n, table, ja = 0, None, None, None
for l in head:
    m = re.search(r"\sja\s+0x([0-9a-f]+)", l)
    if m and ja is None: ja = int(m.group(1), 16) - base
    m = re.search(r"lea\s+eax,\[e[a-z]x-0x([0-9a-f]+)\]", l)
    if m: low = int(m.group(1), 16)
    m = re.search(r"cmp\s+eax,0x([0-9a-f]+)", l)
    if m and n is None: n = int(m.group(1), 16)
    m = re.search(r"\[e[a-z]x\*4\+0x([0-9a-f]+)\]", l)
    if m:
        table = (int(m.group(1), 16) + low * 4) & 0xffffffff; break
cases = {}
for i in range(n + 1):
    tgt = struct.unpack_from("<I", d, r2o(table - base + 4 * i))[0] - base
    cases.setdefault(tgt, []).append(low + i)
def body(tgt):
    l = dis(tgt, 0x10)[0]
    m = re.search(r"mov\s+eax,0x([0-9a-f]+)", l)
    if m: return "return 0x%x;" % int(m.group(1), 16)
    if "xor    eax,eax" in l: return "return 0;"
    return "/* ? %s */" % l.strip()
items = sorted(cases.items())
dflt = ja
if order == "value": items.sort(key=lambda kv: kv[1][0])
if order == "revaddr": items.reverse()
for tgt, vals in items:
    if tgt == dflt: continue
    for v in vals:
        print("    case 0x%x:" % v)
    print("        %s" % body(tgt))
print("    // default target 0x%x: %s" % (dflt, body(dflt)))
