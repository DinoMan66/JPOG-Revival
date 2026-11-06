// Toshi::TSceneSysMesh (TRenderInterface.dll, TSceneSysMesh.cpp, 0x190f0-0x19350).
// A scene object that renders one TMesh, inside its parent's transform when the parent is a TTransformObject.
#ifndef TSCENESYSMESH_H
#define TSCENESYSMESH_H
#include "TSceneObject.h"

namespace Toshi {

class TMesh;

class TRENDER_API TSceneSysMesh : public TSceneObject                  // 0x38
{
    TOSHI_DECLARE_CLASS(TSceneSysMesh)
public:
    TSceneSysMesh();
    virtual ~TSceneSysMesh();

    virtual bool Render(TScene& scene);                                 // slot 11
    virtual bool Create(TMesh* mesh);                                   // slot 14

    bool SetMesh(TMesh* mesh);
    TMesh* GetMesh() { return m_pMesh; }

protected:
    TMesh* m_pMesh;     // +0x34

private:
    TSceneSysMesh(const TSceneSysMesh&);                                // not exported
    TSceneSysMesh& operator=(const TSceneSysMesh&);
};

} // namespace Toshi
#endif
