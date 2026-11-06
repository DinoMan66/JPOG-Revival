// TRenderInterface.dll - TResourceNameMap.cpp (0x13030-0x133c0 in the original)
#define TRENDERINTERFACE_EXPORTS
#define SETCAP 6
#include "TNewDelete.h"
#include "TResourceNameMap.h"
#include "TSystem.h"

namespace Toshi {

TResourceNameMap::Item* TResourceNameMap::Find(const char* name)       // 0x13030
{
    for (int i = 0; i < m_aItems.m_count; i++)
    {
        if (TSystem::StringCompareNoCase(m_aItems[i].GetName(), name, -1) == 0)
            return &m_aItems[i];
    }
    return 0;
}

bool TResourceNameMap::Item::IsValid()                                 // 0x130a0
{
    if (m_pResource)
        return true;
    return false;
}

void TResourceNameMap::Item::SetResource(TResource* resource)          // 0x130b0
{
    m_pResource = resource;
}

void TResourceNameMap::Item::SetName(const char* name)                 // 0x130c0
{
    TSystem::StringCopy(m_szName, name, -1);
}

TResource* TResourceNameMap::Item::GetResource()                       // 0x130e0
{
    return m_pResource;
}

const char* TResourceNameMap::Item::GetName() const                    // 0x130f0
{
    return m_szName;
}

TResourceNameMap::Item::Item()                                         // 0x13100
{
    m_pResource = 0;
    m_szName[0] = '\0';
}

TArray<TResourceNameMap::Item>& TResourceNameMap::GetItemList()        // 0x13110
{
    return m_aItems;
}

TResourceNameMap::Item* TResourceNameMap::AddItem()                    // 0x13120
{
    return &m_aItems.AddEmpty();
}

bool TResourceNameMap::SetNum(int num)                                  // 0x13170
{
    m_aItems.SetCapacity(num);
    return true;
}

TResourceNameMap::TResourceNameMap(int grow)                            // 0x13190
    : m_aItems(grow)
{
}

} // namespace Toshi
