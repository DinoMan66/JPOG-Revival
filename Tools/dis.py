#!/usr/bin/env python3
"""Annotated disassembly of a whole DLL (objdump + names), split per exported/known function.

usage: dis.py DLL [OUT.txt] [--map MAP]

Annotates: import thunks (ds:ADDR -> dll!name), call targets (-> export/map name),
immediate addresses of exported data (vtables, m_sClass...), float constants, and strings.
"""
import sys, re, struct, subprocess, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import compare as C

def imports(d):
    pe = struct.unpack_from("<I", d, 0x3c)[0]
    base = struct.unpack_from("<I", d, pe + 0x34)[0]
    nsec = struct.unpack_from("<H", d, pe + 6)[0]
    so = pe + 24 + struct.unpack_from("<H", d, pe + 20)[0]
    secs = [struct.unpack_from("<8sIIII", d, so + 40 * i) for i in range(nsec)]
    def r2o(r):
        for nm, vs, va, rs, ra in secs:
            if va <= r < va + max(vs, rs):
                return ra + r - va
        return None
    imp_rva = struct.unpack_from("<I", d, pe + 0x80)[0]
    out = {}
    o = r2o(imp_rva)
    while o is not None:
        ilt, _, _, name, iat = struct.unpack_from("<IIIII", d, o)
        if name == 0:
            break
        dll = d[r2o(name):d.index(b"\0", r2o(name))].decode()
        t = r2o(ilt or iat); k = 0
        while True:
            e = struct.unpack_from("<I", d, t + 4 * k)[0]
            if e == 0:
                break
            if e & 0x80000000:
                nm = "#%d" % (e & 0xffff)
            else:
                p = r2o(e) + 2
                nm = d[p:d.index(b"\0", p)].decode()
            out[base + iat + 4 * k] = dll.split(".")[0] + "!" + nm
            k += 1
        o += 20
    return base, secs, r2o, out

def main():
    dll = sys.argv[1]
    outp = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[2].startswith("--") else None
    mp = {}
    if "--map" in sys.argv:
        for ln in open(sys.argv[sys.argv.index("--map") + 1]):
            ln = ln.split("#")[0].strip()
            if "=" in ln:
                k, v = ln.split("=", 1); mp[int(v, 16)] = k.strip()
    d = open(dll, "rb").read()
    base, secs, r2o, imp = imports(d)
    _, _, exports, code = C.load_dll(dll)
    names = {base + r: n for n, r in exports.items()}
    names.update({base + r: n for r, n in mp.items()})
    def strat(a):
        o = r2o(a - base)
        if o is None: return None
        s = d[o:o + 80].split(b"\0")[0]
        if len(s) >= 3 and all(32 <= c < 127 for c in s):
            return '"%s"' % s.decode()
        return None
    def fl(a):
        o = r2o(a - base)
        if o is None or o + 4 > len(d): return None
        f = struct.unpack_from("<f", d, o)[0]
        return "%g" % f if f == f and 1e-6 < abs(f) < 1e7 or f == 0 else None
    txt = subprocess.run(["objdump", "-d", "-M", "intel", "--no-show-raw-insn", dll],
                         capture_output=True, text=True).stdout
    out = []
    for ln in txt.splitlines():
        m = re.match(r"\s*([0-9a-f]+):\s*(.*)", ln)
        if not m:
            continue
        a = int(m.group(1), 16); ins = m.group(2)
        if a in names:
            out.append("\n===== %s  (rva 0x%x)" % (names[a], a - base))
        notes = []
        for h in re.findall(r"0x([0-9a-f]{6,8})", ins):
            v = int(h, 16)
            if v in imp: notes.append(imp[v])
            elif v in names: notes.append(names[v])
            elif base <= v < base + 0x100000:
                s = strat(v)
                if s: notes.append(s)
                elif "fld" in ins or "fcom" in ins or "fmul" in ins or "fadd" in ins or "fsub" in ins or "fdiv" in ins:
                    f = fl(v)
                    if f: notes.append("float " + f)
        out.append("%08x: %s%s" % (a - base, ins, ("    ; " + ", ".join(notes)) if notes else ""))
    s = "\n".join(out) + "\n"
    if outp:
        open(outp, "w").write(s)
        print("written", outp)
    else:
        sys.stdout.write(s)

if __name__ == "__main__":
    main()
