// Toshi::TApplication - class layout reconstructed from the installed TApplication.dll
// (SHA1 61af511074b3b1a29e5c2c309c3943152fac9970, linked 2003-02-07).
// Original source: Z:\Toshi\Source\TApplication\TApplication.cpp
// Signatures come straight from the DLL's exported (decorated) names.
#ifndef TAPPLICATION_H
#define TAPPLICATION_H

#include "TCString.h"
#include "TKernelInterface.h"
#include "TRef.h"

// The real DLL exports every member of this class (the export table lists the constructors,
// operator=, the virtuals ... and the vtable). Defining TAPPLICATION_EXPORTS turns that on.
#ifdef TAPPLICATION_EXPORTS
#define TAPPLICATION_API __declspec(dllexport)
#else
#define TAPPLICATION_API
#endif

namespace Toshi {

class TAPPLICATION_API TApplication
{
public:
    TApplication();                                   // ??0TApplication@Toshi@@QAE@XZ
    TApplication(const TApplication& rhs);            // ??0...@ABV01@@Z
    virtual ~TApplication();                          // vtable slot 0

    TApplication& operator=(TApplication& rhs);       // ??4...@AAV01@@Z

    bool  Create(const char* name, int argc, char** const argv);
    bool  Execute();
    bool  ShowConsole(bool show);                     // returns previous state
    bool  ToggleConsole();                            // returns new state
    void  DisplayErrorMessageBox(TCString message);   // TCString passed BY VALUE
    bool  IsCreated();
    void  Destroy();
    void  SetVerbose(bool verbose);
    TKernelInterface* GetKernel();

    virtual bool OnCreate(int argc, char** const argv);   // slot 1
    virtual bool OnUpdate(float deltaTime);               // slot 2
    virtual bool OnDestroy();                             // slot 3

private:
    // +0x00 vtable pointer
    TRef<TKernelInterface> m_pKernel;   // +0x04 intrusive ref-counted pointer
    TCString          m_name;           // +0x08 (8 bytes)
    unsigned int      m_flags;          // +0x10 bit0 = created, bit1 = quit requested
    bool              m_verbose;        // +0x14
    bool              m_consoleVisible; // +0x15
    void**            m_pConsoleHwnd;   // +0x18 -> heap cell holding the console HWND
};                                      // sizeof == 0x1C

} // namespace Toshi

#endif
