// TRenderInterface.dll - TTextureFactory.cpp (0xde90-0xe490 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TTextureFactory.h"
#include "TTextureResource.h"
#include "TCString.h"
#include "TSystem.h"

namespace Toshi {

void TTextureFactory::Dump()                                            // 0xdf10
{
    TCString().Format("Texture Factory [%s] Dump:\n>\n", GetName()).Print();
    int count = 0;
    for (int i = 0; i < NUM_LISTS; i++)
    {
#if !defined(TTF_FT) || TTF_FT == 2
        for (NameEntry* entry = m_aLists[i].Begin(); entry != m_aLists[i].End(); entry = entry->Next())
        {
            TCString().Format("[%d] Texture [%s]: W=%d , H=%d\n", count++, entry->GetName(),
                              entry->GetTexture()->GetWidth(), entry->GetTexture()->GetHeight()).Print();
        }
#else
        for (TNodeList<NameEntry>::Iterator it = m_aLists[i].Begin(); it != m_aLists[i].End(); ++it)
        {
            TCString().Format("[%d] Texture [%s]: W=%d , H=%d\n", count++, it->GetName(),
                              it->GetTexture()->GetWidth(), it->GetTexture()->GetHeight()).Print();
        }
#endif
    }
    TCString().Format("There are %d registered textures\n", count).Print();
    TCString().Format("<\n").Print();
}

TTextureResource* TTextureFactory::FindTexture(const char* name)        // 0xe0a0
{
    TNodeList<NameEntry>& list = m_aLists[HashName(name)];
#if defined(TTF_FT) && TTF_FT == 1
    for (NameEntry* entry = list.Begin(); entry != list.End(); entry = entry ? entry->Next() : 0)
    {
        if (TSystem::StringCompareNoCase(entry->GetName(), name, -1) == 0)
            return entry->GetTexture();
    }
#elif defined(TTF_FT) && TTF_FT == 2
    for (NameEntry* entry = list.Begin(); entry != list.End(); entry = entry->Next())
    {
        if (TSystem::StringCompareNoCase(entry->GetName(), name, -1) == 0)
            return entry->GetTexture();
    }
#else
    for (TNodeList<NameEntry>::Iterator it = list.Begin(); it != list.End(); ++it)
    {
        if (TSystem::StringCompareNoCase(it->GetName(), name, -1) == 0)
            return it->GetTexture();
    }
#endif
    return 0;
}

void TTextureFactory::DeregisterTexture(TTextureResource* texture)      // 0xe110
{
    NameEntry* entry = texture->m_pNameEntry;
    if (entry)
    {
        entry->GetList()->Remove(entry);
        delete texture->m_pNameEntry;
    }
}

unsigned int TTextureFactory::HashName(const char* name)                // 0xe160
{
    int length = TSystem::StringLength(name);
    unsigned int hash = 0;
    for (int i = 0; i < length; i++)
        hash += name[i];
    return hash & (NUM_LISTS - 1);
}

TTextureFactory::NameEntry* TTextureFactory::RegisterTexture(const char* name, TTextureResource* texture) // 0xe1e0
{
    TNodeList<NameEntry>& list = m_aLists[HashName(name)];
    texture->m_pNameEntry = new NameEntry(name, texture);
    list.InsertHead(texture->m_pNameEntry);
    return texture->m_pNameEntry;
}

TOSHI_DEFINE_CLASS(TTextureFactory, TResource)                         // 0xe290

TTextureFactory::~TTextureFactory()                                     // 0xe2e0
{
}

TTextureFactory::TTextureFactory()                                      // 0xe360
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TTextureFactory)                           // 0xe3a0-0xe3e0

} // namespace Toshi
