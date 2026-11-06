// TRenderInterface.dll - TJPShadowFudgeNode.cpp (0x10160-0x10310 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TJPShadowFudgeNode.h"

namespace Toshi {

TOSHI_DEFINE_CLASS(TJPShadowFudgeNode, TSceneObject)                    // 0x101e0

bool TJPShadowFudgeNode::Render(TScene& scene)                          // 0x10240
{
    return TSceneObject::Render(scene);
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TJPShadowFudgeNode)               // 0x10260-0x102d0

} // namespace Toshi
