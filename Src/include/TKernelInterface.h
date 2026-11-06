// Toshi::TKernelInterface / THPTimer - stubs (the real classes live in TKernelInterface.dll).
// Only what TApplication.dll uses. Decorated import names:
//   ??0TKernelInterface@Toshi@@QAE@HQAPAD_N@Z   TKernelInterface(int argc, char** const argv, bool verbose)
//   ?Update@TKernelInterface@Toshi@@QAE_NXZ     bool Update()
//   ?GetSystemTimer@TKernelInterface@Toshi@@QAEPAVTHPTimer@2@XZ   THPTimer* GetSystemTimer()
//   ?GetDelta@THPTimer@Toshi@@QAEMXZ            float GetDelta()
// Layout facts (from TApplication's smart-pointer code):
//   +0x00 vtable, +0x04 reference count, total size 0x58 (tmalloc(0x58,...))
//   vtable slot 1 is called with no arguments when the count reaches zero.
#ifndef TKERNELINTERFACE_H
#define TKERNELINTERFACE_H
#include "TMemory.h"

namespace Toshi {

class THPTimer
{
public:
    TOSHI_API float GetDelta();
};

class TRefCounted
{
public:
    virtual void Slot0();           // vtable slot 0 (purpose unknown)
    virtual void Delete();          // vtable slot 1 - called when m_refCount hits 0
    int m_refCount;                 // +0x04
};

class TKernelInterface : public TRefCounted
{
public:
    TOSHI_API TKernelInterface(int argc, char** const argv, bool verbose);
    TOSHI_API bool Update();
    TOSHI_API THPTimer* GetSystemTimer();

private:
    unsigned char m_rest[0x58 - 8]; // remaining members unknown; total size must be 0x58
};

} // namespace Toshi
#endif
