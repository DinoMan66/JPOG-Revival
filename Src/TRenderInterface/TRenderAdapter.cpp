// TRenderInterface.dll - TRenderAdapter.cpp (Z:\Toshi\Source\TRender\TRenderAdapter.cpp in the original).
// Only GetModeList lives here; the other adapter members are defined in TRenderAdapter.h.
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TRenderAdapter.h"

namespace Toshi {

TNodeList<TRenderAdapter::Mode>* TRenderAdapter::GetModeList()          // 0x9b90
{
    return &m_oModes;
}

} // namespace Toshi
