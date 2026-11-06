#!/usr/bin/env python3
"""
symcheck.py - relocation target verification (anti-cheat oracle rule).

A byte match that ignores relocated fields only proves the instructions are the same; it does not
prove that a call goes to the same function, a load reads the same global or a string is the same
string. This module checks every relocation of a matched function against the original:

  call/jmp rel32 to symbol S     original target  == address of S
  dir32 to S (+addend)           original operand == address of S + addend
  __imp_X                        original operand is the import-table slot of X (by name)
  string literal in our object   original bytes at the target == our string (Zoo: .obj data available)

Where the address of S comes from (first hit wins):
  1. the project's address map (zoo.map / Notes/<Dll>.map) - same matching rules as compare.py
  2. the original's export table (JPOG DLLs)
  3. an address embedded in a placeholder name: f48d052, m44316b, g_638068, fn_4a0932, FUN_00401234,
     UIH_4E0EE9, DAT_00638000 (only if the value lies inside the image)

Result per reference: "ok", "bad" (points somewhere else -> the function does NOT match) or
"unverified" (the symbol's address is not known yet; reported, never counted as verified).
"""
import re, struct

TYPE_DIR32, TYPE_DIR32NB, TYPE_REL32 = 6, 7, 0x14

class PEInfo:
    def __init__(self, path):
        self.path = path
        d = self.d = open(path, "rb").read()
        pe = struct.unpack_from("<I", d, 0x3c)[0]
        nsec = struct.unpack_from("<H", d, pe + 6)[0]
        optsz = struct.unpack_from("<H", d, pe + 20)[0]
        self.base = struct.unpack_from("<I", d, pe + 24 + 28)[0]
        self.size = struct.unpack_from("<I", d, pe + 24 + 56)[0]
        so = pe + 24 + optsz
        self.secs = [struct.unpack_from("<8sIIII", d, so + 40 * i) for i in range(nsec)]
        dd = pe + 24 + 96
        self.exports = {}                     # name -> va
        erva, _ = struct.unpack_from("<II", d, dd)
        if erva:
            eo = self.r2o(erva)
            nn, af, an, ao = struct.unpack_from("<IIII", d, eo + 24)
            for i in range(nn):
                n = struct.unpack_from("<I", d, self.r2o(an) + 4 * i)[0]
                s = d[self.r2o(n):self.r2o(n) + 600].split(b"\0")[0].decode("latin1")
                od = struct.unpack_from("<H", d, self.r2o(ao) + 2 * i)[0]
                self.exports[s] = self.base + struct.unpack_from("<I", d, self.r2o(af) + 4 * od)[0]
        self.imports = {}                     # IAT slot va -> (dll, name)
        irva, _ = struct.unpack_from("<II", d, dd + 8)
        if irva:
            o = self.r2o(irva)
            while o is not None:
                ilt, _, _, name_rva, iat = struct.unpack_from("<IIIII", d, o)
                if not (ilt or iat):
                    break
                dll = d[self.r2o(name_rva):self.r2o(name_rva) + 260].split(b"\0")[0].decode("latin1")
                lo = self.r2o(ilt or iat); k = 0
                while lo is not None:
                    e = struct.unpack_from("<I", d, lo + 4 * k)[0]
                    if not e:
                        break
                    if e & 0x80000000:
                        nm = f"#{e & 0xFFFF}"
                    else:
                        no = self.r2o(e)
                        nm = d[no + 2:no + 600].split(b"\0")[0].decode("latin1") if no is not None else "?"
                    self.imports[self.base + iat + 4 * k] = (dll, nm)
                    k += 1
                o += 20
    def r2o(self, rva):
        for name, vs, va, rs, rp in self.secs:
            if va <= rva < va + max(vs, rs):
                return rva - va + rp if rva - va < rs else None
        return None
    def read(self, va, n):
        o = self.r2o(va - self.base)
        return self.d[o:o + n] if o is not None else b""
    def u32(self, va):
        b = self.read(va, 4)
        return struct.unpack("<I", b)[0] if len(b) == 4 else None
    def in_image(self, v):
        return self.base <= v < self.base + self.size

EMBED = re.compile(r"^(?:f|m|fn_|g_|FUN_|sub_|DAT_|UIH_|s_)0*([0-9a-fA-F]{6,8})(.*)$")

def identifier(sym):
    """The innermost name of a decorated or C symbol: ?f48d052@@YAXXZ -> f48d052, _g_638068 -> g_638068."""
    if sym.startswith("?"):
        m = re.match(r"\?([A-Za-z_]\w*)@", sym)
        return m.group(1) if m else None
    s = sym[1:] if sym.startswith("_") else sym
    return s.split("@")[0]

class Resolver:
    """symbol -> set of possible addresses (templates and CRT helpers exist in several copies)."""
    def __init__(self, pe, map_patterns=(), multi=None):
        self.pe = pe
        self.patterns = list(map_patterns)          # [(text, va)], compare.py semantics
        self.exact = {k: v for k, v in self.patterns}
        self.multi = multi or {}                    # exact symbol -> set(va), e.g. verified instances
    def embedded(self, sym):
        ident = identifier(sym)
        if not ident:
            return None
        m = EMBED.match(ident)
        if not m:
            return None
        h, rest = m.group(1), m.group(2)
        for cand in (h, h[:6]):
            v = int(cand, 16)
            if self.pe.in_image(v) and (cand == h and not rest[:1].isalnum() or cand != h or rest == ""
                                        or not re.match(r"[0-9a-fA-F]", rest[:1])):
                return v
        return None
    def addresses(self, sym):
        if sym in self.exact:
            return {self.exact[sym]}
        if sym in self.multi:
            return self.multi[sym]
        if sym in self.pe.exports:
            return {self.pe.exports[sym]}
        for k, v in self.patterns:
            if k and k in sym:
                return {v}
        v = self.embedded(sym)
        return {v} if v is not None else None
    def address(self, sym):
        a = self.addresses(sym)
        return min(a) if a else None

def import_name(sym):
    """__imp__GetDC@4 -> GetDC ; __imp_?tfree@@YGXPAX@Z -> ?tfree@@YGXPAX@Z"""
    s = sym[len("__imp_"):]
    if s.startswith("?"):
        return s
    s = s[1:] if s.startswith("_") else s
    return s.split("@")[0]

def check(res, sym, rtype, addend, field_va, orig_field, our_data=None, orig_target=None):
    """One relocation. field_va: address of the 4-byte field in the original; orig_field: its bytes.
    our_data: for symbols defined as data in our own object, (bytes, is_string) to compare by content.
    Returns ("ok"|"bad"|"unverified", message)."""
    pe = res.pe
    if orig_target is not None and rtype == TYPE_REL32:
        target = orig_target                   # decoded branch target (BBT may use a short form)
    elif len(orig_field) != 4:
        return "bad", f"{sym}: original field unreadable"
    else:
        v = struct.unpack("<I", orig_field)[0]
    if orig_target is not None and rtype == TYPE_REL32:
        pass
    elif rtype == TYPE_REL32:
        target = (field_va + 4 + v) & 0xFFFFFFFF
    elif rtype in (TYPE_DIR32,):
        target = v
    else:
        return "unverified", f"{sym}: relocation type {rtype:#x} not checked"
    if sym.startswith("__imp_"):
        want = import_name(sym)
        got = pe.imports.get(target)
        if got is None:
            return "bad", f"{sym}: original uses {target:#x}, not an import slot"
        return ("ok", "") if got[1] == want or got[1].split("@")[0].lstrip("_") == want else \
               ("bad", f"{sym}: original imports {got[0]}!{got[1]}")
    if our_data is not None:
        data, is_string = our_data
        if data:
            n = len(data)
            theirs = pe.read(target, n)          # target already includes the addend
            if theirs == data:
                return "ok", ""
            return "bad", f"{sym}: data differs ({data[:24]!r} vs {theirs[:24]!r})"
        return "unverified", f"{sym}: local data without content"
    cands = res.addresses(sym)
    if not cands:
        return "unverified", f"{sym}: address unknown"
    wants = {(a + (addend if rtype == TYPE_DIR32 else 0)) & 0xFFFFFFFF for a in cands}
    if target in wants or (rtype == TYPE_REL32 and addend and target in {(a + addend) & 0xFFFFFFFF for a in cands}):
        return "ok", ""
    w = sorted(wants)
    if sym in res.multi and sym not in res.exact:
        return "unverified", f"{sym}: target {target:#x} is not one of the {len(w)} recorded copies"
    return "bad", f"{sym}: expected {w[0]:#x}{' (or %d other copies)' % (len(w) - 1) if len(w) > 1 else ''}, original references {target:#x}"

# ------------------------------------------------------------------ Zoo (COFF .obj, compare.py hook)
def zoo_multi(map_path, pe):
    """Extra known addresses: CRT functions (Notes/libmatch.csv) and verified template copies
    (Notes/instances_*.csv rows '@self:<symbol>' with status MATCH)."""
    import csv, glob, os
    notes = os.path.dirname(map_path)
    multi = {}
    p = os.path.join(notes, "libmatch.csv")
    if os.path.exists(p):
        for r in csv.DictReader(open(p, newline="")):
            if r.get("status", "").startswith("MATCH"):
                for n in r["name"].split(" / "):
                    multi.setdefault(n.strip(), set()).add(int(r["va"], 16))
    for p in glob.glob(os.path.join(notes, "instances_*.csv")):
        for r in csv.DictReader(open(p, newline="")):
            k = r.get("key", "")
            if k.startswith("@self:") and r.get("status") == "MATCH":
                multi.setdefault(k[6:], set()).add(int(r["va"], 16))
    return multi

class ZooChecker:
    """Installed as compare.SYMCHECK; called for every instruction the CFG walk accepted."""
    def __init__(self, compare_mod, pe_path, map_path):
        self.C = compare_mod
        self.pe = PEInfo(pe_path)
        pats = []
        import os
        extra = os.path.join(os.path.dirname(map_path), "symbols.map")   # known addresses, not written yet
        for mp in [map_path] + ([extra] if os.path.exists(extra) else []):
            for ln in open(mp, errors="replace"):
                ln = ln.split("#", 1)[0].strip()
                if "=" in ln:
                    k, val = ln.rsplit("=", 1); val = int(val.strip(), 0)
                    pats.append((k.strip(), val if val >= self.pe.base else val + self.pe.base))
        self.res = Resolver(self.pe, pats, zoo_multi(map_path, self.pe))
        self.reset()
    def reset(self):
        self.ok = 0; self.unverified = []; self.bad = []
    def our_data(self, obj, si, addend):
        name, value, secnum, scl = obj.syms.get(si, (None, 0, 0, 0))
        if secnum <= 0:
            return None
        S = obj.sections[secnum - 1]
        if S["ch"] & 0x20:                       # code section: our own function, check by address
            return None
        raw = S["raw"]; start = value + addend
        if name and (name.startswith("??_C@") or name.startswith("$SG")):
            end = raw.find(b"\0", start)
            return (raw[start:end + 1] if end >= 0 else raw[start:start + 64]), True
        # other local data: compare up to 16 bytes that carry no relocation of their own
        n = 0
        while n < 16 and start + n < len(raw) and (start + n) not in S["rels"]:
            n += 1
        return (raw[start:start + n] if n >= 4 else b""), False
    def __call__(self, obj, sec, oi, gi):
        S = obj.sections[sec]
        for j in range(len(oi.b)):
            va = oi.addr + j
            if va not in S["rels"]:
                continue
            si, typ = S["rels"][va]
            name = obj.syms.get(si, ("?",))[0]
            addend = struct.unpack_from("<I", S["raw"], va)[0]
            # BBT re-encoded [reg+0] -> [reg]: fields after the dropped byte shift by one
            gj = j if len(oi.b) == len(gi.b) else j - 1
            field = gi.b[gj:gj + 4]
            od = self.our_data(obj, si, addend)
            if od is not None:
                od = (od[0], od[1])
            tgt = gi.target if (typ == TYPE_REL32 and gi.kind in ("call", "jmp", "jcc")) else None
            st, msg = check(self.res, name, typ, addend, gi.addr + gj, field, od, tgt)
            if st == "ok": self.ok += 1
            elif st == "bad": self.bad.append(msg)
            else: self.unverified.append(msg)
        if self.bad:
            raise self.C.Mismatch("relocation target differs: " + self.bad[0])
    def tail(self, obj, sec, oi, g):
        """Our tail jump to another function that BBT placed at original address g."""
        S = obj.sections[sec]
        rel = S["rels"].get(oi.addr + 1)
        if rel is None:
            return
        name = obj.syms.get(rel[0], ("?",))[0]
        a = self.res.address(name)
        if a is None:
            self.unverified.append(f"{name}: tail-jump target address unknown")
        elif a != g:
            raise self.C.Mismatch(f"relocation target differs: tail jump to {name} ({a:#x}), original continues at {g:#x}")
        else:
            self.ok += 1

# ------------------------------------------------------------------ JPOG (dumpbin /relocations text)
REL_LINE = re.compile(r"^\s*([0-9A-F]{8})\s+(DIR32NB|DIR32|REL32|SECREL|SECTION)\s+([0-9A-F]+)\s+[0-9A-F]+\s+(\S+)")
SYM_LINE = re.compile(r"^[0-9A-F]{3,}\s+([0-9A-F]{8})\s+SECT([0-9A-F]+)\s+\w+\s+\(\)\s+(External|Static)\s+\|\s+(\S+)")
TYPES = {"DIR32": TYPE_DIR32, "DIR32NB": TYPE_DIR32NB, "REL32": TYPE_REL32, "SECREL": 0xB, "SECTION": 0xA}

def jpog_relocs(rel_path):
    """{function name: [(offset in function, type, addend, symbol)]} from NAME.rel."""
    funcs, rels, cur = {}, {}, None
    for ln in open(rel_path, encoding="latin1"):
        m = SYM_LINE.match(ln)
        if m:
            funcs.setdefault(int(m.group(2), 16), []).append((int(m.group(1), 16), m.group(4))); continue
        m = re.match(r"\s*RELOCATIONS #([0-9A-F]+)", ln)
        if m:
            cur = int(m.group(1), 16); continue
        m = REL_LINE.match(ln)
        if m and cur is not None:
            rels.setdefault(cur, []).append((int(m.group(1), 16), TYPES[m.group(2)], int(m.group(3), 16), m.group(4)))
    out = {}
    for sn, lst in rels.items():
        fl = sorted(funcs.get(sn, []))
        for off, typ, add, sym in lst:
            owner = None
            for v, n in fl:
                if v <= off: owner = (v, n)
            if owner:
                out.setdefault(owner[1], []).append((off - owner[0], typ, add, sym))
    return out

def jpog_check_function(res, func_va, relocs):
    """-> (ok, [bad messages], [unverified messages]) for one matched JPOG function."""
    ok, bad, unv = 0, [], []
    for off, typ, add, sym in relocs:
        fva = func_va + off
        st, msg = check(res, sym, typ, add, fva, res.pe.read(fva, 4))
        if st == "ok": ok += 1
        elif st == "bad": bad.append(msg)
        else: unv.append(msg)
    return ok, bad, unv
