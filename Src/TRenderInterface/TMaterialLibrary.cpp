// TRenderInterface.dll - TMaterialLibrary.cpp: TMaterialLibrary, TMaterialLibraryManager
// (0xcee0-0xdb00 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TMaterialLibrary.h"
#include "TRenderInterface.h"
#include "TShader.h"
#include "TTextureFactory.h"
#include "TTextureResource.h"
#include "TFile.h"
#include "TCString.h"
#include "TSystem.h"


namespace Toshi {

// The platform shaders and materials (TRenderD3DInterface.dll) - only the virtual slots this file
// calls; the names are placeholders.
class TSkinShaderI : public TShader
{
public:
    virtual void _Slot12() = 0;
    virtual void _Slot13() = 0;
    virtual void _Slot14() = 0;
    virtual void _Slot15() = 0;
    virtual void _Slot16() = 0;
    virtual void _Slot17() = 0;
    virtual void _Slot18() = 0;
    virtual void _Slot19() = 0;
    virtual void _Slot20() = 0;
    virtual void _Slot21() = 0;
    virtual void _Slot22() = 0;
    virtual void _Slot23() = 0;
    virtual void _Slot24() = 0;
    virtual void _Slot25() = 0;
    virtual void _Slot26() = 0;
    virtual void _Slot27() = 0;
    virtual void _Slot28() = 0;
    virtual void _Slot29() = 0;
    virtual void _Slot30() = 0;
    virtual void _Slot31() = 0;
    virtual void _Slot32() = 0;
    virtual void _Slot33() = 0;
    virtual TMaterial* CreateMaterial(const char* name) = 0;   // slot 34
};

class TTerrainShaderI : public TShader
{
public:
    virtual void _Slot12() = 0;
    virtual void _Slot13() = 0;
    virtual void _Slot14() = 0;
    virtual void _Slot15() = 0;
    virtual void _Slot16() = 0;
    virtual void _Slot17() = 0;
    virtual void _Slot18() = 0;
    virtual void _Slot19() = 0;
    virtual void _Slot20() = 0;
    virtual void _Slot21() = 0;
    virtual void _Slot22() = 0;
    virtual void _Slot23() = 0;
    virtual void _Slot24() = 0;
    virtual void _Slot25() = 0;
    virtual void _Slot26() = 0;
    virtual void _Slot27() = 0;
    virtual void _Slot28() = 0;
    virtual void _Slot29() = 0;
    virtual void _Slot30() = 0;
    virtual void _Slot31() = 0;
    virtual void _Slot32() = 0;
    virtual void _Slot33() = 0;
    virtual void _Slot34() = 0;
    virtual void _Slot35() = 0;
    virtual void _Slot36() = 0;
    virtual void _Slot37() = 0;
    virtual void _Slot38() = 0;
    virtual void _Slot39() = 0;
    virtual void _Slot40() = 0;
    virtual void _Slot41() = 0;
    virtual void _Slot42() = 0;
    virtual void _Slot43() = 0;
    virtual void _Slot44() = 0;
    virtual void _Slot45() = 0;
    virtual void _Slot46() = 0;
    virtual void _Slot47() = 0;
    virtual void _Slot48() = 0;
    virtual void _Slot49() = 0;
    virtual void _Slot50() = 0;
    virtual void _Slot51() = 0;
    virtual void _Slot52() = 0;
    virtual void _Slot53() = 0;
    virtual void _Slot54() = 0;
    virtual void _Slot55() = 0;
    virtual void _Slot56() = 0;
    virtual void _Slot57() = 0;
    virtual void _Slot58() = 0;
    virtual void _Slot59() = 0;
    virtual void _Slot60() = 0;
    virtual void _Slot61() = 0;
    virtual void _Slot62() = 0;
    virtual void _Slot63() = 0;
    virtual void _Slot64() = 0;
    virtual void _Slot65() = 0;
    virtual void _Slot66() = 0;
    virtual void _Slot67() = 0;
    virtual void _Slot68() = 0;
    virtual void _Slot69() = 0;
    virtual void _Slot70() = 0;
    virtual void _Slot71() = 0;
    virtual TMaterial* CreateMaterial(const char* name) = 0;   // slot 72
};

class TTerrainDecalShaderI : public TShader
{
public:
    virtual void _Slot12() = 0;
    virtual void _Slot13() = 0;
    virtual void _Slot14() = 0;
    virtual void _Slot15() = 0;
    virtual void _Slot16() = 0;
    virtual void _Slot17() = 0;
    virtual void _Slot18() = 0;
    virtual void _Slot19() = 0;
    virtual void _Slot20() = 0;
    virtual void _Slot21() = 0;
    virtual void _Slot22() = 0;
    virtual void _Slot23() = 0;
    virtual void _Slot24() = 0;
    virtual void _Slot25() = 0;
    virtual void _Slot26() = 0;
    virtual void _Slot27() = 0;
    virtual void _Slot28() = 0;
    virtual void _Slot29() = 0;
    virtual void _Slot30() = 0;
    virtual void _Slot31() = 0;
    virtual void _Slot32() = 0;
    virtual void _Slot33() = 0;
    virtual void _Slot34() = 0;
    virtual void _Slot35() = 0;
    virtual void _Slot36() = 0;
    virtual void _Slot37() = 0;
    virtual void _Slot38() = 0;
    virtual void _Slot39() = 0;
    virtual void _Slot40() = 0;
    virtual void _Slot41() = 0;
    virtual void _Slot42() = 0;
    virtual void _Slot43() = 0;
    virtual void _Slot44() = 0;
    virtual void _Slot45() = 0;
    virtual void _Slot46() = 0;
    virtual void _Slot47() = 0;
    virtual void _Slot48() = 0;
    virtual void _Slot49() = 0;
    virtual void _Slot50() = 0;
    virtual void _Slot51() = 0;
    virtual void _Slot52() = 0;
    virtual void _Slot53() = 0;
    virtual void _Slot54() = 0;
    virtual void _Slot55() = 0;
    virtual void _Slot56() = 0;
    virtual void _Slot57() = 0;
    virtual void _Slot58() = 0;
    virtual void _Slot59() = 0;
    virtual void _Slot60() = 0;
    virtual void _Slot61() = 0;
    virtual void _Slot62() = 0;
    virtual void _Slot63() = 0;
    virtual void _Slot64() = 0;
    virtual void _Slot65() = 0;
    virtual void _Slot66() = 0;
    virtual void _Slot67() = 0;
    virtual void _Slot68() = 0;
    virtual void _Slot69() = 0;
    virtual void _Slot70() = 0;
    virtual void _Slot71() = 0;
    virtual void _Slot72() = 0;
    virtual TMaterial* CreateMaterial(const char* name) = 0;   // slot 73
};

class TSysShaderI : public TShader
{
public:
    virtual void _Slot12() = 0;
    virtual void _Slot13() = 0;
    virtual TMaterial* CreateMaterial(const char* name) = 0;   // slot 14
};

class TSpriteShaderI : public TShader
{
public:
    virtual TMaterial* CreateMaterial(const char* name) = 0;   // slot 12
};

// every platform material: blend mode in slot 12, textures from +0x40
class TPlatformMaterialI : public TMaterial
{
public:
    virtual void SetBlendMode(int mode) = 0;    // slot 12
    virtual void _Slot13() = 0;
    virtual void Finalise() = 0;                // slot 14 (skin materials)

    TTextureResource* m_apTextures[1];          // +0x40
};

const TTEXTURERESOURCEFORMAT TMaterialLibrary::gs_aRemapFormat[] =       // 0x10030538
{
    (TTEXTURERESOURCEFORMAT)1, (TTEXTURERESOURCEFORMAT)2, (TTEXTURERESOURCEFORMAT)3,
    (TTEXTURERESOURCEFORMAT)0, (TTEXTURERESOURCEFORMAT)0, (TTEXTURERESOURCEFORMAT)0,
    (TTEXTURERESOURCEFORMAT)4, (TTEXTURERESOURCEFORMAT)5, (TTEXTURERESOURCEFORMAT)0,
    (TTEXTURERESOURCEFORMAT)0, (TTEXTURERESOURCEFORMAT)8
};

TMaterialLibrary::TMaterialLibrary(TMaterialLibraryManager* manager)   // 0xcee0
{
    m_pManager = manager;
    m_uiNumTextures = 0;
    m_ppTextures = 0;
    m_uiNumMaterials = 0;
    m_ppMaterialNames = 0;
    m_ppMaterials = 0;
    m_pNameData = 0;
}

bool TMaterialLibrary::Create(const char* fileName)                    // 0xcf10
{
    TFile* file = TFile::Create(fileName, TFile::FileMode_Read);
    if (file && ReadHeader(file) && LoadTextures(file) && LoadMaterials(file))
    {
        TFile::Destroy(file);
        return true;
    }
    TFile::Destroy(file);
    Destroy();
    return false;
}

void TMaterialLibrary::Destroy()                                        // 0xcfa0
{
    if (m_ppMaterials)
    {
        for (unsigned int i = 0; i < m_uiNumMaterials; i++)
            m_ppMaterials[i]->DestroyResource();
    }
    m_uiNumMaterials = 0;
    tfree(m_ppMaterialNames);
    m_ppMaterialNames = 0;
    tfree(m_ppMaterials);
    m_ppMaterials = 0;
    tfree(m_pNameData);
    m_pNameData = 0;

    if (m_ppTextures)
    {
        for (unsigned int i = 0; i < m_uiNumTextures; i++)
        {
            if (m_ppTextures[i])
                m_ppTextures[i]->DestroyResource();
        }
    }
    m_uiNumTextures = 0;
    tfree(m_ppTextures);
    m_ppTextures = 0;
}

TMaterial* TMaterialLibrary::GetMaterial(const char* name)              // 0xd060
{
    int index = GetIndexForName(name, m_ppMaterialNames, m_uiNumMaterials);
    if (index >= 0 && (unsigned int)index < m_uiNumMaterials)
        return m_ppMaterials[index];
    return 0;
}

bool TMaterialLibrary::ReadHeader(TFile* file)                          // 0xd090
{
    struct { unsigned int m_uiMagic; unsigned int m_uiVersion; } header;
    if (file->Read(&header, sizeof(header)) != sizeof(header))
        return false;
    if (header.m_uiMagic != 0x314c4d54)     // "TML1"
        return false;
    if (header.m_uiVersion != 0)
        return false;
    return true;
}

bool TMaterialLibrary::LoadTextures(TFile* file)                        // 0xd0e0
{
    if (file->Read(&m_uiNumTextures, 4) != 4)
        return false;
    m_ppTextures = (TTextureResource**)tmalloc(m_uiNumTextures * sizeof(TTextureResource*), 0, -1);
    for (unsigned int i = 0; i < m_uiNumTextures; i++)
    {
        if (!LoadTexture(i, file))
            return false;
    }
    return true;
}

bool TMaterialLibrary::LoadTexture(int index, TFile* file)              // 0xd170
{
    TMLDISKTEXTUREHEADER header;
    if (file->Read(&header, sizeof(header)) != sizeof(header))
        return false;

    TTextureFactory* factory =
        (TTextureFactory*)TRenderInterface::GetRenderer()->GetSystemResource((TRenderInterface::SYSRESOURCES)18);
    void* data = tmalloc(header.m_uiDataSize, 0, -1);
    if (data)
    {
        if (file->Read(data, header.m_uiDataSize) != header.m_uiDataSize ||
            gs_aRemapFormat[header.m_usFormat] == 0)
        {
            tfree(data);
            return false;
        }
        m_ppTextures[header.m_uiIndex] =
            factory->CreateEx(data, header.m_uiDataSize, header.m_usWidth, header.m_usHeight, 0,
                              gs_aRemapFormat[header.m_usFormat], (header.m_usFlags & 2) ? 0 : 1);
        if (!m_ppTextures[header.m_uiIndex])
        {
            tfree(data);
            return false;
        }
    }
    tfree(data);
    return true;
}

bool TMaterialLibrary::LoadMaterials(TFile* file)                       // 0xd250
{
    if (file->Read(&m_uiNumMaterials, 4) != 4)
        return false;
    m_ppMaterialNames = (char**)tmalloc(m_uiNumMaterials * sizeof(char*), 0, -1);
    unsigned int nameSize = m_uiNumMaterials * 32;
    m_pNameData = (char*)tmalloc(nameSize, 0, -1);
    if (file->Read(m_pNameData, nameSize) != nameSize)
        return false;
    m_ppMaterials = (TMaterial**)tmalloc(m_uiNumMaterials * sizeof(TMaterial*), 0, -1);
    unsigned int i;
    for (i = 0; i < m_uiNumMaterials; i++)
    {
        m_ppMaterials[i] = 0;
        m_ppMaterialNames[i] = m_pNameData + i * 32;
    }
    for (i = 0; i < m_uiNumMaterials; i++)
    {
        if (!LoadMaterial(i, file))
            return false;
    }
    return true;
}

bool TMaterialLibrary::LoadMaterial(int index, TFile* file)             // 0xd340
{
    TMLDISKMATERIALHEADER header;
    if (file->Read(&header, sizeof(header)) != sizeof(header))
        return false;
    switch (header.m_usFlags & 0xf000)
    {
    case 0x0000: return LoadSkinMaterial(index, file, header);
    case 0x1000: return LoadTerrainMaterial(index, file, header);
    case 0x2000: return LoadTerrainDecalMaterial(index, file, header);
    case 0x3000: return LoadSystemMaterial(index, file, header);
    case 0x4000: return LoadSpriteMaterial(index, file, header);
    }
    return false;
}

bool TMaterialLibrary::LoadSkinMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header) // 0xd410
{
    unsigned int flags = 0;
    TPlatformMaterialI* material = (TPlatformMaterialI*)
        ((TSkinShaderI*)m_pManager->GetShader(0))->CreateMaterial(m_ppMaterialNames[index]);
    if (!material)
        return false;

    if (header.m_usFlags & 0x4)
    {
        material->SetBlendMode(1);
        flags = 2;
    }
    else if (header.m_usFlags & 0x8)
    {
        material->SetBlendMode(4);
        flags = 2;
    }
    else if (header.m_usFlags & 0x10)
    {
        material->SetBlendMode(3);
        flags = 2;
    }
    else
        material->SetBlendMode(0);

    material->SetNumTextures(header.m_usNumTextures);
    for (int i = 0; i < header.m_usNumTextures; i++)
    {
        unsigned int textureIndex;
        if (file->Read(&textureIndex, 4) != 4)
            return false;
        material->m_apTextures[i] = m_ppTextures[textureIndex];
    }
    if (header.m_usFlags & 0x1)
        flags |= 1;
    material->SetFlags(flags);
    material->Finalise();
    m_ppMaterials[index] = material;
    return true;
}

bool TMaterialLibrary::LoadTerrainMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header) // 0xd550
{
    TPlatformMaterialI* material = (TPlatformMaterialI*)
        ((TTerrainShaderI*)m_pManager->GetShader(1))->CreateMaterial(0);
    if (!material)
        return false;
    material->Create();
    material->SetNumTextures(header.m_usNumTextures);
    for (int i = 0; i < header.m_usNumTextures; i++)
    {
        unsigned int textureIndex;
        if (file->Read(&textureIndex, 4) != 4)
            return false;
        material->m_apTextures[i] = m_ppTextures[textureIndex];
    }
    m_ppMaterials[index] = material;
    return true;
}

bool TMaterialLibrary::LoadTerrainDecalMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header) // 0xd5f0
{
    TPlatformMaterialI* material = (TPlatformMaterialI*)
        ((TTerrainDecalShaderI*)m_pManager->GetShader(2))->CreateMaterial(0);
    if (!material)
        return false;
    material->Create();
    material->SetNumTextures(header.m_usNumTextures);
    for (int i = 0; i < header.m_usNumTextures; i++)
    {
        unsigned int textureIndex;
        if (file->Read(&textureIndex, 4) != 4)
            return false;
        material->m_apTextures[i] = m_ppTextures[textureIndex];
    }
    m_ppMaterials[index] = material;
    return true;
}

bool TMaterialLibrary::LoadSystemMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header) // 0xd690
{
    unsigned int flags = 0;
    TPlatformMaterialI* material = (TPlatformMaterialI*)
        ((TSysShaderI*)m_pManager->GetShader(0))->CreateMaterial(0);
    if (!material)
        return false;

    if (header.m_usFlags & 0x4)
        material->SetBlendMode(1);
    else if (header.m_usFlags & 0x8)
        material->SetBlendMode(4);
    else if (header.m_usFlags & 0x10)
        material->SetBlendMode(3);
    else
        material->SetBlendMode(0);

    material->SetNumTextures(header.m_usNumTextures);
    for (int i = 0; i < header.m_usNumTextures; i++)
    {
        unsigned int textureIndex;
        if (file->Read(&textureIndex, 4) != 4)
            return false;
        material->m_apTextures[i] = m_ppTextures[textureIndex];
    }
    if (header.m_usFlags & 0x1)
        flags = 1;
    material->SetFlags(flags);
    m_ppMaterials[index] = material;
    return true;
}

bool TMaterialLibrary::LoadSpriteMaterial(int index, TFile* file, TMLDISKMATERIALHEADER& header) // 0xd7a0
{
    TPlatformMaterialI* material = (TPlatformMaterialI*)
        ((TSpriteShaderI*)m_pManager->GetShader(4))->CreateMaterial(0);
    if (!material)
        return false;
    material->SetNumTextures(header.m_usNumTextures);
    for (int i = 0; i < header.m_usNumTextures; i++)
    {
        unsigned int textureIndex;
        if (file->Read(&textureIndex, 4) != 4)
            return false;
        material->m_apTextures[i] = m_ppTextures[textureIndex];
        m_ppTextures[textureIndex]->SetAddressModeMode((TTextureResource::ADDRESSMODE)2);
    }
    if (header.m_usFlags & 0x4)
        material->SetBlendMode(1);
    else if (header.m_usFlags & 0x8)
        material->SetBlendMode(4);
    else if (header.m_usFlags & 0x10)
        material->SetBlendMode(3);
    else
        material->SetBlendMode(0);
    m_ppMaterials[index] = material;
    return true;
}

int TMaterialLibrary::GetIndexForName(const char* name, char** names, int count) // 0xd880
{
    int lo = 0;
    int hi = count;
    while (hi > lo)
    {
        if (lo + 1 == hi)
        {
            if (TSystem::StringCompareNoCase(name, names[lo], 32) == 0)
                return lo;
            return -1;
        }
        int mid = lo + ((hi - lo) >> 1);
        int cmp = TSystem::StringCompareNoCase(name, names[mid], 32);
        if (cmp > 0)
            lo = mid;
        else if (cmp < 0)
            hi = mid;
        else
            return mid;
    }
    return -1;
}

// ---- TMaterialLibraryManager
TMaterialLibraryManager::TMaterialLibraryManager()                      // 0xd910
{
    m_pInvalidMaterial = 0;
    for (int i = 0; i < NUM_SHADERS; i++)
        m_apShaders[i] = 0;
}

void TMaterialLibraryManager::Create()                                  // 0xd940
{
    m_pInvalidMaterial = ((TSkinShaderI*)TRenderInterface::GetRenderer()->GetSystemResource(
        (TRenderInterface::SYSRESOURCES)9))->CreateMaterial(0);
    ((TPlatformMaterialI*)m_pInvalidMaterial)->SetBlendMode(0);
    m_pInvalidMaterial->SetNumTextures(4);
}

unsigned int TMaterialLibraryManager::LoadMaterialLibrary(const char* fileName) // 0xd980
{
    TMaterialLibrary* library = new TMaterialLibrary(this);
    if (!library->Create(fileName))
    {
        delete library;
        return (unsigned int)-1;
    }
    m_oLibraries.InsertTail(library);
    return (unsigned int)library;
}

void TMaterialLibraryManager::UnloadMaterialLibrary(unsigned int handle) // 0xd9f0
{
    if (handle == (unsigned int)-1)
        return;
    TMaterialLibrary* library = (TMaterialLibrary*)handle;
    m_oLibraries.Remove(library);
    library->Destroy();
    delete library;
}

TMaterial* TMaterialLibraryManager::GetMaterial(const char* name)       // 0xda40
{
    TNodeList<TMaterialLibrary>::Iterator end = m_oLibraries.End();
    for (TNodeList<TMaterialLibrary>::Iterator library = m_oLibraries.Begin(); library != end; ++library)
    {
        TMaterial* material = library->GetMaterial(name);
        if (material)
            return material;
    }
    return m_pInvalidMaterial;
}

void TMaterialLibraryManager::Destroy()                                 // 0xda90
{
    if (m_pInvalidMaterial)
    {
        m_pInvalidMaterial->DestroyResource();
        m_pInvalidMaterial = 0;
    }
}

TMaterialLibraryManager::~TMaterialLibraryManager()                     // 0xdab0
{
}

TMaterialLibrary::~TMaterialLibrary()                                   // 0xdb00
{
}

} // namespace Toshi
