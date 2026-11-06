# JPOG / Toshi matching notes

## Target
- Installed game files (NOT the `Setup\` folder - that is an older, Jan-14-2003 build set).
- 18 code files link-dated 2003-02-07 (engine DLLs + `SimJP.exe`); third-party: binkw32, fmod, libmmd.
- Source paths live inside each DLL's compiler banner (see `JPOG_source_files_installed.txt`), e.g.
  `Z:\Toshi\Source\TApplication\TApplication.cpp`, `Z:\Toshi\Projects\<Name>\Win\Final\<Name>.pdb`.

## Toolchain (verified: TApplication.dll 20/20 matching)
- Intel C++ 5.0.1 build 010525Z (game was built with 020125Z - no visible difference so far) + Visual C++ 6.0.
- Flags: `-Qvc6 -nologo -Gz -MD -W3 -vmg -vmm -Zi -O2 -D TOSHI_SKU_WINDOWS -D WIN32 -D _WINDOWS
  -D _UNICODE -D TOSHI_FINAL -D NDEBUG -D USER_JPBUILD` (per-DLL differences are listed in the build notes).
- Workflow: write C++ on the PC -> `tools\build.bat` in the XP VM -> copy `results\*.txt` back ->
  `py tools\compare.py original\X.dll results\X.txt --map notes\X.map --tracker notes\function_tracker.csv`.

## Compiler habits learned
1. **Exported classes are `__declspec(dllexport)`.** Evidence: the DLL exports the vtable (`??_7...`) and every
   member. Effect on codegen: the compiler emits the *vector deleting destructor* (`??_E`) in vtable slot 0.
   Without it you get the scalar one (`??_G`), which will not match. The array operators / array new alone do not
   trigger it.
2. **`-Gz` => free functions are `__stdcall`** (`YG` in decorated names). `operator new/delete` stay `__cdecl`.
3. **Toshi-DLL imports are `dllimport`** (`call dword ptr [__imp_...]`). Declare imported members with
   `__declspec(dllimport)` (macro `TOSHI_API`), either per member or per class.
4. **Boolean tests are written `x == true`** (`cmp al,1` in the binary), and "return" shapes follow
   `if (!x) return false; return true;`.
5. **By-value class parameters are destroyed by the callee** (`ret 8` + destructor call inside the function).
6. **Inlined smart-pointer code re-checks null after storing** (extra `je` right after the store). Reproduced by
   `if (m_p) m_p->m_refCount++;` inside the "if (o.m_p)" branch.
7. **Replacement `operator new/delete` are defined per DLL** (non-exported): `tmalloc(size, 0, -1)` / `tfree(p)`.
   Define them before first use so the constructor and Create() inline the call.
8. Loops and branch targets are 16-byte aligned with multi-byte NOPs (`lea esi,[esi+0]`) - automatic.

## Tooling pitfalls we hit
- A stray `X.h` next to `X.cpp` silently overrides `include\X.h` (quoted includes search the .cpp's folder first).
- `dumpbin /disasm` shows no symbol for calls inside the same object -> `compare.py` detects `E8` calls by opcode.
- Copying a folder into a folder with the same name creates `Tools\tools\...`; keep the layout flat.

## More habits (TInputInterface / TInputDXInterface, Oct 2026)
9. **Small virtuals are inline in the class body** (e.g. `Flush() { return true; }`,
   `IsForceFeedbackDevice() { return false; }`, `SetVibration(...) {}`). Evidence: a derived class in
   ANOTHER DLL has its own local (non-exported) copy in its vtable instead of an import thunk.
10. **Hidden loads through `const int&`**: `TMath::Max/Min` take and return `const T&`; with a named
    static const the binary compares against memory (`cmp eax,[STICK_DEADZONE]`) while plain
    arithmetic with the same constant is folded. Static consts must be DEFINED before use in the .cpp.
11. **Real 4-byte float stack temporaries**: binding a call result to `const float&` gives `fstp dword`;
    plain `float x = f();` gives an 8-byte spill (`fstp qword`). Stack slots follow declaration order.
12. **switch statements**: case bodies come out in REVERSE source order (ascending source -> descending code).
    `tools/switchgen.py DLL RVA --order value` rebuilds `case K: return C;` switches from the jump table.
13. **Loop tests through inline bool helpers** (e.g. `DeviceList::IsValid(d)`) produce `setne/test` pre-tests;
    the same comparison written inline does not.
14. Ternaries matter: `return (f < -0.5f) ? -1 : ((f > 0.5f) ? 1 : 0);` differs from if/return chains;
    `(x == true) ? 0 : 1` gives `cmp al,1 / je` where `(x != true)` gives `setne`.
15. Out-of-range "if" bodies: ICC lays out `if (FAILED) {...}` vs `if (SUCCEEDED) {...}` differently; when
    the success path is the fall-through, write it first.
16. Unresolved (register allocation): TArray iterator loops (StartRepeat/StopRepeat/Latch/Unlatch/IsLatched,
    ProcessRepeats, SetCapacity) keep the iterator in memory with write-through in the original. Many
    variants tried (operator forms, copy ctors, named returns) without success - revisit later.

## Build automation (current)
- `Src\<Dll>\*.cpp` per DLL, shared `Src\include`. Results are `Results\<Dll>.<file>.txt/.rel` plus one
  bundle `Results\ALL.txt` (`tools/unbundle.py` splits it). `compare.py` reads exact relocations from `.rel`.
- One-shot experiments: `Src\<Dll>\VarX_<file>.cpp` = `#define VAR_X` + `#include "<file>.cpp"`; deleted by
  the build after compiling. Score them with `tools/varcmp.py DLL MAP "res/*.txt" names...`.
- `Tools\queue\cmd.bat` (optional) is run once inside the VM before compiling; output in `Results\cmd.txt`.

## PPropertyParser findings (rounds 13-18, 2026-10-17)
17. **Exported static pointer members are declared `static const T* X;` (no second const)**. ICC then mangles
    them `2PBV...@B` exactly like the original and loads them from memory at each use. With `const T* const`
    it mangles `2QBV...@B` and constant-folds the address (`push offset`), which never matches.
18. **Pooled-string copy constructors clear first**: `m_p = 0; if (o.m_p) { m_p = o.m_p; if (m_p) ++count; }`
    (the if/else form gives a different block order).
19. **Intrusive reference counts of TObject-derived classes live in a second base class** (`TRefCountBase`,
    ctor stores 0 before the vtable), like TInputInterface's +4 base. Members declared before an embedded
    list must be set in the mem-initializer list to be stored before the list's own constructor runs.
20. Ternary vs `if` inside tiny inline accessors (GetCString) made no difference - the block order there
    comes from somewhere else (still open).

21. **Toshi's pooled/localised string destructors and assignments are NOT inline functions.** The
    original calls a local out-of-line copy of ~TPCString in every translation unit (0x1bf0 in
    PProperties, 0x3190 in PPropertyName, 0x5820 in PPropertyReader, 0x76f0 in PPropertyValue), plus
    local copies of TPCString/TPWString/TLString operator= and the TPWString/TLString destructors in
    the PPropertyName, PPropertyReader and PPropertyValue units. Nothing made ICC stop inlining an
    `inline` function (tried: explicit/qualified dtor calls, late definitions after the callers,
    dllimport class, `__inline`, different body shapes). Defining the function out-of-class WITHOUT
    `inline` does it (round 25: Error/Warning/SkipBlock/ParseError dtor matched at once, 136 -> 167
    after round 26). The assignments are inlined in the PProperties/PPropertyBlock units (SetName,
    SetComment...) but not in the other three, so those three .cpp files define
    TOSHI_STRING_ASSIGN_NOT_INLINED before their includes. Caveat: plain definitions in a header
    would collide when linking several objects; a linkable build needs per-file copies (to do).
22. Token types (TFileLexer::TokenType) come from TFileLexerUTF16::tostring and PPropertyWriter::PutToken:
    0 EOF, 1 ';', 2 ':', 3 ',', 4 '.', 5 '*', 6 '$', 7 '[', 8 ']', 9 '{', 10 '}', 11 '(', 12 ')',
    13 '<', 14 '>', 15 '=', 0x10 IDENT, 0x11 STRING, 0x12 INTEGER, 0x13 UINTEGER, 0x14 FLOAT, 0x15 COMMENT.

## Infrastructure notes
- The device file bridge can serve a CACHED copy of a file that keeps the same name/size: always write a
  build request from a uniquely named local file (`queue/req_<round>.txt`) and judge Results by mtime,
  not by DONE.txt's content.
- Ghidra exports for every DLL are in the PC's `decompiled\` folder (Tools\export_all.bat).

## #23 Compiler build and original source layout (2026-10-17, round 39)
- Every shipped (`*_inst.dll`, 2003-02-07) DLL embeds the Intel compiler's per-object comment record:
  `Intel(R) C++ Compiler for 32-bit applications, Version 5.0.1 Build 020125Z : Z:\Toshi\Source\TRender\TAnimation.cpp : -Qvc6 -Qlocation,link,... -Qvc6 -Qlocation,link,... -nologo -Gz -MD -W3 -vmg -vmm -Zi -O2 -I Z:\Toshi -I Z:\Toshi\Include -D TOSHI_FINAL -D NDEBUG -D TOSHI_SKU_WINDOWS -D WIN32 -D _WINDOWS -D _UNICODE -D USER_JPBUILD -FoFinal/ -FdFinal/ -c`
  (PPropertyParser adds `-I Z:\Toshi\Plugins\Include -FD`).
- Our VM compiler is **5.0.1 Build 010525Z** (C:\ICC501). The game used **Build 020125Z**: same version, a later update. Flags are identical.
- Inlining differs (original never inlines one non-inline function into another: ~TKeyframeLibraryInstance->Destroy,
  TAnimation::GetSequencePtr->TSkeleton::GetSequence(int), TResource::IsValid->IsCreated, TResource::SetName->GetUId).
  No switch reproduces it (tried -Ob0/-Ob1, -Qip-, -Qip_no_inlining, -Qip_no_inlining -Ob1, #pragma auto_inline(off),
  header-inline definitions, -O1, -Os, -G5/-G6/-G7). -G6 = our default. Most likely a heuristic change between the builds.
- The non-`_inst` copies of some DLLs (e.g. TRenderInterface.dll, 2003-01-14) are a different build: `-D TOSHI_RELEASE
  -D USER_AHATT -D TOSHI_SKU_WINDOWSMFC` (ReleaseMFC). Always match against `_inst`.
- The records list the original source files. TRenderInterface (Z:\Toshi\Source\TRender\): TAnimation, TCameraObject, TDBase,
  TDImp, TDebugText, TEnvironment, TGFXFileInstanceObject, TGFXFileObject, TIndexFactoryResourceInterface,
  TIndexPoolResourceInterface, TJPShadowFudgeNode, TLightingNode, TMaterialLibrary, TModel, TRenderAdapter, TRenderCapture,
  TRenderContext, TRenderInterface, TResource, TResourceNameMap, TScene, TSceneObject, TSceneSysMesh, TShader, TTerrainObject,
  TTextureFactory, TTextureResource, TTransformObject, TVertexFactoryResourceInterface, TVertexPoolResourceInterface, TViewport (.cpp).
  Our Src\<Dll> files should use exactly these names.

## #24 TRenderInterface rules (rounds 33-38)
- Implicit copies: TVector*/TQuaternion/TMatrix44 have no user copy operators. Two adjacent 16-bit fields that the original
  copies as one dword are a union with a 32-bit member.
- TObject: class-level dllimport (the inline copy ctor reads TObject's vtable through the import table) and a private,
  undefined operator= (no TObject-derived class exports one).
- TNodeTree/TNodeList node constructors store the links first and the owner pointer last. TNodeTree::Insert writes
  `node->m_pPrev->m_pNext = node` (it reuses the value just stored).
- TResource::Parent: `tree = m_pTree; parent = m_pParent; if (parent == 0 || parent == tree->GetRoot()) parent = 0; return parent;`
- TQList::InsertTail links through the node, like InsertHead: `p->m_pPrev = m_oHead.m_pPrev; p->m_pPrev->m_pNext = p;`
  (the original keeps the old tail in a register; matched PutProperty x2 and PutPropertyUnique, round 85).
- PProperties::PutPropertyUnique(name, value, comment) is a full copy of PutProperty's body, not a call to it - so the
  TPCString overload calls it instead of PutProperty (that was the one wrong-target match).
- ICC's layout of `if (!p) return x;` vs `p ? a : b` does not follow the spelling: swapping/rewording did not change
  GetRequiredProperty, operator==(const char*) or TPCString::GetCString (round 85). "Condition inverted" here is
  most likely the compiler build (see #23) - don't spend attempts on swapping if/else bodies.
- Experiment chains (`#if defined(VAR_A)`, TML_POS, TKF_FORM) were resolved to their active branch on 2026-10-28;
  resolve any new chain with Tools\bake.py in the same session. Still open: TQLIST_FORM - TAnimation compiles
  TQList.h with form 3, PPropertyParser with form 0; one real header has one form.
- Iterators (TNodeList / TQList): operator== / operator!= take the iterator BY VALUE - with `const Iterator&` ICC
  keeps the temporaries in memory (an 8-byte frame the original does not have). Some loops also cache
  `Iterator end = list.End();` before the loop (GetMaterial, PProperties::GetProperty); the others build End()
  per comparison. Fixed 4 functions in round 88 with no regressions in the 9 files that use these lists.
