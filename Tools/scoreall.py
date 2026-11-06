#!/usr/bin/env python3
"""One-shot scoring of a whole build: unbundle Results/ALL.txt, then run verdict.py per DLL.

usage: scoreall.py [ALL.txt] [DLL ...] [--res DIR] [--all] [--budget N]

  ALL.txt   default: Results/ALL.txt next to this Tools folder
  DLL       only these DLL folders (e.g. TRenderInterface)
  --res     where to unbundle (default: a fresh temporary folder)
  --all     pass --all to verdict.py (also show limited / unchanged functions)

Prints verdict.py's compact output per DLL and updates Notes/function_tracker.csv,
Notes/attempts.csv and Notes/COMPILER_LIMITED.md. Works from any folder, on the PC or on Claude's side.
"""
import argparse, glob, os, subprocess, sys, tempfile

T = os.path.dirname(os.path.abspath(__file__))
O = os.path.dirname(T)

def ci(path):
    """case-insensitive lookup of a path relative to the project (notes/ vs Notes/)"""
    cur = O
    for part in path.split("/"):
        try:
            cur = os.path.join(cur, next((e for e in os.listdir(cur) if e.lower() == part.lower()), part))
        except OSError:
            cur = os.path.join(cur, part)
    return cur

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("bundle", nargs="?", default=ci("Results/ALL.txt"))
    ap.add_argument("dlls", nargs="*")
    ap.add_argument("--res"); ap.add_argument("--all", action="store_true"); ap.add_argument("--budget")
    a = ap.parse_args()
    if a.bundle and not a.bundle.lower().endswith(".txt"):         # first word was a DLL name
        a.dlls.insert(0, a.bundle); a.bundle = ci("Results/ALL.txt")
    res = a.res or tempfile.mkdtemp(prefix="jpog_res_")
    subprocess.run([sys.executable, "-I", os.path.join(T, "unbundle.py"), a.bundle, res], check=True,
                   stdout=subprocess.DEVNULL)
    originals = {}
    for p in glob.glob(os.path.join(O, "Original", "**", "*.dll"), recursive=True) + \
             glob.glob(os.path.join(O, "Original", "*.exe")):              # SimJP.exe -> Src\SimJP\
        b = os.path.basename(p).replace("_inst", "").replace(".dll", "").replace(".exe", "")
        if b not in originals or "_inst" in p:
            originals[b] = p
    dlls = sorted({f.split(".")[0] for f in os.listdir(res)          # <Dll>.<file>.txt only (skips cmd.txt etc.)
                   if f.endswith(".txt") and not f.endswith(".compile.txt") and f.count(".") >= 2})
    if a.dlls:
        dlls = [d for d in dlls if d in a.dlls]
    for dll in dlls:
        if dll not in originals:
            print(dll, ": no original DLL found under Original/"); continue
        files = sorted(f for f in glob.glob(os.path.join(res, dll + ".*.txt"))
                       if not f.endswith(".compile.txt") and not os.path.basename(f).split(".")[1].startswith("Var"))
        args = [sys.executable, "-I", os.path.join(T, "verdict.py"), originals[dll]] + files + \
               ["--tracker", ci("Notes/function_tracker.csv")]
        m = ci("Notes/%s.map" % dll)
        if os.path.exists(m): args += ["--map", m]
        if a.all: args.append("--all")
        if a.budget: args += ["--budget", a.budget]
        sys.stdout.flush()
        subprocess.run(args)
        # compile errors are the other thing worth seeing
        for f in sorted(glob.glob(os.path.join(res, dll + ".*.compile.txt"))):
            errs = [l.strip() for l in open(f, errors="replace") if ": error" in l]
            if errs:
                print(f"  COMPILE ERRORS in {os.path.basename(f).replace('.compile.txt', '.cpp')}:")
                for e in errs[:5]: print("    " + e)

if __name__ == "__main__":
    main()
