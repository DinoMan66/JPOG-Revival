# CLAUDE: START HERE (JPOG / Toshi matching decompilation)

Read this file first in every session. It replaces re-reading old conversations.
Goal: byte-matching C++ for the 18 code files of Jurassic Park: Operation Genesis (`Original\DLLS\*_inst.dll`,
`Original\SimJP.exe`). Usage limits are the bottleneck, so work in a cheap, scripted way.

## ANTI-CHEAT RULES (read every session; they override anything else in this file)

A function counts only when it is real, readable C++ that the real compiler turns into the original's code.
Checked by the user's independent anti-cheat (`C:\AntiCheat\anticheat.py`, outside your reach) - a
violation fails the whole check, so there is nothing to gain from any of these:

1. **Forbidden in Src\**: inline assembly (`__asm`, `_asm`, `asm`), naked functions, `_emit`, byte arrays
   executed as code, code/data placement pragmas (`code_seg`, `data_seg`, `section`, `init_seg`,
   `alloc_text`), `#pragma comment(linker ...)`, `__declspec(allocate)`, `/alternatename`, hard-coded
   addresses cast to pointers or called (`(T*)0x638148`, `((void(*)())0x401000)()`), `#include` of anything
   that is not a header, and any .obj/.lib/.asm/.bin file. (Functions the original wrote in assembly
   are listed in the ORIGINAL ASSEMBLY section below - ask the user before touching them.)
2. **Compiler flags** (`flags.txt`, `*.cpp.flags`) only from the allowed list: /D /U /I (inside the project or
   C:\stlport), /O.., /G.., /W.., ICC -Q options for code generation. Never /B1 /B2 /Fo /link -Qlocation -Qoption.
3. **Never modify the verification tools or build scripts** (protected, hash-locked):
   Tools\compare.py, symcheck.py, verdict.py, triage_lib.py, scoreall.py, unbundle.py, varcmp.py,
   build_core.bat, build.bat, watch.bat, Original\*. If one needs a change, write the proposal to
   `Tools\proposed\<file>` and explain it to the user; the user reviews it and re-baselines. Never edit
   files in Results\, and never use Tools\queue\cmd_*.bat to change compilers (the build refuses them).
4. **A match needs the right targets.** verdict.py and the anti-cheat check every relocation: the call must go to the same
   function, the load must read the same global, the string must be the same string (Tools\symcheck.py).
   "relocation target differs" is a MISMATCH, never something to explain away.
5. **No justifications.** Comments or notes that argue a difference is acceptable do not change the result -
   a function verifies or it does not. Do not widen tolerances, exclude functions or relabel statuses.
6. **Honest progress.** Report progress only from `Notes\VERIFIED_PROGRESS.md` (written by the anti-cheat).
   Placeholder code (Z_<va>, UIH_<va>, f<va>/m<va>/g_<va>, m_unkXX, pad[], dummy virtuals), compiler-generated
   copies and Microsoft library code are separate tiers and are never reported as reconstruction.
   Prefer giving real names (class, method, member) over generating more placeholders.
7. **Readability standard** (`Notes\READABILITY_STANDARD.md`, calibrated against sm64/oot/mm/tp, enforced by
   the anti-cheat). Hard rules: plain text, consistent formatting (one statement per line), comments on
   non-obvious logic, the game's code written plainly. NOT allowed: types invented for one function and named
   after it (`struct Z_4DA485 { void f(); }`), one-letter methods (`X::f()`), `c0`/`v12` members, generator
   output (never let Tools\gentrivial.py / genshapes.py write into Src\ - analysis output into Notes\ only),
   macros that stamp out function bodies, verification scaffolding (probes, reference units of invented
   globals/classes, files that only force template instantiations).
   ALLOWED as honest placeholders ("decompiled, not fully documented"): offset-named members with an offset
   comment (`/* 0x35C */ bool m_unk35C;`), address-named functions/globals whose purpose is unknown
   (`UIH_4D5A49`, `f48d052`, `g_638068`), unknown vtable slots in a real class (`vf3`). Replace them with real
   names as soon as the purpose is known (renaming members/locals/non-exported names never changes the code; recorded names - JPOG exports, strings - are kept verbatim). Label a deliberate fakematch
   `// FIXME: fakematch`. If a function can only be matched with not-allowed code, do not commit it: add a note
   to `Notes\removed\` and move on. Do not re-add files listed in `Notes\READABILITY_REVIEW.md`; the old
   versions of deleted files are kept (reference only) in `Notes\removed\*.md`.

## Files to know (read only what the task needs)
| File | What |
|---|---|
| `Notes\MATCHING_NOTES.md` | Compiler habits + codebase rules learned so far (numbered). Read before writing code. |
| `Notes\COMPILER_LIMITED.md` | Functions that our ICC build 010525Z cannot match. **Never retry these.** |
| `Notes\function_tracker.csv` | Every exported function of all 19 binaries (18 DLLs + SimJP.exe): dll, rva, size, name, status |
| `Notes\attempts.csv` | Written by verdict.py: attempts per function, category, status |
| `Notes\READABILITY_STANDARD.md` | **What counts as readable source** - every file in Src must meet it |
| `Notes\READABILITY_REVIEW.md` | 2026-10-27 review: files removed / pending your decision; notes in `Notes\removed\` |
| `Notes\VERIFIED_PROGRESS.md` | **The only progress numbers to report** (anti-cheat output, honest tiers) |
| `Notes\classes.csv` / `classes.md` | Tools\classmap.py: 1,024 vtables in all 19 binaries - class, base class, ctors/dtors, every slot's exported name, slots done |
| `Tools\show.py <Dll> <name or rva>` | One function at once: tracker/attempts rows, Ghidra C, annotated disassembly. Use it instead of grepping |
| `Notes\generated\` | From trivial.py: `trivial_<Dll>.cpp`, `clusters.md`, `summary.txt` |
| `decompiled\<Dll>_inst.dll.c` | Ghidra output. Large - grep for the function, never read whole |
| `Results\ALL.txt` | Build bundle (10+ MB). Never read it - feed it to scoreall.py |

## The cheap workflow (do it this way)
1. **Choose work from `Notes\generated\` first**, before opening Ghidra output:
   - `clusters.md`: "FREE WIN" clusters already have a matched member - copy its source to the others.
     Other clusters: write one function, reuse the shape for the rest.
   - `trivial_<Dll>.cpp`: ready-made bodies for getters/setters/constants/empty functions. Paste into the
     class's .cpp, rename `m_unkXX` to the real member (offset is in the comment).
2. **Batch**: put a whole class or .cpp file into one build round, not one function per round.
3. **Build round** (the user's XP VM runs `Tools\watch.bat`):
   - commit the changed `Src\<Dll>\*.cpp` / `Src\include\*.h`;
   - commit `Tools\queue\request.txt` containing `round-NNN <unix time> <Dll>` (write it from a uniquely named
     local file - the bridge can serve a cached copy of a same-named file);
   - check `Results\DONE.txt` with device_list_dir until its mtime is newer than the request, then stage
     `Results\ALL.txt` only.
4. **Score with one command** and read nothing else:
   `python3 -I Tools/scoreall.py <staged ALL.txt> <Dll>`
   It prints one summary line per DLL, then only functions that changed:
   - `MATCH` done. `DIFF` open, with 2 instructions of context and a category hint.
   - `FILED` / `LIMIT` = compiler-limited. **Stop working on it.** (Exception: the one re-check described under "When a function won't match".)
   - `HUMAN` = source-type difference that survived 4 attempts: ask the user, don't keep trying.
   Categories: encoding, regalloc, sched, shrink-wrap, inline, fp (compiler-limited after 2 attempts);
   stack (reorder local declarations), branch (swap if/else bodies), outlined (missing `inline`?), source.
5. Commit back the updated `Notes\function_tracker.csv`, `Notes\attempts.csv`, `Notes\COMPILER_LIMITED.md`.
6. New habits learned -> one numbered line in `MATCHING_NOTES.md`. Keep it short.
7. End of session: update "Current state" below (numbers + next step). Re-run
   `python3 -I Tools/trivial.py jpog` when many functions changed status.

## ORIGINAL ASSEMBLY (the game itself contains hand-written code here)
- TRenderD3DInterface 0x1002D01D, 0x100317CD, 0x10032445: MMX routines; 0x10038B87: CPU detection (cpuid).
- SimJP.exe: sections `.cms_t`/`.cms_d` (same names SecuROM copy protection uses) and `PSGSFD00`..`PSGSFD_B`
  (unidentified, ~15 KB) are not game code - exclude them.
These are the only places where `__asm` could be legitimate. Ask the user first.

## Code-rules profile (where names come from, what may change bytes)
- Compiler: Intel C++ 5.0.1 (build 010525Z). No RTTI. Every DLL exports its functions by decorated name, so the
  class, function name and parameter types of exported functions are RECORDED: use them verbatim (llvm-undname),
  never rename or "fix" them - that changes the export table. Members, locals and non-exported helpers are free.
- Other evidence: source paths in strings `Z:\Toshi\Source\<Module>\<File>.cpp`, `Z:\Toshi\Plugins\Source\...`,
  `Z:\Projects\SimJP\Source\...` (unit names and folders - keep our Src layout in line with them), `Class::method`
  strings, exported vtables (`??_7`). SimJP.exe also exports its functions by name (13,664 functions + 1,775 data exports).
- Style: Toshi Hungarian-style members (`m_uiMagic`, `m_pChild`, `m_bEnabled`, `m_fScale`), classes prefixed
  `T` (engine), `P` (plugins), `A` (SimJP game). Follow the existing headers.
- Placeholders allowed: `m_unkXX` (offset), `_SlotNN` / `SlotN` vtable slots - name slots from the D3D DLL exports
  (`python3 -I Tools/classmap.py Original/DLLS/D3D/*.dll Original/DLLS/TRenderD3DInterface_inst.dll --slots TSkinShader`: slot N of the HAL class = the placeholder `_SlotN`; e.g. TSkinShader slot 12 = SetSunTheta, 13 = SetSunIntensity).
- `__FILE__` strings must equal the original path: `#line N "Z:\\Toshi\\Source\\TRender\\TFoo.cpp"`.

## When a function won't match: classify the wall first (in this order)
A pinned compiler is deterministic, so a remaining difference means an input differs. Do not call a wall class N
while class N-1 still diverges (adapted from lemiur's wall-identifier):
1. **Referent** - instructions identical, but a call/global/string target differs (symcheck note in verdict.py).
   Fix the identity/label. Never "fix" this with codegen tricks.
2. **Call set** - the set of out-of-line callees differs (verdict categories `inline` / `outlined`). Usually the
   caller's body is incomplete, an inline boundary is wrong, or a helper is open-coded. Finish the caller first.
3. **Control flow** - block/branch/return counts differ (`branch`). A source construct: separate returns vs one
   exit, `while` vs `do`, `||`/`&&` collapse, switch shape.
4. **Register / schedule** (`regalloc`, `sched`, `stack`) - only after 1-3 agree. Check in turn: variable widths
   (`unsigned char` vs `bool`/`int`), one local per meaning, declaration and first-use order, the scope/lifetime
   of temporaries, inline-helper boundaries. Only then is it compiler-limited.
**Helper recovery:** when code reaches into another class's internals, repeats a field path or has a scoped
`{ }` block, the original probably called a small accessor/inline/macro. Restore it in the owning class's header
only with evidence (the same pattern at several sites, an existing helper, a string), apply it at every site, and
rebuild: exact functions must stay exact. This is real source even when no score moves.
**Compiler-limited list:** entries filed before 2026-10-28 only because of regalloc/sched get ONE re-check against
step 4 and helper recovery (one attempt each, note the result in COMPILER_LIMITED.md); after that they are final.
**Fakematches:** if a match needs `volatile`, a dummy local, an empty block, `do{}while(0)`, `+ 0` etc., it is not a
match (MATCH-PENDING in the anti-cheat report). Use the trick as a clue to the real source, then remove it.

## Rules that save usage
- Re-read the ANTI-CHEAT RULES and this section at the start of every build round - rules fade as the
  context fills (momo5502). Keep context small: one file/class per round, fresh session for a new area.
- Routine rounds (trivial bodies, clusters, renames, helper application) run fine on a cheaper model; the
  harness (verdict.py + anti-cheat) is what keeps them honest. Save the strongest model for walls and reviews.
- Retry budget is 2 real attempts per function (verdict.py counts them), each one aimed at the wall class above. Don't hand-craft a 3rd variant of a
  regalloc/sched difference - it is almost always the compiler build (see MATCHING_NOTES #23).
- Flag sweeps run unattended in the VM (Results\sweep*). Read only their log summary, never the outputs.
- Don't read whole dumpbin, ALL.txt or Ghidra files; grep/slice by function name or address.
- Tools need `capstone` (pip) or GNU objdump; trivial.py needs `llvm-undname` (Claude's side) or MSVC `undname`
  for signatures - without it, bodies are still generated with decorated names.

## Current state (2026-10-27)
- **Mismatch clean-up (2026-10-28, rounds 85-88):** fixed the wrong-target match (PutPropertyUnique) and 7 more:
  TQList::InsertTail through the node, iterator comparisons by value, `End()` cached in GetMaterial/GetProperty
  (MATCHING_NOTES, last 5 lines). PPropertyParser 201, TRenderInterface 923 matched; no regressions. Leftover
  experiment chains (VAR_*, TML_POS, TKF_FORM) resolved to their active branch. Tracker statuses caught up
  (297 earlier matches were still 'todo'). Open: TQLIST_FORM (two forms of TQList.h in different files); ICC
  branch-layout differences don't follow the source spelling (don't spend attempts swapping if/else).
  Remaining differences: PPropertyParser 44, TRenderInterface 60 open + 50 compiler-limited, TInput 9 + 4.
  Last build round: round-088.
- **SimJP.exe added to the tracker (2026-10-28, user's decision - keep the numbers honest):** Notes\function_tracker.csv
  now has 20,432 functions = 6,768 in the 18 DLLs + 13,664 in SimJP.exe (rows `SimJP.exe`; its 1,775 data exports
  are not functions and are left out). Still to do: 19,298. trivial.py (re-run, Notes\generated\): 4,677 trivial
  bodies, 815 identical-code clusters (52 FREE WIN) covering another 2,602, about 12,000 need real work.
  The tracker lists exported functions only, in every binary: unexported statics/library code are not counted.
  SimJP.exe source goes in `Src\SimJP\` (build request folder name `SimJP`; not built yet, so the first round may need build fixes). Tools\scoreall.py needs the reviewed change
  in Tools\proposed\scoreall.py before SimJP builds can be scored (the user applies it and re-baselines).
- Readability review (2026-10-27, calibrated standard): 106 files readable, 6 "decompiled, not fully documented"
  (TInputInterface.h m_unk04/m_unk10/m_unk34, TFileLexer.h m_pad04/m_pad09, TKernelInterface.h Slot0,
  TMaterialLibrary/TGFXFileInstanceObject _SlotNN - names recoverable from the D3D DLL exports). Only the 5 old
  copies in Src\*.cpp (root) are deleted. Documentation to-do: Notes\READABILITY_REVIEW.md.
- Anti-cheat run 2026-10-27: 917 TRenderInterface functions verified, all readable (real names), 0 wrong
  relocation targets, 0 rule violations. The other DLLs were not in the last build - re-verify with --rebuild.
- verdict.py now tolerates only real relocations (exact .rel data) and checks every relocation target.

### Earlier state (2026-10-21)
- TRenderInterface: 917/1048 exported functions matched, 65 open differences (mostly regalloc/shrink-wrap: they
  will be auto-filed on their next changed attempt), 45 known compiler-limited, 21 not written yet.
- TApplication 16/16 and TInputInterface 74 matched earlier; PPropertyParser partly written (rounds 13-26).
- Whole project: 5,634 functions still to do. trivial.py: 1,267 trivial (C++ generated), 371 identical-code
  clusters cover another 1,024 (37 FREE WIN clusters, e.g. 99x `CreateObjectInPlace` = copy from
  TCameraObject). About 3,300 need real work.
- Next steps: (1) the 21 unwritten TRenderInterface functions; (2) FREE WIN clusters in clusters.md;
  (3) paste trivial functions DLL by DLL, starting with the DLL you work on next.
- Last build round: round-084.
