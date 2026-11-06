// Toshi::TFile - import declarations (the class lives in TKernelInterface.dll). Only the members
// the decompiled modules call are declared; signatures follow the kernel's export table.
#ifndef TFILE_H
#define TFILE_H
#include "TCString.h"

namespace Toshi {

class TFileSystem;

class TFile
{
public:
    enum FileMode
    {
        FileMode_Read     = 1,
        FileMode_Write    = 2,
        FileMode_ReadWrite = 4,
        FileMode_CreateNew = 8
    };

    // vtable order from TNativeFile's (TKernelInterface.dll ??_7TNativeFile)
    enum TSEEK { };
    virtual int Read(void* buffer, int size) = 0;                   // +0x00
    virtual int Write(const void* buffer, int size) = 0;            // +0x04
    virtual bool Seek(int offset, TSEEK origin) = 0;                // +0x08
    virtual int Tell() = 0;                                         // +0x0C
    virtual int GetSize() = 0;                                      // +0x10
    virtual unsigned __int64 GetDate() = 0;                         // +0x14
    virtual int GetCChar() = 0;                                     // +0x18
    virtual int GetWChar() = 0;                                     // +0x1C
    virtual int PutCChar(char c) = 0;                               // +0x20
    virtual int PutWChar(TWChar c) = 0;                             // +0x24
    virtual int VCPrintf(const char* format, char* args) = 0;       // +0x28
    virtual int VWPrintf(const TWChar* format, char* args) = 0;     // +0x2C

    TOSHI_API static TFile* __stdcall Create(const TCString& fileName, unsigned int mode);
    TOSHI_API static void __stdcall Destroy(TFile* file);
    TOSHI_API void Destroy();
    TOSHI_API int __cdecl CPrintf(const char* fmt, ...);
    TOSHI_API int __cdecl WPrintf(const TWChar* fmt, ...);
    TOSHI_API TFileSystem* GetFileSystem() const;
    TOSHI_API static TCString __stdcall ConcatPath(const TCString& a, const TCString& b);
    TOSHI_API static TCString __stdcall SimplifyPath(const TCString& path);
    TOSHI_API static void __stdcall SplitPath(const TCString& path, TCString& a, TCString& b);
    TOSHI_API static void __stdcall PrintFileAccess(bool enable);
};

} // namespace Toshi
#endif
