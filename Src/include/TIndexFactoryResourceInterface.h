// Toshi::TIndexFactoryResourceInterface (TRenderInterface.dll, TIndexFactoryResourceInterface.cpp,
// 0xcb40-0xccd0). Abstract: the platform renderer derives the real factory (CreatePool is pure
// but has a body that the derived classes call).
#ifndef TINDEXFACTORYRESOURCEINTERFACE_H
#define TINDEXFACTORYRESOURCEINTERFACE_H
#include "TResource.h"

namespace Toshi {

class TIndexPoolResourceInterface;

class TRENDER_API TIndexFactoryResourceInterface : public TResource      // 0x3c
{
    TOSHI_DECLARE_CLASS(TIndexFactoryResourceInterface)
    friend class TIndexPoolResourceInterface;
public:
    TIndexFactoryResourceInterface();
    virtual ~TIndexFactoryResourceInterface();

    virtual bool Create(unsigned short maxStaticIndices, unsigned int flags);                   // slot 10
    virtual TIndexPoolResourceInterface* CreatePool(unsigned short maxIndices, unsigned int flags) = 0; // slot 11

    unsigned int GetFlags();
    unsigned short GetMaxStaticIndices();

protected:
    unsigned int   m_uiNumPools;            // +0x30
    unsigned short m_usMaxStaticIndices;    // +0x34
    unsigned int   m_uiFlags;               // +0x38

private:
    TIndexFactoryResourceInterface(const TIndexFactoryResourceInterface&);  // not exported
    TIndexFactoryResourceInterface& operator=(const TIndexFactoryResourceInterface&);
};

} // namespace Toshi
#endif
