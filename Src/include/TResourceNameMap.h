// Toshi::TResourceNameMap (TRenderInterface.dll, TResourceNameMap.cpp, 0x13030-0x133c0).
// A growable array of (name, resource) pairs searched by case-insensitive name.
#ifndef TRESOURCENAMEMAP_H
#define TRESOURCENAMEMAP_H
#include "TArray.h"
#include "TRenderAPI.h"

namespace Toshi {

class TResource;

class TRENDER_API TResourceNameMap
{
public:
    class TRENDER_API Item                                              // 0x14
    {
    public:
        Item();
        bool IsValid();
        void SetResource(TResource* resource);
        void SetName(const char* name);
        TResource* GetResource();
        const char* GetName() const;

        char       m_szName[16];    // +0x00
        TResource* m_pResource;     // +0x10
    };

    TResourceNameMap(int grow = 0);
    ~TResourceNameMap() {}

    Item* Find(const char* name);
    TArray<Item>& GetItemList();
    Item* AddItem();
    bool SetNum(int num);

    TArray<Item> m_aItems;          // +0x00

private:
    TResourceNameMap(const TResourceNameMap&);                          // not exported
    TResourceNameMap& operator=(const TResourceNameMap&);
};

} // namespace Toshi
#endif
