// TRenderInterface.dll - TTextureResource.cpp (0xe490-0xe5f0 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TTextureResource.h"
#include "TTextureFactory.h"
#include "TRenderInterface.h"

namespace Toshi {

TTextureResource::~TTextureResource()                                   // 0xe4a0
{
    ((TTextureFactory*)TRenderInterface::GetRenderer()->GetSystemResource((TRenderInterface::SYSRESOURCES)18))
        ->DeregisterTexture(this);
}

TOSHI_DEFINE_CLASS(TTextureResource, TResource)                        // 0xe4e0

bool TTextureResource::CreateEx(void* data, unsigned int dataSize, unsigned int width, unsigned int height,
                                unsigned int mipLevels, TTEXTURERESOURCEFORMAT format,
                                unsigned int flags)                     // 0xe530
{
    return TResource::Create();
}

bool TTextureResource::Create(void* data, unsigned int dataSize, unsigned int flags,
                              unsigned int width, unsigned int height)  // 0xe540
{
    return TResource::Create();
}

bool TTextureResource::Create(const char* fileName, unsigned int flags) // 0xe550
{
    return TResource::Create();
}

void TTextureResource::Unlock()                                         // 0xe560
{
}

bool TTextureResource::Lock(LOCKSTATE& lockState)                       // 0xe570
{
    return false;
}

TTextureResource::TTextureResource()                                    // 0xe580
{
    m_eAddressMode = (ADDRESSMODE)0;
    m_uiCreateFlags = 0;
    m_pNameEntry = 0;
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TTextureResource)                          // 0xe5a0-0xe5e0

} // namespace Toshi
