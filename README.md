# Jurassic Park: Operation Genesis — matching decompilation

A work-in-progress **matching decompilation** of *Jurassic Park: Operation Genesis* (2003, Blue Tongue
Entertainment / Universal Interactive) for PC: the Toshi engine DLLs and the game itself, `SimJP.exe`.

Hello everyone. This is my first decompilation project. While ZT1 Open Source was published first, I started game decompilation here, with Jurassic Park Operation Genesis, one of my favorite games ever. I'm new to the decompilation scene and am still trying to improve my set up. Bear with me while I get the hang of things.
The reason why I've named this decompilation JPOG Revival is because I hope to revive this classic game and get it running on modern systems, implement several new features, and add back cut content.
Has nothing to do with the JPOG Revival series of mods for Jurassic World Evolution 3.

The goal is readable C++ source that, compiled with Intel C++ 5.0.1, produces **the same machine code as the
original, function by function**. A function only counts when it compiles to identical instructions *and*
every call, global and string it references points to the same target as in the original.

> **This repository contains no game code, assets or binaries.** You need your own legal copy of the game.
> Not affiliated with or endorsed by Universal, Vivendi or Blue Tongue Entertainment.

## Status

Progress is measured by an independent verification tool, never by hand
(see [`Notes/VERIFIED_PROGRESS.md`](Notes/VERIFIED_PROGRESS.md)). As of 2026-10-29:

| | Functions | Share of 20,432 |
|---|---:|---:|
| Readable reconstruction (matched, real names) | 1,895 | 9.3% |
| Matched, honest placeholder names | 0 | 0% |
| Not yet matching (attempted) | 331 | 1.6% |

The 20,432 functions cover all 18 DLLs (6,768 functions) **and** `SimJP.exe` (13,664 functions);
the full list is in [`Notes/function_tracker.csv`](Notes/function_tracker.csv).

## Target

| | |
|---|---|
| Files | `SimJP.exe` plus 18 Toshi/plugin DLLs (`TKernelInterface`, `TRenderInterface`, `TRenderD3DInterface`, `TInputInterface`, `PPropertyParser`, `TGuiInterface`, ...) |
| `SimJP.exe` SHA-1 | `3a4deb24af734c42ee20e4c96be288316c9afac4` |
| Compiler | Intel C++ 5.0.1 (the original used build 020125Z; we use 010525Z, see `Notes/COMPILER_LIMITED.md`) |

Every DLL and `SimJP.exe` exports its functions by decorated C++ name, so class names, function names and
parameter types come straight from the binaries and are kept exactly as recorded.

## Building and verifying

You need:
- Your own copy of the game: `SimJP.exe` in `Original/`, the DLLs in `Original/DLLS/`
- Intel C++ 5.0.1 for Windows (with a Visual C++ 6.0 install for headers and libraries); a Windows XP VM works well
- Python 3 with `capstone` (`pip install capstone`) on the machine that compares

Steps:
1. Source goes in `Src/<DLL>/*.cpp`, shared headers in `Src/include/`.
2. Build: run `Tools/watch.bat` in the VM, or `Tools/build.bat` directly. Output is bundled into `Results/ALL.txt`.
3. Verify: `python Tools/scoreall.py Results/ALL.txt <DLL>` — prints a summary per DLL, then `MATCH` / `DIFF`
   per changed function, with a hint at why a function differs.

Handy tools:
- `Tools/show.py <Dll> <name|rva>` — everything about one function at once: tracker status, Ghidra output, annotated disassembly
- `Tools/classmap.py` — classes, base classes and vtable slot names recovered from all 19 binaries
- `Tools/trivial.py` — finds trivial functions and groups of identical functions

## Repository layout

| Path | Contents |
|---|---|
| `Src/` | The reconstructed source, one folder per DLL |
| `Tools/` | Build scripts, comparison and analysis tools |
| `Notes/function_tracker.csv` | Every function with its status |
| `Notes/MATCHING_NOTES.md` | Compiler habits learned so far |
| `Notes/COMPILER_LIMITED.md` | Functions our compiler build cannot reproduce |
| `Notes/READABILITY_STANDARD.md` | What counts as acceptable source |
| `Notes/classes.csv` | 1,024 vtables, 758 of them named |

## Standards

- **Readable source only.** No generated code, no macro-stamped functions, no invented one-off types, no
  inline assembly. Unknown members get honest placeholder names (`m_unk04`) until their purpose is known.
  The standard is calibrated against established projects such as [zeldaret/oot](https://github.com/zeldaret/oot),
  [zeldaret/tp](https://github.com/zeldaret/tp) and [n64decomp/sm64](https://github.com/n64decomp/sm64).
- **No fakematches.** A function that only matches through a trick (`volatile`, dummy locals, empty blocks)
  is not counted.
- **Right targets, not just right bytes.** Every relocation is checked; byte-identical functions that call the
  wrong function are reported as mismatches.

## How AI is used

Much of the reconstruction is done with an AI assistant (Claude), working through the same build-and-verify
loop as a human would. Because of that, nothing is taken on trust:
- every function is compiled and compared against the original, including its call/data targets;
- progress figures come only from an independent verification tool the AI cannot modify;
- generated or placeholder code is reported separately and never counted as reconstruction.

## Related projects

These take different approaches and complement this one:
- [AdventureT/OpenJPOG](https://github.com/AdventureT/OpenJPOG) — decompilation of the Toshi engine and JPOG
- [Voikanaa/jpog-decomp](https://github.com/Voikanaa/jpog-decomp) — portable C++20 reimplementation, tested against the original DLLs

## Credits

I would like to give a big thanks to bwrsandman for finding Intel C++ Compiler Version 5.0.1 Build 010525Z. I wouldn't have been able to get this project off the ground without him finding the compiler I'm using. It isn't 020125Z unfortunately (the compiler build Jurassic Park Operation Genesis was built with), but so far I have a match rate of ~%90. So long as I'm unable to find build 020125Z I won't be able to match all functions, but I can get pretty close.
OpenToshi, OpenJPOG, and OpenBarnyard were also used as reference materials.

## Legal

This project contains only original source written to reproduce the game's behaviour, plus tools and notes.
It contains no copyrighted game files, and none are distributed. Jurassic Park is a trademark of Universal
City Studios and Amblin Entertainment. This project is non-commercial.

## License

CC0 1.0 Universal. You can do anything you want with the source code, no strings attached. I'd still like it if you'd credit me should you use it for something. Not a requirement, of course.
