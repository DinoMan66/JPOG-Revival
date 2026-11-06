# Hidden-option sweep of ICC 5.0.1 (build 010525Z): results

Diagnostic only. These are mcpcom's internal options, passed as `-Qoption,c,-m<NAME>[=value]`.
The anti-cheat rules forbid `-Qoption` in flags.txt / *.cpp.flags, so nothing here can be used in Src\.
The sweep's only purpose is to find out *why* our build differs from the original (build 020125Z).

## Coverage (passes 1-5, 2026-10-18)
- Every option mcpcom accepts (sweep_opts.txt, sweep_opts4.txt). Boolean options were tried TRUE and FALSE;
  integer options 0/1/2/4/64/1024; the inlining limits at 3-200 (sweep_vals5.txt).
- Test files: TViewport.cpp and TScene.cpp (passes 3-4); TViewport, TMaterialLibrary and TRenderInterface.cpp (pass 5).
- Scored on 2026-10-28 against TRenderInterface_inst.dll, per exported function, against the same
  build without the option (BASE).

## Result: no option creates a single new match
No option and no value made a mismatching function match. The ones that move things in the right direction:

| Option | Effect (differing bytes, BASE -> option) | Losses |
|---|---|---|
| `P3OPT_pcg_ra_optim_color`, `P3OPT_pcg_ra_region_picking=0` (same output) | TViewport::~TViewport 210->83, DeleteHandlers 51->26, TRenderAdapter::~TRenderAdapter 102->44 | none in TViewport; 2 matches lost in TScene |
| `P3OPT_fast_float_to_int` | TRenderInterface::MakeRGBA 197->93 | none |
| `IPOPT_inline_max_total_stats=32` | ~TRenderInterface 283->205, CreateResource 234->127, CreateResourceInPlace 265->138, SetAsLastChild 218->98, ~TRenderAdapter 102->30 | 1-2 matches lost per file |
| `IPOPT_inline_max_total_stats=16..24` | also ~TMaterialLibraryManager 75->15, TMesh/TMaterial/TShader/TNullResource ctors | 5-11 matches lost per file |
| `IPOPT_inline_max_total_stats=3..8` | biggest gains on the dtors | 5-19 matches lost per file |

Everything else is either identical to BASE or only worse (e.g. `P2OPT_pgo_gen`, `P3OPT_debug_gz` lose ~200 matches).

## What this says about the compiler
- The register allocator (`pcg_ra_*`) and the inliner's budget (`inline_max_total_stats`) are where our
  build and the original part ways. That fits the two biggest compiler-limited groups in
  COMPILER_LIMITED.md: **regalloc/sched** and **same-file inlining**.
- No single setting reproduces the original, so the difference is most likely in the compiler code itself
  (build 020125Z), not in a default value.

## Next pass (prepared, not run)
`cmd_s3_combo.bat` combines the helpful options (A = ra_optim_color, B = ra_region_picking=0,
C = fast_float_to_int, D = inline_max_total_stats=32) and compiles every TRenderInterface source with
relocations, so each combination gets an exact score. It needs the user's permission to run commands
in the VM (`C:\anticheat_allow_cmd.txt`).
