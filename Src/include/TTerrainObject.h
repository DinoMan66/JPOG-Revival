// Toshi::TTerrainObject (TRenderInterface.dll, TTerrainObject.cpp, 0x112b0-0x118f0).
// A scene object that holds a TTerrainInterface* and builds the camera frustum when rendered.
#ifndef TTERRAINOBJECT_H
#define TTERRAINOBJECT_H
#include "TSceneObject.h"

namespace Toshi {

class TTerrainInterface;

class TRENDER_API TTerrainObject : public TSceneObject                 // 0x38
{
    TOSHI_DECLARE_CLASS(TTerrainObject)
public:
    TTerrainObject();
    virtual ~TTerrainObject();

    virtual bool Render(TScene& scene);                                 // slot 11
    virtual bool Create(TTerrainInterface* terrain);                    // slot 14

    bool SetTerrain(TTerrainInterface* terrain);
    TTerrainInterface* GetTerrain() { return m_pTerrain; }

protected:
    TTerrainInterface* m_pTerrain;  // +0x34

private:
    TTerrainObject(const TTerrainObject&);                              // not exported
    TTerrainObject& operator=(const TTerrainObject&);
};

} // namespace Toshi
#endif
