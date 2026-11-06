// Toshi::TGFXFileObject (TRenderInterface.dll, TGFXFileObject.cpp, 0xff00-0x10160).
// A scene object owning a TModel; instances of it are TGFXFileInstanceObjects.
#ifndef TGFXFILEOBJECT_H
#define TGFXFILEOBJECT_H
#include "TSceneObject.h"

namespace Toshi {

class TModel;
class TGFXFileInstanceObject;

class TRENDER_API TGFXFileObject : public TSceneObject                 // 0x38
{
    TOSHI_DECLARE_CLASS(TGFXFileObject)
public:
    TGFXFileObject();
    virtual ~TGFXFileObject();

    virtual void OnDestroy();                                           // slot 9
    virtual bool Update(TScene& scene);                                 // slot 10
    virtual bool Render(TScene& scene);                                 // slot 11
    virtual bool Create(char* name);                                    // slot 14

    TGFXFileInstanceObject* CreateInstance(const char* name, TResource* parent);
    TModel* GetModel() { return m_pModel; }

protected:
    TModel* m_pModel;   // +0x34

private:
    TGFXFileObject(const TGFXFileObject&);                              // not exported
    TGFXFileObject& operator=(const TGFXFileObject&);
};

} // namespace Toshi
#endif
