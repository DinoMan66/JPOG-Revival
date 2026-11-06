# Functions skipped as compiler-limited (build 010525Z vs the original 020125Z)

These functions are written. They match the original's logic, but our ICC 5.0.1 build 010525Z
generates different code for reasons no source change has fixed. Re-test them when another
compiler is available (ICC 6.0 comparison builds, or build 020125Z if it turns up).

The patterns (see MATCHING_NOTES.md #23) are:
- **inline** — the original keeps small inline functions as calls; ours inlines them (no -Ob/-Qip option changes this)
- **shrink-wrap** — the original saves callee-saved registers in the prologue; ours saves them inside the branch that needs them
- **sched** — instruction order or register choice differs, with the same operations
- **dead-store** — the original keeps dead temporaries or stores
- **fp-const** — float constants are used as DWORD vs QWORD operands, 2*x is fmul vs fadd, or float-to-int conversion differs

## TRenderInterface.dll

| Function | Pattern | Notes |
|---|---|---|
| TAnimation::GetSequencePtr | sched | |
| ~TKeyframeLibraryInstance | sched | |
| TResource::IsValid | inline | IsCreated is kept as a call |
| TResource / TNullResource / TShader copy ctor | sched | movzx vs mov in the char[] copy |
| TResource::SetName | sched | |
| TAtmosphere::SetAtmHGg, InitVSConstants, CalculateScatteringConstants | fp-const | |
| TSun::ComputeAttenuation | fp-const | constants and slot order |
| TSun::SetTheta / Set | sched | static-guard block layout |
| TSun::SetBiasColour | fp-const | |
| TRenderAdapter::~TRenderAdapter | shrink-wrap | |
| TRenderContext::CullSphereToFrustum | sched | 1 byte |
| TRenderContext::GetWorldPlanes / GetWorldGuardPlanes | sched | induction-variable choice |
| TRenderContext ctor | sched / dead-store | |
| TRenderInterface ctor | sched / dead-store | vtable store position; the original keeps a dead TVector2 temp (the memset now matches) |
| TRenderInterface dtor | shrink-wrap | |
| TRenderInterface::CreateResource / CreateResourceInPlace | sched | the parent parameter is reloaded from the stack |
| TRenderInterface::FlushShadersRecurse | shrink-wrap | |
| TRenderInterface::MakeRGBA(float x4) | fp-const | the original truncates double to uint inline (shld/shr); ours uses fistp + fldcw |
| TRenderInterface::FindDevice, DumpStats | (re-testing) | GetModeList turned out to be TRenderAdapter.cpp's only function (non-inline): fixed in round 67 |
| TNodeTree<TResource>::Recurse (0xc840) | inline | the original calls the out-of-line TResource::Next/Child (0x7e30/0x7e40) |
| TRenderInterface::DestroyDyingResources, DestroyResourceTypeRecurse | padding | 3 bytes: a different encoding of the nop padding inside the loop |
| TMaterialLibrary::Create, ReadHeader, LoadMaterial, LoadTerrainMaterial | sched | the original duplicates the small `return false` epilogues; ours merges them |
| TMaterialLibrary::LoadMaterials, LoadTexture, LoadSkin/System/SpriteMaterial | sched | register allocation: the original keeps the flags local in a stack slot |
| TMaterialLibraryManager::LoadMaterialLibrary, TModel::CreateInstance | inline | the original calls the protected constructors out of line; ours inlines them |
| ~TMaterialLibraryManager | sched | null-checked list stepping / loop layout |
| TMaterial / TMesh / TShader copy ctors | sched | movzx vs mov in the inherited TResource name copy |
| TDBase::TKeyFrame::IsInTimeRange, IsInExclusiveTimeRange | sched | the original pops the x87 stack on each early exit and keeps two `return false` epilogues; ours merges them. 5 source forms tried (round 83) |
| TDBase::TKeySequence::DeleteData, TSequence::DeleteData | sched | the original saves registers with pushes and keeps RemoveHead's empty-list `return 0` inline; ours uses stack spills and an out-of-line block |
| TKeyframeLibraryManager::GetLibrary, DebugDump | dead-store | ours stores the End() iterator to two stack slots and rotates the loop; same family as FindTexture |
| TSkeleton::GetSequence(name), GetBone(name) | inline | the original calls GetSequence(int)/GetBone(int) out of line (same file) |

## Auto-filed by verdict.py

Filed after 2 real attempts with a difference of a compiler-limited kind (see Tools/triage_lib.py). Re-test when another compiler build is available.

| Module | Function | Address | Kind | First difference (ours / orig) | Filed |
|---|---|---|---|---|---|
| TRenderInterface.dll | TSkeletonInstance::RemoveAnimation | 0x47c0 | regalloc | `push   ebx` / `push   ebp` | 2026-10-28 |
| TRenderInterface.dll | TAnimation::SetTime | 0x5360 | regalloc | `push   esi` / `push   edi` | 2026-10-28 |
| TRenderInterface.dll | TAnimation::UpdateTime | 0x54c0 | regalloc | `push   edi` / `push   esi` | 2026-10-28 |
| TRenderInterface.dll | TDImp::ComposeHierarchy | 0x156f0 | shrink-wrap | `push   esi` / `sub    esp,0xc` | 2026-10-28 |
| TRenderInterface.dll | TDImp::ReorderAllKeySeqs | 0x15730 | shrink-wrap | `push   edi` / `sub    esp,0x6c` | 2026-10-28 |
| TRenderInterface.dll | TDImp::MarkMeshSet | 0x16b70 | regalloc | `mov    esi,ecx` / `mov    edi,ecx` | 2026-10-28 |
| TRenderInterface.dll | TBone::ReorderKeySeqs | 0x17bb0 | shrink-wrap | `push   edi` / `sub    esp,0x24` | 2026-10-28 |

**Resolved 2026-10-28 (rounds 85-88):** TMaterialLibraryManager::GetMaterial and TTextureFactory::FindTexture were
not the compiler: the iterators' comparison operators take the iterator BY VALUE (TNodeList.h / TQList.h), and
GetMaterial / PProperties::GetProperty cache `End()` in a local. Also fixed: TKeySequence::GenerateExportCloseKey,
PProperties::PutProperty x2 and PutPropertyUnique (TQList::InsertTail through the node). The 7 functions filed
above on 2026-10-28 were counted as a second attempt because the header change altered their code.
