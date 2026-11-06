// Toshi::TTextureResource (TRenderInterface.dll, TTextureResource.cpp, 0xe490-0xe5f0). Abstract
// texture; TTextureResourceHAL is the platform implementation. The inline members are emitted
// with TRenderInterface.cpp (0xc650-0xc6b9).
#ifndef TTEXTURERESOURCE_H
#define TTEXTURERESOURCE_H
#include "TResource.h"
#include "TTextureFactory.h"

namespace Toshi {

class TRENDER_API TTextureResource : public TResource                   // 0x3c
{
    TOSHI_DECLARE_CLASS(TTextureResource)
    friend class TTextureFactory;
public:
    enum ADDRESSMODE { };
    struct LOCKSTATE;

    TTextureResource();
    virtual ~TTextureResource();

    virtual unsigned int GetWidth() = 0;                                // slot 10
    virtual unsigned int GetHeight() = 0;                               // slot 11
    virtual bool Lock(LOCKSTATE& lockState);                            // slot 12
    virtual void Unlock();                                              // slot 13
protected:
    virtual bool Create(void* data, unsigned int dataSize, unsigned int flags,
                        unsigned int width, unsigned int height) = 0;   // slot 14 (pure, with a body)
    virtual bool Create(const char* fileName, unsigned int flags) = 0;  // slot 15 (pure, with a body)
    virtual bool CreateEx(void* data, unsigned int dataSize, unsigned int width, unsigned int height,
                          unsigned int mipLevels, TTEXTURERESOURCEFORMAT format,
                          unsigned int flags);                          // slot 16
public:
    ADDRESSMODE GetAddressMode() { return m_eAddressMode; }
    ADDRESSMODE SetAddressModeMode(ADDRESSMODE mode) { return m_eAddressMode = mode; }
    unsigned int GetCreateFlags() { return m_uiCreateFlags; }
    TTextureFactory::NameEntry* GetNameEntry() { return m_pNameEntry; }

    static TTextureResource* __stdcall CreateHAL(TRenderInterface* renderer, const char* name, TResource* parent)
    {
        return (TTextureResource*)renderer->CreateResource(TClass::Find("TTextureResourceHAL", 0), name, parent);
    }

protected:
    ADDRESSMODE                 m_eAddressMode;     // +0x30
    unsigned int                m_uiCreateFlags;    // +0x34
    TTextureFactory::NameEntry* m_pNameEntry;       // +0x38

private:
    TTextureResource(const TTextureResource&);
    TTextureResource& operator=(const TTextureResource&);
};

} // namespace Toshi
#endif
