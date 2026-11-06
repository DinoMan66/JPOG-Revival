// Toshi::TMaterialLibrary / TMaterialLibraryManager (TRenderInterface.dll, TMaterialLibrary.cpp,
// 0xcee0-0xdb00). A material library is a "TML1" file: texture headers + data, then 32-byte
// material names (sorted, binary searched) and per-material headers.
#ifndef TMATERIALLIBRARY_H
#define TMATERIALLIBRARY_H
#include "TRenderAPI.h"
#include "TNodeList.h"

namespace Toshi {

class TFile;
class TMaterial;
class TShader;
class TTextureResource;
class TMaterialLibraryManager;

enum TTEXTURERESOURCEFORMAT { };

class TRENDER_API TMaterialLibrary : public TNodeList<TMaterialLibrary>::TNode  // 0x28
{
    friend class TMaterialLibraryManager;
    friend class TNodeList<TMaterialLibrary>;      // the manager's list deletes its libraries
public:
    struct TMLDISKMATERIALHEADER                // 8 bytes
    {
        unsigned int   m_uiUnk;                 // +0
        unsigned short m_usFlags;               // +4: type in bits 12-15, blend flags 0x1/0x4/0x8/0x10
        unsigned short m_usNumTextures;         // +6
    };
    struct TMLDISKTEXTUREHEADER                 // 0x1c bytes
    {
        unsigned int   m_uiIndex;               // +0x00
        unsigned int   m_uiDataSize;            // +0x04
        unsigned short m_usFlags;               // +0x08
        unsigned short m_usUnk0a;               // +0x0a
        unsigned int   m_uiUnk0c;               // +0x0c
        unsigned short m_usFormat;              // +0x10
        unsigned short m_usWidth;               // +0x12
        unsigned short m_usHeight;              // +0x14
        unsigned short m_usUnk16;               // +0x16
        unsigned int   m_uiUnk18;               // +0x18
    };

protected:
    TMaterialLibrary(TMaterialLibraryManager* manager);
    ~TMaterialLibrary();

    bool Create(const char* fileName);
    void Destroy();
    TMaterial* GetMaterial(const char* name);
    TTextureResource* GetTexture(int index) { return m_ppTextures[index]; }

    bool ReadHeader(TFile* file);
    bool LoadTextures(TFile* file);
    bool LoadTexture(int index, TFile* file);
    bool LoadMaterials(TFile* file);
    bool LoadMaterial(int index, TFile* file);
    bool LoadSkinMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header);
    bool LoadTerrainMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header);
    bool LoadTerrainDecalMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header);
    bool LoadSystemMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header);
    bool LoadSpriteMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header);
    int GetIndexForName(const char* name, char** names, int count);

    TMaterialLibraryManager* m_pManager;        // +0x0c
    unsigned int             m_uiNumTextures;   // +0x10
    TTextureResource**       m_ppTextures;      // +0x14
    unsigned int             m_uiNumMaterials;  // +0x18
    char**                   m_ppMaterialNames; // +0x1c
    TMaterial**              m_ppMaterials;     // +0x20
    char*                    m_pNameData;       // +0x24 (32 bytes per name)

private:
    static const TTEXTURERESOURCEFORMAT gs_aRemapFormat[];   // 0x10030538
    TMaterialLibrary(const TMaterialLibrary&);
    TMaterialLibrary& operator=(const TMaterialLibrary&);
};

class TRENDER_API TMaterialLibraryManager                   // 0x28
{
    friend class TMaterialLibrary;
public:
    enum { NUM_SHADERS = 5 };    // skin, terrain, terrain decal, system, sprite

    TMaterialLibraryManager();
    ~TMaterialLibraryManager();
    void Create();
    void Destroy();
    unsigned int LoadMaterialLibrary(const char* fileName);
    void UnloadMaterialLibrary(unsigned int handle);
    TMaterial* GetMaterial(const char* name);

    void SetShader(int index, TShader* shader) { m_apShaders[index] = shader; }
    TMaterial* GetInvalidMaterial() { return m_pInvalidMaterial; }

protected:
    TShader* GetShader(int index) { return m_apShaders[index]; }

    TNodeList<TMaterialLibrary> m_oLibraries;           // +0x00
    TMaterial*                  m_pInvalidMaterial;     // +0x10
    TShader*                    m_apShaders[NUM_SHADERS]; // +0x14

private:
    TMaterialLibraryManager& operator=(const TMaterialLibraryManager&);  // holds lists: not copyable
};

} // namespace Toshi
#endif
