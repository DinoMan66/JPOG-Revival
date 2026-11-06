// Toshi::TTextureFactory (TRenderInterface.dll, TTextureFactory.cpp, 0xde90-0xe490): a 1024-bucket
// hash of texture names. The platform renderer derives the real factory (TTextureFactoryHAL).
// The inline members (NameEntry, CreateHAL) are emitted with TRenderInterface.cpp (0xc6c0-0xc790).
#ifndef TTEXTUREFACTORY_H
#define TTEXTUREFACTORY_H
#include "TResource.h"
#include "TNodeList.h"
#include "TSystem.h"
#include "TRenderInterface.h"

namespace Toshi {

class TTextureResource;

class TRENDER_API TTextureFactory : public TResource                    // 0x4030
{
    TOSHI_DECLARE_CLASS(TTextureFactory)
public:
    enum { NUM_LISTS = 1024, MAX_NAME = 50 };

    class TRENDER_API NameEntry : public TNodeList<NameEntry>::TNode     // 0x44
    {
    public:
        NameEntry(const char* name, TTextureResource* texture)
        {
            m_pTexture = texture;
            if ((unsigned int)TSystem::StringLength(name) < MAX_NAME)
                TSystem::StringCopy(m_szName, name, MAX_NAME);
            else
                m_szName[0] = 0;
        }
        ~NameEntry() {}

        TTextureResource* GetTexture() { return m_pTexture; }
        const char* GetName() const { return m_szName; }

        char              m_szName[MAX_NAME];   // +0x0c
        TTextureResource* m_pTexture;           // +0x40

    private:
        NameEntry(const NameEntry&);            // not exported (operator= is)
    };

    TTextureFactory();
    virtual ~TTextureFactory();

    virtual void Dump();                                                // slot 10
    virtual TTextureResource* CreateTextureFromFile(const char* fileName, unsigned int flags) = 0;     // slot 11 (names unknown)
    virtual TTextureResource* CreateTextureFromMemory(void* data, unsigned int dataSize,
                                                      unsigned int flags, unsigned int width,
                                                      unsigned int height) = 0;                        // slot 12
    virtual TTextureResource* CreateEx(void* data, unsigned int dataSize, unsigned int width,
                                       unsigned int height, unsigned int mipLevels,
                                       TTEXTURERESOURCEFORMAT format, unsigned int flags) = 0;        // slot 13

    TTextureResource* FindTexture(const char* name);
    void DeregisterTexture(TTextureResource* texture);

    static TTextureFactory* __stdcall CreateHAL(TRenderInterface* renderer, const char* name, TResource* parent)
    {
        return (TTextureFactory*)renderer->CreateResource(TClass::Find("TTextureFactoryHAL", 0), name, parent);
    }

protected:
    unsigned int HashName(const char* name);
    NameEntry* RegisterTexture(const char* name, TTextureResource* texture);

    TNodeList<NameEntry> m_aLists[NUM_LISTS];   // +0x30

private:
    TTextureFactory(const TTextureFactory&);
    TTextureFactory& operator=(const TTextureFactory&);
};

} // namespace Toshi
#endif
