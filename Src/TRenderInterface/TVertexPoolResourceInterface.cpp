// TRenderInterface.dll - TVertexPoolResourceInterface.cpp (0xe7f0-0xea00 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TVertexPoolResourceInterface.h"
#include "TVertexFactoryResourceInterface.h"

namespace Toshi {

TVertexPoolResourceInterface::TVertexPoolResourceInterface()              // 0xe870
    : m_pFactory(0), m_usFlags(0), m_usMaxVertices(0), m_usNumVertices(0), m_usLockCount(0)
{
}

TOSHI_DEFINE_CLASS(TVertexPoolResourceInterface, TResource)             // 0xe8b0

void TVertexPoolResourceInterface::OnDestroy()                           // 0xe900
{
    m_pFactory->m_uiNumPools--;
    TResource::OnDestroy();
}

bool TVertexPoolResourceInterface::Create(TVertexFactoryResourceInterface* factory, unsigned int maxVertices,
                                         unsigned int flags)            // 0xe910
{
    m_pFactory = factory;
    m_usMaxVertices = maxVertices;
    m_usFlags = flags;
    return TResource::Create();
}

unsigned int TVertexPoolResourceInterface::GetFlags() const              // 0xe940
{
    return m_usFlags;
}

unsigned int TVertexPoolResourceInterface::GetMaxVertices() const         // 0xe950
{
    return m_usMaxVertices;
}

unsigned int TVertexPoolResourceInterface::GetNumVertices() const         // 0xe960
{
    return m_usNumVertices;
}

bool TVertexPoolResourceInterface::IsLocked() const                      // 0xe970
{
    if (m_usLockCount > 0)
        return true;
    return false;
}

TVertexFactoryResourceInterface* TVertexPoolResourceInterface::GetFactory() // 0xe990
{
    return m_pFactory;
}

TVertexPoolResourceInterface::~TVertexPoolResourceInterface()             // 0xe9a0
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TVertexPoolResourceInterface)               // 0xe9b0-0xe9f0

} // namespace Toshi
