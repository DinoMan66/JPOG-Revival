// TRenderInterface.dll - TSceneObject.cpp: TSceneObject, TNullSceneResource (0x10ef0-0x116a0)
// Not written yet: the file-static frustum helper at 0x112b0 (not exported).
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TSceneObject.h"
#include "TScene.h"

namespace Toshi {

bool TSceneObject::Update(TScene& scene)                                // 0x10ef0
{
    TResource* first = m_pChild;
    TResource* child = first;
    while (child)
    {
        TResource* next = child->m_pNext;
        if (next == first)
            next = 0;
        if (child->IsSceneObject() == true)
            ((TSceneObject*)child)->Update(scene);
        child = next;
    }
    return true;
}

bool TSceneObject::Render(TScene& scene)                                // 0x10f60
{
    if (m_uiFlags & FLAG_RENDER)
    {
        TResource* first = m_pChild;
        TResource* child = first;
        while (child)
        {
            TResource* next = child->m_pNext;
            if (next == first)
                next = 0;
            if (child->IsSceneObject() == true && (((TSceneObject*)child)->m_uiFlags & FLAG_RENDER))
                ((TSceneObject*)child)->Render(scene);
            child = next;
        }
    }
    return true;
}

bool TSceneObject::Recurse(TScene& scene)                               // 0x10ff0
{
    TResource* first = m_pChild;
    TResource* child = first;
    while (child)
    {
        TResource* next = child->m_pNext;
        if (next == first)
            next = 0;
        if (child->IsSceneObject() == true)
            ((TSceneObject*)child)->Recurse(scene);
        else
            scene.m_iNumNonSceneObjects++;
        child = next;
    }
    return true;
}

TOSHI_DEFINE_CLASS(TSceneObject, TResource)                            // 0x11060
TOSHI_DEFINE_CLASS(TNullSceneResource, TSceneObject)

TNullSceneResource::~TNullSceneResource()                               // 0x11100
{
}

TNullSceneResource::TNullSceneResource()                                // 0x11110
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TNullSceneResource)               // 0x11140-0x111b0

void TSceneObject::GetWorldMatrix(TMatrix44& matrix)                    // 0x111c0
{
    matrix.Identity();
}

void TSceneObject::GetLocalMatrix(TMatrix44& matrix)                    // 0x111d0
{
    matrix.Identity();
}

unsigned int TSceneObject::SetFlags(unsigned int flags)                 // 0x111e0
{
    return m_uiFlags = flags;
}

TSceneObject::~TSceneObject()                                           // 0x111f0
{
}

TSceneObject::TSceneObject()                                            // 0x11200
{
    m_uiFlags = FLAG_RENDER;
    MarkAsSceneObject();
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TSceneObject)                     // 0x11230-0x112a0

} // namespace Toshi
