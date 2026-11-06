// Toshi::TSceneObject / TNullSceneResource (TRenderInterface.dll, TSceneObject.cpp,
// 0x10ef0-0x116a0). A scene object is a resource that walks its child ring for
// Update/Render/Recurse; children that are not scene objects are skipped.
#ifndef TSCENEOBJECT_H
#define TSCENEOBJECT_H
#include "TResource.h"
#include "TVector.h"

namespace Toshi {

class TScene;

class TRENDER_API TSceneObject : public TResource                       // 0x34
{
    TOSHI_DECLARE_CLASS(TSceneObject)
public:
    enum FLAGS { FLAG_RENDER = 2 };

    TSceneObject();
    virtual ~TSceneObject();

    virtual bool Update(TScene& scene);                                 // slot 10
    virtual bool Render(TScene& scene);                                 // slot 11
    virtual bool Recurse(TScene& scene);                                // slot 12
    virtual void GetLocalMatrix(TMatrix44& matrix);                     // slot 13

    void GetWorldMatrix(TMatrix44& matrix);
    unsigned int SetFlags(unsigned int flags);
    unsigned int GetFlags() { return m_uiFlags; }

protected:
    unsigned int m_uiFlags;     // +0x30

private:
    TSceneObject(const TSceneObject&);                                  // not exported
    TSceneObject& operator=(const TSceneObject&);
};

class TRENDER_API TNullSceneResource : public TSceneObject             // 0x34
{
    TOSHI_DECLARE_CLASS(TNullSceneResource)
public:
    TNullSceneResource();
    virtual ~TNullSceneResource();

private:
    TNullSceneResource(const TNullSceneResource&);
    TNullSceneResource& operator=(const TNullSceneResource&);
};

} // namespace Toshi
#endif
