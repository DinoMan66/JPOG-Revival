// TRenderInterface.dll - TVertexFactoryResourceInterface.cpp (0xe600-0xe7f0 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TVertexFactoryResourceInterface.h"
#include "TVertexPoolResourceInterface.h"

namespace Toshi {

TVertexFactoryResourceInterface::TVertexFactoryResourceInterface()      // 0xe670
    : m_uiNumPools(0), m_usMaxStaticVertices(0), m_uiFlags(0)
{
}

bool TVertexFactoryResourceInterface::Create(TVertexFactoryFormat* format, unsigned short maxStaticVertices,
                                             unsigned int flags)        // 0xe6b0
{
    m_oVertexFormat = *format;
    m_usMaxStaticVertices = maxStaticVertices;
    m_uiFlags = flags;
    return TResource::Create();
}

TOSHI_DEFINE_CLASS(TVertexFactoryResourceInterface, TResource)         // 0xe700

// pure virtual with a body: the derived factories call it to count their pools
TVertexPoolResourceInterface* TVertexFactoryResourceInterface::CreatePool(unsigned short maxVertices,
                                                                          unsigned int flags) // 0xe750
{
    m_uiNumPools++;
    return 0;
}

unsigned int TVertexFactoryResourceInterface::GetFlags()                // 0xe760
{
    return m_uiFlags;
}

unsigned short TVertexFactoryResourceInterface::GetMaxStaticVertices()  // 0xe770
{
    return m_usMaxStaticVertices;
}

TVertexFactoryFormat* TVertexFactoryResourceInterface::GetVertexFormat() // 0xe780
{
    return &m_oVertexFormat;
}

TVertexFactoryResourceInterface::~TVertexFactoryResourceInterface()     // 0xe790
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TVertexFactoryResourceInterface)           // 0xe7a0-0xe7e0

} // namespace Toshi
