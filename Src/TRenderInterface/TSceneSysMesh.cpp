// TRenderInterface.dll - TSceneSysMesh.cpp (0x190f0-0x19350 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TSceneSysMesh.h"
#include "TTransformObject.h"
#include "TShader.h"

namespace Toshi {

TSceneSysMesh::TSceneSysMesh()                                          // 0x19170
{
    m_pMesh = 0;
    SetFlags(GetFlags() & ~1);
}

bool TSceneSysMesh::Render(TScene& scene)                               // 0x191a0
{
    if (m_pMesh)
    {
        TResource* parent = Parent();
        if (parent && parent->IsA(TTransformObject::m_sClass))
            ((TTransformObject*)parent)->Push(scene);
        m_pMesh->Render();
        if (parent && parent->IsA(TTransformObject::m_sClass))
            ((TTransformObject*)parent)->Pop(scene);
    }
    return TSceneObject::Render(scene);
}

TOSHI_DEFINE_CLASS(TSceneSysMesh, TSceneObject)                         // 0x19220

bool TSceneSysMesh::SetMesh(TMesh* mesh)                                // 0x19280
{
    m_pMesh = mesh;
    return true;
}

bool TSceneSysMesh::Create(TMesh* mesh)                                 // 0x19290
{
    SetMesh(mesh);
    return TResource::Create();
}

TSceneSysMesh::~TSceneSysMesh()                                         // 0x192b0
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TSceneSysMesh)                    // 0x192c0-0x19330

} // namespace Toshi
