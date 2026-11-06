// TRenderInterface.dll - TIndexFactoryResourceInterface.cpp (0xcb40-0xccd0 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TIndexFactoryResourceInterface.h"
#include "TIndexPoolResourceInterface.h"

namespace Toshi {

TOSHI_DEFINE_CLASS(TIndexFactoryResourceInterface, TResource)          // 0xcbb0

// pure virtual with a body: the derived factories call it to count their pools
TIndexPoolResourceInterface* TIndexFactoryResourceInterface::CreatePool(unsigned short maxIndices,
                                                                        unsigned int flags) // 0xcc00
{
    m_uiNumPools++;
    return 0;
}

bool TIndexFactoryResourceInterface::Create(unsigned short maxStaticIndices, unsigned int flags) // 0xcc10
{
    m_usMaxStaticIndices = maxStaticIndices;
    m_uiFlags = flags;
    return TResource::Create();
}

unsigned int TIndexFactoryResourceInterface::GetFlags()                 // 0xcc30
{
    return m_uiFlags;
}

unsigned short TIndexFactoryResourceInterface::GetMaxStaticIndices()    // 0xcc40
{
    return m_usMaxStaticIndices;
}

TIndexFactoryResourceInterface::~TIndexFactoryResourceInterface()       // 0xcc50
{
}

TIndexFactoryResourceInterface::TIndexFactoryResourceInterface()        // 0xcc60
    : m_uiNumPools(0), m_usMaxStaticIndices(0), m_uiFlags(0)
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TIndexFactoryResourceInterface)            // 0xcc80-0xccc0

} // namespace Toshi
