// TRenderInterface.dll - TGFXFileObject.cpp (0xff00-0x10160 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TGFXFileObject.h"
#include "TGFXFileInstanceObject.h"
#include "TModel.h"
#include "TRenderInterface.h"

namespace Toshi {

bool TGFXFileObject::Create(char* name)                                 // 0xff80
{
    m_pModel = TRenderInterface::GetRenderer()->CreateModel(name, true);
    if (m_pModel)
        return TResource::Create();
    return false;
}

TGFXFileInstanceObject* TGFXFileObject::CreateInstance(const char* name, TResource* parent) // 0xffb0
{
    if (!parent)
        parent = this;
    TGFXFileInstanceObject* instance = (TGFXFileInstanceObject*)GetRenderer()->CreateResource(
        &TGFXFileInstanceObject::m_sClass, name, parent);
    instance->Create(this);
    return instance;
}

TOSHI_DEFINE_CLASS(TGFXFileObject, TSceneObject)                        // 0xfff0

bool TGFXFileObject::Render(TScene& scene)                              // 0x10050
{
    return TSceneObject::Render(scene);
}

bool TGFXFileObject::Update(TScene& scene)                              // 0x10070
{
    return TSceneObject::Update(scene);
}

void TGFXFileObject::OnDestroy()                                        // 0x10090
{
    if (m_pModel)
    {
        m_pModel->Delete();
        m_pModel = 0;
        TResource::OnDestroy();
    }
}

TGFXFileObject::~TGFXFileObject()                                       // 0x100b0
{
}

TGFXFileObject::TGFXFileObject()                                        // 0x100c0
{
    m_pModel = 0;
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TGFXFileObject)                   // 0x100e0-0x10150

} // namespace Toshi
