#!/usr/bin/env python3
"""
compare.py - check compiled functions against the original DLL, byte for byte.

Usage:
  python compare.py ORIGINAL.dll build1.txt [build2.txt ...] [options]     (wildcards such as results\\TInput*.txt work)

Options:
  --map FILE        address map for NON-exported functions. One entry per line:
                        part-of-decorated-name = 0xRVA        (# starts a comment)
                    e.g.  ??3@YAXPAX@Z = 0x1510
  --rva NAME=0xRVA  same thing for a single function, on the command line
  --tracker CSV     update the status column (matched / mismatch) of that function
                    tracker file for every exported function that was compared
  --report FILE     where to write the full report (default: reports\\<dll name>_report.txt).
                    UPLOAD THIS FILE instead of copying the console output.
  --brief           print only the totals on screen (details are always in the report)
  --verbose         print the full bytes of mismatching functions

The .txt files are the output of:  dumpbin /disasm something.obj > something.txt
Each function is looked up by its decorated name in the DLL's export table.
Functions that are NOT exported (file-local helpers) can be given an address with
  --rva ?MyHelper@@YAXXZ=0x1000
(the RVA is the address from Ghidra minus the image base, e.g. 0x10001000 -> 0x1000).

Result per function:
  MATCH                      identical bytes (including alignment padding)
  MATCH (relocs tolerated)   only differs in the 4-byte address/displacement field of
                             instructions the linker patches (calls, references to globals
                             or imports). Everything else in those instructions - opcode,
                             registers, length - must still be identical.
  MISMATCH                   real difference - shows the first differing offset
  SIZE DIFF                  our function is a different length
  NOT FOUND                  not exported and no --rva given
Exit code is 0 only if every function matched.
"""
import re, struct, sys, argparse

def load_dll(path):
    d = open(path, "rb").read()
    pe = struct.unpack_from("<I", d, 0x3c)[0]
    nsec = struct.unpack_from("<H", d, pe + 6)[0]
    optsz = struct.unpack_from("<H", d, pe + 20)[0]
    so = pe + 24 + optsz
    secs = [struct.unpack_from("<8sIIII", d, so + i * 40) for i in range(nsec)]
    code = [(struct.unpack_from("<I", d, so + i * 40 + 12)[0], struct.unpack_from("<I", d, so + i * 40 + 8)[0])
            for i in range(nsec) if struct.unpack_from("<I", d, so + i * 40 + 36)[0] & 0x20000000]
    def r2o(x):
        for _, vs, va, rs, rp in secs:
            if va <= x < va + max(vs, rs):
                return x - va + rp
        return None
    exports = {}
    erva = struct.unpack_from("<I", d, pe + 24 + 96)[0]
    if erva:
        eo = r2o(erva)
        nn, af, an, ao = struct.unpack_from("<IIII", d, eo + 24)
        ordbase = struct.unpack_from("<I", d, eo + 16)[0]
        for i in range(nn):
            n = struct.unpack_from("<I", d, r2o(an) + 4 * i)[0]
            s = d[r2o(n):r2o(n) + 600].split(b"\0")[0].decode(errors="replace")
            od = struct.unpack_from("<H", d, r2o(ao) + 2 * i)[0]
            exports[s] = struct.unpack_from("<I", d, r2o(af) + 4 * od)[0]
    return d, r2o, exports, code

HDR = re.compile(r"^(\S+?)(?: \(.*\))?:$")
def parse_dumpbin(path):
    """-> {name: [(bytes, has_symbol_reference), ...]} (one entry per instruction)"""
    funcs, cur, last = {}, None, None
    for ln in open(path, errors="replace").read().splitlines():
        if ln and not ln[0].isspace():
            m = HDR.match(ln.strip())
            if m and not ln.startswith(("Microsoft", "Copyright", "Dump of", "File Type", "Summary")):
                cur = m.group(1); funcs[cur] = []; last = None
            continue
        if cur is None or len(ln) < 13:
            continue
        m = re.match(r"^\s+[0-9A-F]{8}: ", ln)
        if m:
            raw = ln[12:31].split()
            if not all(re.fullmatch(r"[0-9A-F]{2}", t) for t in raw):
                continue
            text = ln[31:].strip()
            ins = [bytes(int(t, 16) for t in raw), bool(re.search(r"[?_$@][\w?@$]", text)) ]
            funcs[cur].append(ins); last = ins
        elif ln.startswith(" " * 12) and last is not None:      # wrapped long instruction
            raw = ln.split()
            if all(re.fullmatch(r"[0-9A-F]{2}", t) for t in raw):
                last[0] += bytes(int(t, 16) for t in raw)
    return funcs

def obj_relocs(path):
    """Read the COFF .obj next to a dumpbin listing and return {function name: set(offsets of
    relocated 4-byte fields, relative to the function start)}. Exact, so no guessing needed."""
    import struct, os
    if not os.path.exists(path):
        rel = os.path.splitext(path)[0] + ".rel"
        return rel_text_relocs(rel) if os.path.exists(rel) else {}
    data = open(path, "rb").read()
    nsect, symptr, nsyms, optsize = (struct.unpack_from("<H", data, 2)[0], struct.unpack_from("<I", data, 8)[0],
                                     struct.unpack_from("<I", data, 12)[0], struct.unpack_from("<H", data, 16)[0])
    strtab = symptr + nsyms * 18
    def symname(off):
        raw = data[off:off + 8]
        if raw[:4] == b"\0\0\0\0":
            o = strtab + struct.unpack_from("<I", raw, 4)[0]
            return data[o:data.index(b"\0", o)].decode("latin1")
        return raw.rstrip(b"\0").decode("latin1")
    funcs = {}          # section number -> [(value, name)]
    i = 0
    while i < nsyms:
        o = symptr + i * 18
        value, secnum, typ, sclass, naux = struct.unpack_from("<IhHBB", data, o + 8)
        if secnum > 0 and (typ >> 4) == 2 and sclass in (2, 3):
            funcs.setdefault(secnum, []).append((value, symname(o)))
        i += 1 + naux
    out = {}
    for sn in range(1, nsect + 1):
        h = 20 + optsize + (sn - 1) * 40
        relptr, nrel = struct.unpack_from("<I", data, h + 24)[0], struct.unpack_from("<H", data, h + 32)[0]
        fl = sorted(funcs.get(sn, []))
        if not fl:
            continue
        for k in range(nrel):
            va = struct.unpack_from("<I", data, relptr + k * 10)[0]
            owner = None
            for v, n in fl:
                if v <= va: owner = (v, n)
            if owner:
                out.setdefault(owner[1], set()).add(va - owner[0])
    return out

def rel_text_relocs(path):
    """Same as obj_relocs, from the text of 'dumpbin /relocations /symbols' (NAME.rel)."""
    funcs, rels, cur = {}, {}, None
    for ln in open(path, encoding="latin1"):
        m = re.match(r"\s*[0-9A-F]{3,}\s+([0-9A-F]{8})\s+SECT([0-9A-F]+)\s+\w+\s+\(\)\s+(External|Static)\s+\|\s+(\S+)", ln)
        if m:
            funcs.setdefault(int(m.group(2), 16), []).append((int(m.group(1), 16), m.group(4)))
            continue
        m = re.match(r"\s*RELOCATIONS #([0-9A-F]+)", ln)
        if m:
            cur = int(m.group(1), 16); continue
        m = re.match(r"\s*([0-9A-F]{8})\s+(DIR32NB|DIR32|REL32|SECREL|SECTION)\b", ln)
        if m and cur is not None:
            rels.setdefault(cur, []).append(int(m.group(1), 16))
    out = {}
    for sn, offs in rels.items():
        fl = sorted(funcs.get(sn, []))
        for va in offs:
            owner = None
            for v, n in fl:
                if v <= va: owner = (v, n)
            if owner:
                out.setdefault(owner[1], set()).add(va - owner[0])
    return out

def reloc_span(b, sym):
    """Return (start, end) of the 4 bytes the linker will patch in this instruction, or None."""
    n = len(b)
    if n < 5:
        return None
    op = b[0]
    # call rel32 to a function in the same object has NO symbol text in dumpbin, so detect it by opcode
    if op == 0xE8:
        return (1, 5)
    if op == 0xE9 and b[1:5] == b"\0\0\0\0":
        return (1, 5)
    if not sym:
        return None
    if op in (0xA1, 0xA3, 0x68) or 0xB8 <= op <= 0xBF:
        return (n - 4, n) if op in (0x68,) or 0xB8 <= op <= 0xBF else (1, 5)
    off = 3 if op == 0x0F else 2                      # skip opcode (+0F) and modrm
    modrm = b[2] if op == 0x0F and n > 2 else (b[1] if n > 1 else None)
    if modrm is not None and (modrm & 0xC7) == 0x05 and n >= off + 4:
        return (off, off + 4)                         # [disp32] absolute address
    return (n - 4, n)

MODRM_OPS = {0x8B,0x89,0x8A,0x88,0x8D,0x03,0x01,0x2B,0x29,0x3B,0x39,0x33,0x31,0x0B,0x09,0x23,0x21,
             0x02,0x00,0x0A,0x08,0x22,0x20,0x2A,0x28,0x3A,0x38,0x84,0x85,0x86,0x87,0x80,0x81,0x83,
             0xC6,0xC7,0xF6,0xF7,0xFE,0xFF,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,0x13,0x11,0x1B,0x19}
def abs_span(b):
    """(start, end) of an absolute 32-bit address operand, or None."""
    n = len(b)
    if n >= 5 and b[0] in (0xA0, 0xA1, 0xA2, 0xA3):
        return (1, 5)
    if n >= 6 and b[0] in MODRM_OPS and (b[1] & 0xC7) == 0x05:
        return (2, 6)
    # [index*scale + disp32] with no base register (switch jump tables): mod=00 rm=100, SIB base=101
    if n >= 7 and b[0] in MODRM_OPS and (b[1] & 0xC7) == 0x04 and (b[2] & 0x07) == 0x05:
        return (3, 7)
    return None

def compare(name, instrs, d, off, relocs=None, exact=False):
    """exact=True: relocation data (.obj/.rel) is available, so ONLY the real relocated fields are
    tolerated. The opcode/address guesses below are a fallback for dumps without relocation data;
    with exact data they would also hide wrong constants (anti-cheat: small value vs DLL address)."""
    ours = b"".join(i[0] for i in instrs)
    theirs = d[off:off + len(ours)]
    tolerate = bytearray()
    for b, sym in ([] if exact else instrs):
        t = bytearray(len(b))
        span = reloc_span(b, sym)
        if span:
            for i in range(span[0], span[1]):
                t[i] = 1
        else:
            # an absolute address that looks relocated: tiny in our .obj, inside the DLL image in the original
            a = abs_span(b)
            if a:
                pos = len(tolerate)
                ours_v = int.from_bytes(b[a[0]:a[1]], "little")
                orig_v = int.from_bytes(d[off + pos + a[0]: off + pos + a[1]], "little")
                delta = (orig_v - ours_v) & 0xFFFFFFFF      # = image base + section rva of the target
                if ours_v < 0x1000000 and 0x10000000 <= orig_v < 0x20000000 or 0x10000000 <= delta < 0x20000000:
                    for i in range(a[0], a[1]):
                        t[i] = 1
        tolerate += t
    if exact:
        tolerate = bytearray(len(ours))
    for r in (relocs or ()):                          # exact relocations from the .obj
        for i in range(r, min(r + 4, len(tolerate))):
            tolerate[i] = 1
    diffs = [i for i in range(len(ours)) if ours[i] != theirs[i]]
    if not diffs:
        return "MATCH", ours, theirs, None
    hard = [i for i in diffs if not tolerate[i]]
    if not hard:
        return "MATCH (relocs tolerated)", ours, theirs, None
    return "MISMATCH", ours, theirs, hard[0]

def pretty(name):
    """Readable short form of a decorated name: Class::function (no real demangler available)."""
    m = re.match(r"\?\?0([A-Za-z_]\w*)@", name)
    if m: return m.group(1) + "::ctor"
    m = re.match(r"\?\?1([A-Za-z_]\w*)@", name)
    if m: return m.group(1) + "::dtor"
    m = re.match(r"\?\?4([A-Za-z_]\w*)@", name)
    if m: return m.group(1) + "::operator="
    m = re.match(r"\?\?_E([A-Za-z_]\w*)@", name)
    if m: return m.group(1) + "::vector-deleting-dtor"
    m = re.match(r"\?\?_G([A-Za-z_]\w*)@", name)
    if m: return m.group(1) + "::scalar-deleting-dtor"
    if name.startswith("??2"): return "operator new"
    if name.startswith("??3"): return "operator delete"
    m = re.match(r"\?([A-Za-z_]\w*)@([A-Za-z_]\w*)@", name)
    if m: return m.group(2) + "::" + m.group(1)
    m = re.match(r"\?([A-Za-z_]\w*)@@", name)
    if m: return m.group(1)
    return name

def main():
    import glob as _glob, os, datetime
    ap = argparse.ArgumentParser()
    ap.add_argument("dll"); ap.add_argument("dumps", nargs="+")
    ap.add_argument("--rva", action="append", default=[])
    ap.add_argument("--map"); ap.add_argument("--tracker"); ap.add_argument("--report")
    ap.add_argument("--brief", action="store_true")
    ap.add_argument("--verbose", "-v", action="store_true")
    a = ap.parse_args()
    expanded = []
    for pat in a.dumps:                       # Windows does not expand wildcards for us
        hits = sorted(_glob.glob(pat))
        expanded.extend(hits if hits else [pat])
    a.dumps = expanded
    d, r2o, exports, code = load_dll(a.dll)
    base = os.path.basename(a.dll).replace("_inst", "")
    extra = {k.strip(): int(v, 0) for k, v in (s.split("=", 1) for s in a.rva)}
    patterns = []
    if a.map:
        for ln in open(a.map, errors="replace"):
            ln = ln.split("#", 1)[0].strip()
            if "=" in ln:
                k, v = ln.rsplit("=", 1); patterns.append((k.strip(), int(v.strip(), 0)))
    def lookup(name):
        if name in extra: return extra[name]
        if name in exports: return exports[name]
        for k, v in patterns:
            if k in name: return v
        return None

    # ---- compare, de-duplicating functions that appear in several .obj dumps (inline functions)
    rank = {"MATCH": 3, "MATCH (relocs tolerated)": 2, "MISMATCH": 1}
    res = {}            # name -> dict
    extras = {}         # names we compiled that the original does not contain
    seen = 0
    for path in a.dumps:
        objrel = obj_relocs(os.path.splitext(path)[0] + ".obj")
        exact = any(os.path.exists(os.path.splitext(path)[0] + x) for x in (".obj", ".rel"))
        for name, instrs in parse_dumpbin(path).items():
            if not instrs:
                continue
            seen += 1
            rva = lookup(name)
            if rva is None:
                extras[name] = os.path.basename(path); continue
            status, ours, theirs, first = compare(name, instrs, d, r2o(rva), objrel.get(name), exact)
            rec = dict(status=status, rva=rva, size=len(ours), first=first, ours=ours, theirs=theirs,
                       files={os.path.basename(path)})
            old = res.get(name)
            if old is None or rank[status] > rank[old["status"]]:
                if old: rec["files"] |= old["files"]
                res[name] = rec
            else:
                old["files"].add(os.path.basename(path))
    matched = {n: r for n, r in res.items() if r["status"].startswith("MATCH")}
    bad = {n: r for n, r in res.items() if not r["status"].startswith("MATCH")}

    # ---- original functions (exported, in code sections) that no build contained yet
    code_exports = sorted((rva, n) for n, rva in exports.items() if any(va <= rva < va + vs for va, vs in code))
    mapped_rvas = {r["rva"] for r in res.values()}
    missing = []
    for i, (rva, n) in enumerate(code_exports):
        if n in res: continue
        end = next(va + vs for va, vs in code if va <= rva < va + vs)
        nxt = code_exports[i + 1][0] if i + 1 < len(code_exports) and code_exports[i + 1][0] < end else end
        missing.append((rva, nxt - rva, n))
    exp_matched = sum(1 for n in matched if n in exports)

    # ---- the report file
    rep = a.report or os.path.join("reports", base.replace(".dll", "") + "_report.txt")
    os.makedirs(os.path.dirname(rep) or ".", exist_ok=True)
    L = []
    L.append(f"JPOG compare report  -  {base}  -  {datetime.datetime.now():%Y-%m-%d %H:%M}")
    L.append(f"target: {a.dll}")
    L.append(f"address map: {a.map or '(none)'}  ({len(patterns)} entries loaded)")
    L.append(f"build files: {', '.join(os.path.basename(p) for p in a.dumps)}")
    L.append("")
    L.append("SUMMARY")
    L.append(f"  original functions in code (exported): {len(code_exports)}")
    L.append(f"  matched: {exp_matched} exported + {len(matched) - exp_matched} non-exported = {len(matched)}")
    L.append(f"  mismatched: {len(bad)}")
    L.append(f"  not written yet: {len(missing)} exported functions have no counterpart in the build")
    L.append(f"  compiler extras (in our build, not in the original): {len(extras)}")
    pct = 100.0 * exp_matched / max(1, len(code_exports))
    L.append(f"  PROGRESS: {exp_matched}/{len(code_exports)} exported functions = {pct:.1f}%")
    L.append("")
    L.append(f"MISMATCHES ({len(bad)})")
    if not bad: L.append("  (none)")
    for n, r in sorted(bad.items(), key=lambda kv: kv[1]["rva"]):
        L.append(f"  {pretty(n)}   [{r['size']} bytes @ rva {r['rva']:#x}]   in {', '.join(sorted(r['files']))}")
        L.append(f"    {n}")
        f = r["first"]; lo = max(0, f - 4); hi = min(r["size"], f + 12)
        L.append(f"    first difference at offset +{f:#x}")
        L.append(f"    ours: {r['ours'][lo:hi].hex(' ')}")
        L.append(f"    orig: {r['theirs'][lo:hi].hex(' ')}")
        if a.verbose:
            L.append(f"    full ours: {r['ours'].hex(' ')}"); L.append(f"    full orig: {r['theirs'].hex(' ')}")
    L.append("")
    L.append(f"NOT WRITTEN YET ({len(missing)})   [rva, approx. max size, function]")
    for rva, sz, n in missing:
        L.append(f"  {rva:#06x} {sz:#6x}  {pretty(n)}    {n}")
    L.append("")
    L.append(f"MATCHED ({len(matched)})")
    for n, r in sorted(matched.items(), key=lambda kv: kv[1]["rva"]):
        tag = "~" if r["status"] != "MATCH" else " "
        L.append(f"  {tag} {r['rva']:#06x} {r['size']:#5x}  {pretty(n)}")
    L.append("  (~ = matched with linker-patched addresses ignored)")
    L.append("")
    L.append(f"COMPILER EXTRAS ({len(extras)})  - functions in our build that the original DLL does not contain")
    for n in sorted(extras): L.append(f"  {pretty(n)}    {n}")
    with open(rep, "w", encoding="utf-8") as fh:
        fh.write("\n".join(L) + "\n")

    # ---- tracker
    if a.tracker:
        import csv
        rows = list(csv.reader(open(a.tracker, newline="")))
        n_up = 0
        for r in rows[1:]:
            if r[0] == base and r[4] in res:
                r[5] = "matched" if res[r[4]]["status"].startswith("MATCH") else "mismatch"; n_up += 1
        csv.writer(open(a.tracker, "w", newline="")).writerows(rows)
        print(f"tracker updated: {n_up} rows")

    # ---- console: short
    print(f"\n{base}: {exp_matched}/{len(code_exports)} exported functions matched ({pct:.1f}%)   "
          f"| mismatched {len(bad)} | not written yet {len(missing)} | compiler extras {len(extras)}")
    if not a.brief:
        for n, r in sorted(bad.items(), key=lambda kv: kv[1]["rva"])[:12]:
            print(f"  MISMATCH {pretty(n)} @ {r['rva']:#x}: +{r['first']:#x}")
        if len(bad) > 12: print(f"  ... and {len(bad) - 12} more (see report)")
    print(f"\nFull report written to: {os.path.abspath(rep)}")
    print("Upload that file to Claude instead of copying this window.")
    sys.exit(0 if not bad else 1)

if __name__ == "__main__":
    main()
