#!/usr/bin/env python3
"""Split Results/ALL.txt (written by build_core.bat) back into the individual files.

usage: unbundle.py ALL.txt OUTDIR

Produces OUTDIR/NAME.txt (dumpbin /disasm), NAME.compile.txt and NAME.rel
(dumpbin /relocations /symbols). compare.py / varcmp.py read NAME.rel when NAME.obj is absent.
"""
import sys, os

def main():
    src, outdir = sys.argv[1], sys.argv[2]
    os.makedirs(outdir, exist_ok=True)
    for f in os.listdir(outdir):
        if f.endswith((".txt", ".rel", ".obj")) and f != os.path.basename(src):
            os.remove(os.path.join(outdir, f))
    cur, buf, n = None, [], 0
    def flush():
        nonlocal n
        if cur:
            with open(os.path.join(outdir, cur), "w", newline="") as fh:
                fh.write("".join(buf))
            n += 1
    with open(src, encoding="latin1", newline="") as fh:
        for ln in fh:
            if ln.startswith("##FILE "):
                flush()
                cur, buf = ln[7:].strip(), []
            else:
                buf.append(ln)
    flush()
    print("%d files written to %s" % (n, outdir))

if __name__ == "__main__":
    main()
