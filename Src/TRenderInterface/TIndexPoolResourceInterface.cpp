// TRenderInterface.dll - TIndexPoolResourceInterface.cpp (0xccd0-0xcee0 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TIndexPoolResourceInterface.h"
#include "TIndexFactoryResourceInterface.h"

namespace Toshi {

TIndexPoolResourceInterface::TIndexPoolResourceInterface()              // 0xcd50
    : m_pFactory(0), m_usFlags(0), m_usMaxIndices(0), m_usNumIndices(0), m_usLockCount(0)
{
}

TOSHI_DEFINE_CLASS(TIndexPoolResourceInterface, TResource)             // 0xcd90

void TIndexPoolResourceInterface::OnDestroy()                           // 0xcde0
{
    m_pFactory->m_uiNumPools--;
    TResource::OnDestroy();
}

bool TIndexPoolResourceInterface::Create(TIndexFactoryResourceInterface* factory, unsigned int maxIndices,
                                         unsigned int flags)            // 0xcdf0
{
    m_pFactory = factory;
    m_usMaxIndices = maxIndices;
    m_usFlags = flags;
    return TResource::Create();
}

unsigned int TIndexPoolResourceInterface::GetFlags() const              // 0xce20
{
    return m_usFlags;
}

unsigned int TIndexPoolResourceInterface::GetMaxIndices() const         // 0xce30
{
    return m_usMaxIndices;
}

unsigned int TIndexPoolResourceInterface::GetNumIndices() const         // 0xce40
{
    return m_usNumIndices;
}

bool TIndexPoolResourceInterface::IsLocked() const                      // 0xce50
{
    if (m_usLockCount > 0)
        return true;
    return false;
}

TIndexFactoryResourceInterface* TIndexPoolResourceInterface::GetFactory() // 0xce70
{
    return m_pFactory;
}

TIndexPoolResourceInterface::~TIndexPoolResourceInterface()             // 0xce80
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TIndexPoolResourceInterface)               // 0xce90-0xced0

} // namespace Toshi
