// Toshi::TRenderCapture (TRenderInterface.dll, TRenderCapture.cpp, 0x9bf0-0x9c10): abstract
// screen capture; implemented by the platform renderer.
#ifndef TRENDERCAPTURE_H
#define TRENDERCAPTURE_H
#include "TRenderAPI.h"

namespace Toshi {

class TRENDER_API TRenderCapture
{
public:
    TRenderCapture();
    virtual ~TRenderCapture();                                          // slot 0

    virtual bool Create() = 0;                                          // slots 1-7 (names unknown)
    virtual void Destroy() = 0;
    virtual void* Lock() = 0;
    virtual void Unlock() = 0;
    virtual unsigned int GetWidth() = 0;
    virtual unsigned int GetHeight() = 0;
    virtual unsigned int GetFormat() = 0;

private:
    TRenderCapture(const TRenderCapture&);                              // not exported
    TRenderCapture& operator=(const TRenderCapture&);
};

} // namespace Toshi
#endif
