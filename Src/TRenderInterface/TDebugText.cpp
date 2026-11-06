// TRenderInterface.dll - TDebugText.cpp (0x9550-0x95f0 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TDebugText.h"
#include "TSystem.h"

namespace Toshi {

TDebugText::String* TDebugText::PrintString(unsigned short x, unsigned short y, const char* text,
                                            unsigned short flags)       // 0x9550
{
    int length = TSystem::StringLength(text);
    String* string = AllocateString((unsigned short)(length + 1));
    if (!string)
        return 0;
    string->m_usFlags = flags;
    string->m_usX = x;
    string->m_usY = y;
    string->m_usLength = (unsigned short)length;
    TSystem::StringCopy(string->m_szText, text, -1);
    return string;
}

TDebugText::String* TDebugText::AddString(const char* text, unsigned short flags) // 0x95c0
{
    return PrintString(0, 0, text, flags | 1);
}

void TDebugText::Destroy()                                              // 0x95e0
{
    tfree(this);
}

} // namespace Toshi
