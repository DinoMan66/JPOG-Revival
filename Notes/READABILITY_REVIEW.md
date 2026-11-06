# Readability review - JPOG (2026-10-27)

Every file in Src\ was checked against Notes\READABILITY_STANDARD.md (calibrated version, 2026-10-27:
honest placeholders are allowed, fabricated or generated code is not; automated rules + a manual pass).
Notes for each file that is (or may be) removed are in Notes\removed\ - one section per function with its
address, placeholder name, status and the exact source that matched, plus the full original file.

| Group | Files | Functions written in them | Compiler-generated copies they back | Decision |
|---|---|---|---|---|
| Readable - keep | 106 | 596 | 0 | keep |
| 2 - Decompiled, not fully documented (honest placeholders: m_unkXX / UIH_<address> / slot names) | 6 | 52 | 0 | keep (documentation to-do below) |
| 3 - Scratch / obsolete files (never part of the program) | 5 | 0 | 0 | **delete** |

## Group 2 - decompiled, not fully documented (keep)

| File | Functions | Copies backed | Why | Notes |
|---|---|---|---|---|
| `Src/TInputInterface/TInputDevice.cpp` | 14 |  |  |  |
| `Src/TRenderInterface/TGFXFileInstanceObject.cpp` | 15 |  |  |  |
| `Src/TRenderInterface/TMaterialLibrary.cpp` | 23 |  |  |  |
| `Src/include/TFileLexer.h` | 0 |  |  |  |
| `Src/include/TInputInterface.h` | 0 |  |  |  |
| `Src/include/TKernelInterface.h` | 0 |  |  |  |

These stay. Their unknowns are honest placeholders of the kind sm64/oot/mm/tp keep (offset-named members,
address-named functions, unknown vtable slots). Renaming members, locals and non-exported names never changes
the generated code, so documenting them later keeps every match (exported names are fixed by the binary). Give a name only when the purpose is known.

Documentation to-do per file:
- `include/TInputInterface.h` -> 10 cpp files (TInputInterface, TInputDXInterface, all their devices): three members. `m_unk04` can be renamed now - it is the intrusive reference count of the second base (MATCHING_NOTES #19, TRefCountBase). `m_unk10` and `m_unk34` (TInputDevice) are never read in this DLL; their purpose must be found in the callers (other DLLs / SimJP.exe) before they can be named.
- `include/TFileLexer.h` -> PPropertyReader/PPropertyWriter (.h + .cpp): `m_pad04[4]`, `m_pad09[0x77]` - the lexer's state bytes; name them from TKernelInterface's TFileLexer functions.
- `include/TKernelInterface.h` -> TApplication (.h + .cpp): one slot `Slot0` (vtable slot 0, purpose unknown).
- `TRenderInterface/TMaterialLibrary.cpp`, `TGFXFileInstanceObject.cpp`: `_Slot12`..`_SlotNN` placeholders for shader/material interfaces implemented in the D3D DLLs. The names can be recovered mechanically: the D3D DLLs export their member functions by name, so each vtable slot maps to an exported name.
- `TInputInterface/TInputDevice.cpp`: uses `m_unk34` (see the header).

## Group 3 - scratch / obsolete (delete)

- `Src/TInputDevice.cpp`
- `Src/TInputDeviceController.cpp`
- `Src/TInputDeviceKeyboard.cpp`
- `Src/TInputDeviceMouse.cpp`
- `Src/TInputInterface.cpp`

## Files to delete

Groups 1, 3 and 4, plus the instance lists that are verified against them:

```
C:\OpenSourceProjects\JPOGDecompiled\Src\TInputDevice.cpp
C:\OpenSourceProjects\JPOGDecompiled\Src\TInputDeviceController.cpp
C:\OpenSourceProjects\JPOGDecompiled\Src\TInputDeviceKeyboard.cpp
C:\OpenSourceProjects\JPOGDecompiled\Src\TInputDeviceMouse.cpp
C:\OpenSourceProjects\JPOGDecompiled\Src\TInputInterface.cpp
```

Notes written by the earlier (strict) review for files that are now kept - no longer needed:

```
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\PPropertyParser__PPropertyReader.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\PPropertyParser__PPropertyWriter.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TApplication__TApplication.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputDXInterface__TInputDXDeviceController.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputDXInterface__TInputDXDeviceKeyboard.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputDXInterface__TInputDXDeviceMouse.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputDXInterface__TInputDXInterface.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputInterface__TInputDevice.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputInterface__TInputDeviceController.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputInterface__TInputDeviceKeyboard.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputInterface__TInputDeviceMouse.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TInputInterface__TInputInterface.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TRenderInterface__TGFXFileInstanceObject.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\TRenderInterface__TMaterialLibrary.cpp.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\include__TFileLexer.h.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\include__TInputInterface.h.md
C:\OpenSourceProjects\JPOGDecompiled\Notes\removed\include__TKernelInterface.h.md
```

