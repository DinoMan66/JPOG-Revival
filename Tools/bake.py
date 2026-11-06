#!/usr/bin/env python3
"""Resolve '#if defined(VAR_X) ... #elif ... #else ... #endif' experiment chains in a source file.

usage: bake.py FILE LETTER [--at TEXT]
Keeps the branch that VAR_<LETTER> would select (or the #else branch for LETTER 'none') in the
first chain at/after TEXT (default: every chain in the file whose conditions mention VAR_).
Nested non-VAR #if blocks are left alone."""
import re, sys

def resolve(lines, i, letter):
    """lines[i] is the opening '#if' of a VAR chain; return (new_lines, end_index)."""
    depth, branches, cur = 0, [], None
    j = i
    while j < len(lines):
        st = lines[j].strip()
        if st.startswith("#if"):
            depth += 1
            if depth == 1:
                cur = [st, []]; branches.append(cur); j += 1; continue
        elif st.startswith("#elif") and depth == 1:
            cur = [st, []]; branches.append(cur); j += 1; continue
        elif st == "#else" and depth == 1:
            cur = ["#else", []]; branches.append(cur); j += 1; continue
        elif st.startswith("#endif"):
            depth -= 1
            if depth == 0:
                break
        cur[1].append(lines[j]); j += 1
    def selected(cond):
        if cond == "#else": return True
        names = re.findall(r"defined\((VAR_\w+)\)", cond)
        neg = cond.startswith("#if !") or "!(" in cond
        hit = ("VAR_" + letter) in names
        return (not hit) if neg else hit
    for cond, body in branches:
        if selected(cond):
            return body, j
    return [], j

def main():
    path, letter = sys.argv[1], sys.argv[2]
    at = sys.argv[sys.argv.index("--at") + 1] if "--at" in sys.argv else None
    lines = open(path).read().split("\n")
    out, i, started = [], 0, at is None
    done = False
    while i < len(lines):
        if not started and at in lines[i]:
            started = True
        st = lines[i].strip()
        if started and not done and st.startswith("#if") and "VAR_" in st:
            body, j = resolve(lines, i, letter)
            out.extend(body); i = j + 1
            if at is not None: done = True
            continue
        out.append(lines[i]); i += 1
    open(path, "w").write("\n".join(out))

if __name__ == "__main__":
    main()
