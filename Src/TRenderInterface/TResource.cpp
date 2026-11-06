// TRenderInterface.dll - TResource / TNullResource (0x12bd0-0x13030 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TResource.h"
#include "TRenderInterface.h"
#include "TSystem.h"

namespace Toshi {

TResource::TResource()                                                  // 0x12bd0
{
    m_iFlags = 0;
    m_uiUId = 0;
    m_pRenderer = 0;
    m_szName[0] = 0;
}

void TResource::SetName(const char* name)                               // 0x12c10
{
#ifdef TRES_SETNAME_B
    char buf[16];
    if (name == 0)
    {
        name = buf;
        buf[0] = 'r';
        buf[1] = 'e';
        buf[2] = 's';
        buf[3] = ':';
        TSystem::StringIntToString(GetUId(), &buf[4], 10);
    }
#else
    if (name == 0)
    {
        char buf[16];
        buf[0] = 'r';
        buf[1] = 'e';
        buf[2] = 's';
        buf[3] = ':';
        TSystem::StringIntToString(GetUId(), buf + 4, 10);
        name = buf;
    }
#endif
    TSystem::StringCopy((char*)m_szName, name, -1);
}

TOSHI_DEFINE_CLASS(TResource, TObject)
TOSHI_DEFINE_CLASS(TNullResource, TResource)

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TNullResource)                    // 0x12d10-0x12d80

void TResource::OnDestroy()                                             // 0x12d90
{
    m_iFlags &= ~FLAG_CREATED;
    m_iFlags |= FLAG_DYING;
}

TResource::~TResource()                                                 // 0x12da0
{
    m_iFlags |= FLAG_DEAD;
}

bool TResource::OnResetDevice()                                         // 0x12dc0
{
    return true;
}

bool TResource::OnLostDevice()                                          // 0x12dd0
{
    return true;
}

void TResource::RecurseSimple(TNodeTree<TResource>::t_RecurseSimpleCallback callback, TResource* start, void* userData) // 0x12de0
{
    TNodeTree<TResource>* tree = m_pTree;
    if (start)
        tree->RecurseSimple(callback, start, false, userData);
    else
        tree->RecurseSimple(callback, tree->m_oRoot.m_pChild, true, userData);
}

void TResource::SetParent(TResource* parent)                            // 0x12e20
{
    TNodeTree<TResource>* tree = m_pTree;
    tree->Remove(*this, false);
    if (parent)
        tree->Insert(parent, this);
    else
        tree->InsertAtRoot(this);
}

unsigned int TResource::GetUId() const                                  // 0x12ec0
{
    return m_uiUId;
}

const char* TResource::GetName() const                                  // 0x12ed0
{
    return (const char*)m_szName;
}

TRenderInterface* TResource::GetRenderer() const                        // 0x12ee0
{
    return m_pRenderer;
}

void TResource::MarkAsSceneObject()                                     // 0x12ef0
{
    m_iFlags |= FLAG_SCENEOBJECT;
}

bool TResource::IsDying()                                               // 0x12f00
{
    return (m_iFlags & FLAG_DYING) != 0;
}

bool TResource::IsCreated()                                             // 0x12f10
{
    return (m_iFlags & FLAG_CREATED) != 0;
}

bool TResource::IsValid()                                               // 0x12f20
{
    if (!IsCreated())
        return false;
    return (m_iFlags & FLAG_VALID) != 0;
}

void TResource::DestroyResource()                                       // 0x12f50
{
    GetRenderer()->DestroyResource(this);
}

void TResource::Invalidate()                                            // 0x12f70
{
    m_iFlags &= ~FLAG_VALID;
}

bool TResource::Validate()                                              // 0x12f80
{
    if (m_iFlags & FLAG_DYING)
        return false;
    m_iFlags |= FLAG_VALID;
    return true;
}

bool TResource::Create()                                                // 0x12fa0
{
    m_iFlags |= FLAG_CREATED;
    return true;
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TResource)                        // 0x12fb0-0x13020

} // namespace Toshi
