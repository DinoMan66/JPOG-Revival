// Toshi::TJPShadowFudgeNode (TRenderInterface.dll, TJPShadowFudgeNode.cpp, 0x10160-0x10310).
// A scene object with no state of its own; the default constructor and destructor are implicit.
#ifndef TJPSHADOWFUDGENODE_H
#define TJPSHADOWFUDGENODE_H
#include "TSceneObject.h"

namespace Toshi {

class TRENDER_API TJPShadowFudgeNode : public TSceneObject             // 0x34
{
    TOSHI_DECLARE_CLASS(TJPShadowFudgeNode)
public:
    virtual bool Render(TScene& scene);                                 // slot 11
};

} // namespace Toshi
#endif
