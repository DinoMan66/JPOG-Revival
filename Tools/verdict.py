#!/usr/bin/env python3
"""
verdict.py - compact build verdicts with a retry budget. Wraps this project's compare.py.

Prints ONE line per function that matters (plus two instructions of context for differences),
remembers how many real attempts each mismatching function has had, and files functions whose
remaining difference looks compiler-limited into Notes/COMPILER_LIMITED.md once the budget is
used up - so nobody (human or AI) keeps re-trying them.

JPOG  (export-based dumpbin .txt + .rel):
  python verdict.py Original/DLLS/TRenderInterface_inst.dll Results/split/TRenderInterface.*.txt
Zoo Tycoon (CFG compare of .obj files):
  python verdict.py Original/zoo.exe Results/*.obj --map Notes/zoo.map

Options:
  --map FILE        address map (same format as compare.py)
  --tracker CSV     JPOG function_tracker.csv: status column set to matched / mismatch / limited
  --state CSV       attempt memory (default Notes/attempts.csv)
  --limited MD      where auto-filed functions go (default Notes/COMPILER_LIMITED.md)
  --budget N        real attempts before a compiler-limited-looking function is filed (default 2)
  --focus TEXT      only show functions whose name contains TEXT (may repeat)
  --all             also show known-limited and unchanged functions
  --dry-run         do not write state / tracker / COMPILER_LIMITED.md

An "attempt" is counted only when OUR bytes for that function changed since the previous run,
so re-running the same build never burns budget.

Statuses in the state file:
  open         still being worked on
  limited      filed as compiler-limited (auto or because COMPILER_LIMITED.md already lists it)
  needs-human  a SOURCE-type difference that survived 2x budget attempts: stop and ask
  matched      done
"""
import argparse, csv, datetime, glob, hashlib, os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
sys.path.insert(0, HERE)
import compare as C          # this project's compare.py
import triage_lib as T
import symcheck as SC

ZOO_MODE = hasattr(C, "compare_function")

def find_ci(path):
    """Case-insensitive path resolution (notes/ vs Notes/ on Linux)."""
    if os.path.exists(path) or not path:
        return path
    parts, cur = os.path.normpath(path).split(os.sep), os.sep if path.startswith(os.sep) else "."
    for p in parts:
        if not p:
            continue
        try:
            hit = next((e for e in os.listdir(cur) if e.lower() == p.lower()), p)
        except OSError:
            hit = p
        cur = os.path.join(cur, hit)
    return cur

# ------------------------------------------------------------------ JPOG back end
def run_jpog(a):
    d, r2o, exports, code = C.load_dll(a.binary)
    patterns = []
    if a.map and os.path.exists(a.map):
        for ln in open(a.map, errors="replace"):
            ln = ln.split("#", 1)[0].strip()
            if "=" in ln:
                k, v = ln.rsplit("=", 1); patterns.append((k.strip(), int(v.strip(), 0)))
    def lookup(name):
        if name in exports: return exports[name]
        for k, v in patterns:
            if k in name: return v
        return None
    rank = {"MATCH": 3, "MATCH (relocs tolerated)": 2, "MISMATCH": 1}
    res = {}
    spe = SC.PEInfo(a.binary)
    sres = SC.Resolver(spe, [(k, v + spe.base if v < spe.base else v) for k, v in patterns])
    for path in a.dumps:
        if path.endswith(".compile.txt") or os.path.basename(path).split(".")[1:2] and \
                os.path.basename(path).split(".")[1].startswith("Var"):
            continue                                   # skip compile logs and one-shot VarX_ builds
        objrel = C.obj_relocs(os.path.splitext(path)[0] + ".obj")
        relpath = os.path.splitext(path)[0] + ".rel"
        exact = os.path.exists(relpath) or os.path.exists(os.path.splitext(path)[0] + ".obj")
        srel = SC.jpog_relocs(relpath) if os.path.exists(relpath) else None
        for name, instrs in C.parse_dumpbin(path).items():
            if not instrs:
                continue
            rva = lookup(name)
            if rva is None:
                continue
            st, ours, theirs, first = C.compare(name, instrs, d, r2o(rva), objrel.get(name), exact)
            note, unv = None, None
            if st.startswith("MATCH") and srel is not None:
                okn, bad, unvl = SC.jpog_check_function(sres, spe.base + rva, srel.get(name, []))
                unv = len(unvl)
                if bad:
                    st, note = "MISMATCH", "relocation target differs: " + bad[0]
            elif st.startswith("MATCH"):
                unv = -1                                  # no relocation data: nothing verified
            old = res.get(name)
            if old and rank[old["st"]] >= rank[st]:
                continue
            rec = dict(st="MATCH" if st.startswith("MATCH") else "MISMATCH", addr=rva,
                       hash=hashlib.sha1(ours).hexdigest()[:12], file=os.path.basename(path), unverified=unv)
            if note:
                rec.update(first=None, ours=[], orig=[], idx=0, note=note)
            elif rec["st"] == "MISMATCH":
                oi = T.disasm(ours, 0); gi = T.disasm(theirs, 0)
                k = next((i for i, (ad, b, t) in enumerate(oi) if ad <= first < ad + len(b)), 0)
                gk = next((i for i, (ad, b, t) in enumerate(gi) if ad <= first < ad + len(b)), k)
                rec.update(first=first, ours=oi[k:k + 8], orig=gi[gk:gk + 8], idx=k)
            res[name] = rec
    code_exports = [n for n, rva in exports.items() if any(va <= rva < va + vs for va, vs in code)]
    missing = [n for n in code_exports if n not in res]
    return res, missing, os.path.basename(a.binary).replace("_inst", "")

# ------------------------------------------------------------------ Zoo back end
def run_zoo(a):
    pe = C.PE(a.binary); dis = C.Disasm(); orig = C.OrigCode(pe, dis)
    patterns = []
    for ln in open(a.map, errors="replace"):
        ln = ln.split("#", 1)[0].strip()
        if "=" in ln:
            k, v = ln.rsplit("=", 1); v = int(v.strip(), 0)
            patterns.append((k.strip(), v if v >= pe.base else v + pe.base))
    def lookup(name):
        for k, v in patterns:
            if k == name: return v
        for k, v in patterns:
            if k in name: return v
        return None
    res = {}
    for p in a.dumps:
        if os.path.basename(p).split(".")[1:2] and os.path.basename(p).split(".")[1].startswith("Var"):
            continue
        obj = C.Obj(p)
        for k, (sec, val, name) in enumerate(obj.funcs):
            va = lookup(name)
            if va is None:
                continue
            s, st, en = obj.func_bytes(k)
            raw = obj.sections[s]["raw"]
            rec = dict(addr=va, hash=hashlib.sha1(raw[st:en]).hexdigest()[:12], file=os.path.basename(p))
            try:
                C.compare_function(obj, k, va, orig, dis)
                rec["st"] = "MATCH"
            except C.Mismatch as e:
                rec["st"] = "MISMATCH"
                msg = str(e)
                mo = re.search(r"ours (0x[0-9a-f]+):", msg); mg = re.search(r"orig (0x[0-9a-f]+):", msg)
                if mo and mg:
                    oa, ga = int(mo.group(1), 16), int(mg.group(1), 16)
                    oi = [x for x in T.disasm(raw[st:en], st) if x[0] >= oa][:8]
                    gi = T.disasm(pe.read(ga, 64), ga)[:8]
                    idx = sum(1 for x in T.disasm(raw[st:en], st) if x[0] < oa)
                    rec.update(first=oa - st, ours=oi, orig=gi, idx=idx)
                else:
                    rec.update(first=None, ours=[], orig=[], idx=0, note=msg.splitlines()[0])
            old = res.get(name)
            if old is None or (old["st"] != "MATCH" and rec["st"] == "MATCH"):
                res[name] = rec
    return res, [], os.path.basename(a.binary)

# ------------------------------------------------------------------ state
FIELDS = ["module", "name", "addr", "attempts", "last_hash", "category", "status", "updated"]
def load_state(path):
    if not os.path.exists(path):
        return {}
    return {(r["module"], r["name"]): r for r in csv.DictReader(open(path, newline="", encoding="utf-8"))}

def save_state(path, st):
    os.makedirs(os.path.dirname(path) or ".", exist_ok=True)
    with open(path, "w", newline="", encoding="utf-8") as fh:
        w = csv.DictWriter(fh, FIELDS); w.writeheader()
        for k in sorted(st): w.writerow(st[k])

def already_listed(limited_text, name):
    """Is this function named in COMPILER_LIMITED.md already (manual entries)?"""
    p = T.pretty(name)
    if "::" not in p:
        return re.search(r"\b" + re.escape(p) + r"\b", limited_text) is not None
    cls, meth = p.split("::", 1)
    if p in limited_text:
        return True
    meth_word = {"ctor": cls + " ctor", "dtor": "~" + cls}.get(meth, meth)
    for ln in limited_text.splitlines():           # "TSun::SetTheta / Set" style rows
        if cls + "::" in ln or cls + " " in ln:
            if re.search(r"(?<![\w~])" + re.escape(meth_word) + r"\b", ln):
                return True
    return False

def insn(x):
    return x[2] if x else "-"

# ------------------------------------------------------------------ main
def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary"); ap.add_argument("dumps", nargs="+")
    ap.add_argument("--map"); ap.add_argument("--tracker")
    ap.add_argument("--state", default=os.path.join(ROOT, "Notes", "attempts.csv"))
    ap.add_argument("--limited", default=os.path.join(ROOT, "Notes", "COMPILER_LIMITED.md"))
    ap.add_argument("--budget", type=int, default=2)
    ap.add_argument("--focus", action="append", default=[])
    ap.add_argument("--all", action="store_true"); ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args()
    a.map = find_ci(a.map) if a.map else None
    a.tracker = find_ci(a.tracker) if a.tracker else None
    a.limited = find_ci(a.limited)
    a.dumps = [p for pat in a.dumps for p in (sorted(glob.glob(pat)) or [pat])]
    if ZOO_MODE and not a.map:
        ap.error("Zoo Tycoon mode needs --map")

    res, missing, module = (run_zoo if ZOO_MODE else run_jpog)(a)
    first_run = not os.path.exists(a.state)
    state = load_state(a.state)
    limited_text = open(a.limited, encoding="utf-8").read() if os.path.exists(a.limited) else ""
    today = datetime.date.today().isoformat()
    lines, filed, counts = [], [], dict(match=0, new_match=0, diff=0, open=0, limited=0, human=0, unchanged=0)

    for name in sorted(res, key=lambda n: res[n]["addr"]):
        r = res[name]
        key = (module, name)
        s = state.get(key) or dict(module=module, name=name, addr=f"{r['addr']:#x}", attempts="0",
                                   last_hash="", category="", status="open", updated=today)
        show = not a.focus or any(f in name or f in T.pretty(name) for f in a.focus)
        if r["st"] == "MATCH":
            counts["match"] += 1
            if s["status"] != "matched":
                counts["new_match"] += 1
                if show and not first_run: lines.append(f"MATCH   {T.pretty(name)}" + ("   (was filed as limited - remove it from COMPILER_LIMITED.md)" if s["status"] == "limited" else ""))
            s.update(status="matched", last_hash=r["hash"], updated=today)
            state[key] = s
            continue
        cat, why = T.classify(r.get("ours"), r.get("orig"), r.get("idx", 0))
        if r.get("note"):
            cat, why = "source", r["note"]
        changed = r["hash"] != s["last_hash"]
        if changed:
            s["attempts"] = str(int(s["attempts"] or 0) + 1)
        s.update(category=cat, last_hash=r["hash"], updated=today if changed else s["updated"])
        att = int(s["attempts"])
        if s["status"] in ("open", "matched") and already_listed(limited_text, name):
            s["status"] = "limited"
        if s["status"] in ("open", "matched") and att >= a.budget and cat in T.LIMITED_KINDS:
            s["status"] = "limited"
            filed.append((name, r, cat, why))
        elif s["status"] in ("open", "matched") and att >= 2 * a.budget and cat not in T.LIMITED_KINDS:
            s["status"] = "needs-human"
        elif s["status"] == "matched":
            s["status"] = "open"                      # regressed
        state[key] = s
        if s["status"] == "limited":
            counts["limited"] += 1
            if not (a.all and show) and not any(f[0] == name for f in filed):
                continue
        if s["status"] == "needs-human":
            counts["human"] += 1
        if s["status"] == "open":
            counts["open"] += 1
        counts["diff"] += 1
        if not changed and s["status"] == "open" and not a.all:
            counts["unchanged"] += 1
            continue
        if not show:
            continue
        tag = {"limited": "FILED " if any(f[0] == name for f in filed) else "LIMIT ",
               "needs-human": "HUMAN ", "open": "DIFF  "}[s["status"]]
        first = f"+{r['first']:#x}" if r.get("first") is not None else ""
        lines.append(f"{tag}  {T.pretty(name)} @{r['addr']:#x} {first} [{cat}] attempt {att}/{a.budget}  ({why})")
        o, g = r.get("ours") or [], r.get("orig") or []
        if o or g:
            lines.append(f"        ours: {insn(o[0] if o else None)} | {insn(o[1] if len(o) > 1 else None)}")
            lines.append(f"        orig: {insn(g[0] if g else None)} | {insn(g[1] if len(g) > 1 else None)}")

    # ---- summary first, then details
    print(f"{module}: {counts['match']} matched ({counts['new_match']} new) | "
          f"{counts['open']} open differences ({counts['unchanged']} unchanged since last run, hidden) | "
          f"{counts['limited']} compiler-limited (hidden) | {counts['human']} need a human | "
          f"{len(missing)} not written yet")
    for ln in lines:
        print(ln)
    if filed:
        print(f"filed {len(filed)} function(s) into {os.path.relpath(a.limited, ROOT)} - do not retry them")

    if a.dry_run:
        return
    save_state(a.state, state)
    if filed:
        txt = limited_text
        hdr = "## Auto-filed by verdict.py"
        if hdr not in txt:
            txt = txt.rstrip() + f"\n\n{hdr}\n\nFiled after {a.budget} real attempts with a difference of a compiler-limited kind " \
                  "(see Tools/triage_lib.py). Re-test when another compiler build is available.\n\n" \
                  "| Module | Function | Address | Kind | First difference (ours / orig) | Filed |\n|---|---|---|---|---|---|\n"
        for name, r, cat, why in filed:
            o = insn((r.get("ours") or [None])[0]); g = insn((r.get("orig") or [None])[0])
            txt = txt.rstrip("\n") + f"\n| {module} | {T.pretty(name)} | {r['addr']:#x} | {cat} | `{o}` / `{g}` | {today} |"
        open(a.limited, "w", encoding="utf-8").write(txt + "\n")
    if a.tracker and os.path.exists(a.tracker):
        raw = open(a.tracker, newline="", encoding="utf-8", errors="replace").read()
        eol = "\r\n" if "\r\n" in raw[:4096] else "\n"
        rows = list(csv.reader(raw.splitlines()))
        n = 0
        for row in rows[1:]:
            if row[0] == module and row[4] in res:
                s = state[(module, row[4])]["status"]
                row[5] = {"matched": "matched", "limited": "limited"}.get(s, "mismatch"); n += 1
        with open(a.tracker, "w", newline="", encoding="utf-8") as fh:
            csv.writer(fh, lineterminator=eol).writerows(rows)

if __name__ == "__main__":
    main()
