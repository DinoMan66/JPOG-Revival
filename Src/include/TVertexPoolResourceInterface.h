// Toshi::TVertexPoolResourceInterface (TRenderInterface.dll, TVertexPoolResourceInterface.cpp,
// 0xe7f0-0xea00). Abstract: Lock/Unlock are implemented by the platform renderer.
#ifndef TVERTEXPOOLRESOURCEINTERFACE_H
#define TVERTEXPOOLRESOURCEINTERFACE_H
#include "TResource.h"

namespace Toshi {

class TVertexFactoryResourceInterface;

class TRENDER_API TVertexPoolResourceInterface : public TResource         // 0x3c
{
    TOSHI_DECLARE_CLASS(TVertexPoolResourceInterface)
public:
    class LockBuffer;

    TVertexPoolResourceInterface();
    virtual ~TVertexPoolResourceInterface();

    virtual bool Lock(LockBuffer* buffer) = 0;                          // slot 10
    virtual void Unlock(unsigned short newNumVertices) = 0;              // slot 11

    unsigned int GetFlags() const;
    unsigned int GetMaxVertices() const;
    unsigned int GetNumVertices() const;
    bool IsLocked() const;
    TVertexFactoryResourceInterface* GetFactory();

protected:
    virtual void OnDestroy();                                           // slot 9
    virtual bool Create(TVertexFactoryResourceInterface* factory, unsigned int maxVertices,
                        unsigned int flags);                            // slot 12

    TVertexFactoryResourceInterface* m_pFactory;     // +0x30
    unsigned short                  m_usFlags;      // +0x34
    unsigned short                  m_usMaxVertices; // +0x36
    unsigned short                  m_usNumVertices; // +0x38
    unsigned short                  m_usLockCount;  // +0x3a

private:
    TVertexPoolResourceInterface(const TVertexPoolResourceInterface&);    // not exported
    TVertexPoolResourceInterface& operator=(const TVertexPoolResourceInterface&);
};

} // namespace Toshi
#endif
