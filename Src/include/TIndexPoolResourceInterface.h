// Toshi::TIndexPoolResourceInterface (TRenderInterface.dll, TIndexPoolResourceInterface.cpp,
// 0xccd0-0xcee0). Abstract: Lock/Unlock are implemented by the platform renderer.
#ifndef TINDEXPOOLRESOURCEINTERFACE_H
#define TINDEXPOOLRESOURCEINTERFACE_H
#include "TResource.h"

namespace Toshi {

class TIndexFactoryResourceInterface;

class TRENDER_API TIndexPoolResourceInterface : public TResource         // 0x3c
{
    TOSHI_DECLARE_CLASS(TIndexPoolResourceInterface)
public:
    class LockBuffer;

    TIndexPoolResourceInterface();
    virtual ~TIndexPoolResourceInterface();

    virtual bool Lock(LockBuffer* buffer) = 0;                          // slot 10
    virtual void Unlock(unsigned short newNumIndices) = 0;              // slot 11

    unsigned int GetFlags() const;
    unsigned int GetMaxIndices() const;
    unsigned int GetNumIndices() const;
    bool IsLocked() const;
    TIndexFactoryResourceInterface* GetFactory();

protected:
    virtual void OnDestroy();                                           // slot 9
    virtual bool Create(TIndexFactoryResourceInterface* factory, unsigned int maxIndices,
                        unsigned int flags);                            // slot 12

    TIndexFactoryResourceInterface* m_pFactory;     // +0x30
    unsigned short                  m_usFlags;      // +0x34
    unsigned short                  m_usMaxIndices; // +0x36
    unsigned short                  m_usNumIndices; // +0x38
    unsigned short                  m_usLockCount;  // +0x3a

private:
    TIndexPoolResourceInterface(const TIndexPoolResourceInterface&);    // not exported
    TIndexPoolResourceInterface& operator=(const TIndexPoolResourceInterface&);
};

} // namespace Toshi
#endif
