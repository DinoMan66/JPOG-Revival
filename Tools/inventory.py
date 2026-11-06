#!/usr/bin/env python3
r"""
inventory.py - catalogue a game install folder so we can see what needs decompiling.

Usage:   py tools\inventory.py "C:\path\to\game folder"  [--out inventory.csv] [--hash]

Writes a CSV (one row per file) and prints a summary. Nothing is modified and nothing
is uploaded anywhere. The CSV is small - upload it to Claude instead of the game files.

Columns: path, size, ext, modified, magic (first 16 bytes, hex), kind,
         pe_linked (PE link time), pe_exports, intel_cc (built with Intel compiler?),
         sha1 (only with --hash)
"""
import argparse, csv, collections, datetime, hashlib, os, re, struct, sys

def pe_info(path, size):
    """Return (link_time, n_exports, intel_cc, section_names) for a PE file, else None."""
    try:
        with open(path, "rb") as f:
            data = f.read(min(size, 64 * 1024 * 1024))
        if data[:2] != b"MZ":
            return None
        pe = struct.unpack_from("<I", data, 0x3c)[0]
        if data[pe:pe + 4] != b"PE\0\0":
            return None
        nsec, ts = struct.unpack_from("<HI", data, pe + 6)
        optsz = struct.unpack_from("<H", data, pe + 20)[0]
        so = pe + 24 + optsz
        names = [data[so + i * 40:so + i * 40 + 8].rstrip(b"\0").decode(errors="replace")
                 for i in range(nsec)]
        secs = [struct.unpack_from("<8sIIII", data, so + i * 40) for i in range(nsec)]
        def r2o(x):
            for _, vs, va, rs, rp in secs:
                if va <= x < va + max(vs, rs):
                    return x - va + rp
        nexp = 0
        erva = struct.unpack_from("<I", data, pe + 24 + 96)[0]
        if erva and r2o(erva) is not None:
            nexp = struct.unpack_from("<I", data, r2o(erva) + 24)[0]
        intel = b"Intel(R) C++ Compiler" in data
        when = datetime.datetime.fromtimestamp(ts, datetime.timezone.utc).strftime("%Y-%m-%d %H:%M")
        return when, nexp, intel, names
    except Exception:
        return None

def classify(ext, magic, pe):
    e = ext.lower()
    if pe:
        if e == ".exe": return "CODE: executable"
        return "CODE: DLL/PE"
    if magic[:4] == b"RIFF": return "audio/video (RIFF)"
    if magic[:3] == b"ID3" or magic[:2] == b"\xff\xfb": return "audio (mp3)"
    if magic[:4] == b"BIKi" or magic[:3] == b"BIK": return "video (Bink)"
    if magic[:4] == b"\x89PNG": return "image (png)"
    if magic[:2] == b"BM": return "image (bmp)"
    if magic[:3] == b"\xff\xd8\xff": return "image (jpg)"
    if magic[:4] == b"DDS ": return "image (dds)"
    if magic[:2] == b"PK": return "archive (zip-like)"
    if magic[:4] == b"FSB3" or magic[:4] == b"FSB4" or magic[:3] == b"FSB": return "audio (FMOD bank)"
    if e in (".txt", ".ini", ".cfg", ".log", ".xml", ".lua", ".bat", ".h", ".cpp", ".c", ".inc", ".csv", ".nfo"): return "text/config"
    if e in (".tga", ".tex", ".pcx"): return "image/texture"
    if e in (".wav", ".mp3", ".ogg", ".wma", ".mid"): return "audio"
    if e in (".avi", ".bik", ".mpg", ".mpeg", ".wmv", ".ogm", ".ogv"): return "video"
    if e in (".cab", ".msi", ".inf", ".ico", ".cur", ".lic", ".dat", ".tmp"): return "installer/misc"
    return "UNKNOWN (needs a look)"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("folder"); ap.add_argument("--out", default="inventory.csv")
    ap.add_argument("--hash", action="store_true", help="also compute SHA1 (slower)")
    a = ap.parse_args()
    root = os.path.abspath(a.folder)
    if not os.path.isdir(root):
        sys.exit(f"Not a folder: {root}")
    rows = []
    for dp, dn, fn in os.walk(root):
        for name in sorted(fn):
            p = os.path.join(dp, name)
            try:
                st = os.stat(p); size = st.st_size
                with open(p, "rb") as f: magic = f.read(16)
            except OSError:
                continue
            ext = os.path.splitext(name)[1]
            pe = pe_info(p, size)
            kind = classify(ext, magic, pe)
            rows.append({
                "path": os.path.relpath(p, root), "size": size, "ext": ext.lower(),
                "modified": datetime.datetime.fromtimestamp(st.st_mtime).strftime("%Y-%m-%d %H:%M"),
                "magic": magic.hex(" "), "kind": kind,
                "pe_linked": pe[0] if pe else "", "pe_exports": pe[1] if pe else "",
                "intel_cc": ("yes" if pe[2] else "no") if pe else "",
                "pe_sections": " ".join(pe[3]) if pe else "",
                "sha1": hashlib.sha1(open(p, "rb").read()).hexdigest() if a.hash else "",
            })
    cols = ["path", "size", "ext", "modified", "magic", "kind", "pe_linked", "pe_exports", "intel_cc", "pe_sections", "sha1"]
    with open(a.out, "w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=cols); w.writeheader(); w.writerows(rows)
    # ---- summary
    by = collections.defaultdict(lambda: [0, 0])
    for r in rows: by[r["kind"]][0] += 1; by[r["kind"]][1] += r["size"]
    print(f"\n{len(rows)} files, {sum(r['size'] for r in rows)/1e6:.1f} MB in {root}\n")
    print(f"{'category':28}{'files':>8}{'MB':>10}")
    for k, (n, s) in sorted(by.items(), key=lambda kv: -kv[1][1]):
        print(f"{k:28}{n:8}{s/1e6:10.1f}")
    print("\nCODE files (these are what get decompiled):")
    for r in rows:
        if r["kind"].startswith("CODE"):
            print(f"  {r['path']:40} {r['size']:>9} B  linked {r['pe_linked']}  exports {r['pe_exports']:>5}  Intel: {r['intel_cc']}")
    unk = [r for r in rows if r["kind"].startswith("UNKNOWN")]
    if unk:
        ex = collections.Counter(r["ext"] or "(none)" for r in unk)
        print("\nUnrecognised extensions:", dict(ex.most_common(15)))
    print(f"\nWrote {a.out}")

if __name__ == "__main__":
    main()
