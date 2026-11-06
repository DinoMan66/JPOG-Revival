// Toshi::TDebugText (TRenderInterface.dll, TDebugText.cpp, 0x9550-0x95f0): abstract buffered
// on-screen debug text; the platform renderer implements the buffer. The constructor,
// destructor and implicit copy members are inline (0x7340-0x73a8).
#ifndef TDEBUGTEXT_H
#define TDEBUGTEXT_H
#include "TRenderAPI.h"

namespace Toshi {

class TRenderInterface;

class TRENDER_API TDebugText                                            // 0x10
{
public:
    struct String                                                       // followed by the text
    {
        unsigned short m_usX;           // +0
        unsigned short m_usY;           // +2
        unsigned short m_usLength;      // +4
        unsigned short m_usFlags;       // +6
        char           m_szText[1];     // +8
    };

    virtual void Reset() = 0;                                           // slot 0
    virtual void Destroy() = 0;                                         // slot 1 (pure, with a body)
    virtual void Render() = 0;                                          // slot 2
    virtual String* AllocateString(unsigned short length) = 0;          // slot 3

    String* PrintString(unsigned short x, unsigned short y, const char* text, unsigned short flags);
    String* AddString(const char* text, unsigned short flags);

protected:
    TDebugText() {}
    virtual ~TDebugText() {}                                            // slot 4

    TRenderInterface* m_pRenderer;      // +0x04
    int               m_iBufferSize;    // +0x08
    unsigned char*    m_pCurrent;       // +0x0c
};

} // namespace Toshi
#endif
