// TRenderInterface.dll - TRenderInterface.cpp (0xab70-0xc640 in the original).
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TRenderInterface.h"
#include "TRenderContext.h"
#include "TCString.h"
#include "TSystem.h"
#include "TFile.h"
#include "TScene.h"
#include "TVertexFactoryResourceInterface.h"
#include "TIndexFactoryResourceInterface.h"
#include <string.h>

namespace Toshi {

TRenderInterface* TRenderInterface::s_Interface;

TRenderInterface::TRenderInterface()                                    // 0xab70
    : m_iUnk4(0), m_bInScene(false), m_bCreated(false), m_bDisplayCreated(false),
      m_ScreenOffset(TVector2(0.0f, 0.0f)), m_pCurrentContext(0), m_pCreatedContext(0), m_pKernel(0),
      m_pDebugText(0), m_pInvalidTexture(0), m_uiNextUId(1), m_uiFrameCount(0), m_bHasDyingResources(false),
      m_BeginSceneEmitter(this), m_EndSceneEmitter(this)
{
    TCString("Creating TRenderInterface\n").Print();
    s_Interface = this;
    memset(m_SystemResources, 0, sizeof(m_SystemResources));
    TCString("Created TRenderInterface\n").Print();

    m_LightColour = TMatrix44(128.0f, 128.0f, 128.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 0.0f,
                              0.0f, 0.0f, 0.0f, 0.0f);
    m_LightDirection = TMatrix44(0.47f, -0.74f, 0.47f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 0.0f,
                                 0.0f, 0.0f, 0.0f, 0.0f);
}

TRenderInterface::~TRenderInterface()                                   // 0xae30
{
    TCString("Destroying TRenderInterface\n").Print();
    s_Interface = 0;
    TCString("Destroyed TRenderInterface\n").Print();
}

bool TRenderInterface::Create(TKernelInterface* kernel)                 // 0xafb0
{
    m_pKernel = kernel;
    m_pCreatedContext = CreateRenderContext();
    if (m_pCreatedContext)
    {
        SetCurrentRenderContext(m_pCreatedContext);
        m_bCreated = true;
        return true;
    }
    return false;
}

bool TRenderInterface::Destroy()                                        // 0xaff0
{
    SetCurrentRenderContext(0);
    if (m_pCreatedContext)
    {
        delete m_pCreatedContext;
        m_pCreatedContext = 0;
    }
    m_pKernel = 0;
    m_bCreated = false;
    return true;
}

TResource* TRenderInterface::CreateResource(const TClass* cls, const char* name, TResource* parent) // 0xb7b0
{
    TResource* resource = (TResource*)cls->CreateObject();
    if (resource == 0)
        return resource;
    if (parent)
        parent->IsDying();
    m_Resources.Remove(*resource, false);
    if (parent)
        m_Resources.Insert(parent, resource);
    else
        m_Resources.InsertAtRoot(resource);
    resource->m_uiUId = m_uiNextUId++;
    resource->m_pRenderer = this;
    resource->SetName(name);
    return resource;
}

TResource* TRenderInterface::CreateResourceInPlace(const TClass* cls, void* memory, const char* name, TResource* parent) // 0xb8d0
{
    TResource* resource = (TResource*)cls->CreateObjectInPlace(memory);
    if (resource == 0)
        return resource;
    if (parent)
        parent->IsDying();
    m_Resources.Remove(*resource, false);
    if (parent)
        m_Resources.Insert(parent, resource);
    else
        m_Resources.InsertAtRoot(resource);
    resource->m_uiUId = m_uiNextUId++;
    resource->m_pRenderer = this;
    resource->SetName(name);
    resource->m_iFlags |= TResource::FLAG_INPLACE;
    return resource;
}

static const char* s_szDepthStencilFormats[5] =                         // 0x1002f8c0
{
    "D16_S0", "D15_S1", "D24_S0", "D24_S8", "D32_S0"
};

void TRenderInterface::DumpStats()                                      // 0xb030
{
    TFile* file = TFile::Create("TRenderInterface.txt", TFile::FileMode_CreateNew);
    if (file)
    {
        file->CPrintf("-\r\n");
        file->CPrintf("Toshi rendering interface through object [\"%s\"] Version : %u.%u\r\n",
                      GetClass().GetName(), GetClass().GetVersionMajor(), GetClass().GetVersionMinor());
        file->CPrintf("-\r\n");
        for (TNodeList<TRenderAdapter>::Iterator adapter = m_AdapterList.Begin(); adapter != m_AdapterList.End();
             ++adapter)
        {
            file->CPrintf("Adapter       : %d\r\n", adapter->GetAdapterIndex());
            file->CPrintf("  Driver      : %s\r\n", adapter->GetDriver().GetString(0));
            file->CPrintf("  Description : %s\r\n", adapter->GetDriverDescription().GetString(0));
            file->CPrintf("  Product ID  : %u\r\n", adapter->GetProductID());
            file->CPrintf("  Version     : %u\r\n", adapter->GetVersion());
            file->CPrintf("  Sub-version : %u\r\n", adapter->GetSubVersion());
            file->CPrintf("  Build       : %u\r\n", adapter->GetBuild());
            file->CPrintf("  GUID        : %u-%u-%u-%u\r\n", adapter->GetDeviceIdentifier()->Data1,
                          adapter->GetDeviceIdentifier()->Data2, adapter->GetDeviceIdentifier()->Data3,
                          adapter->GetDeviceIdentifier()->Data4);
            file->CPrintf("  Supports %d modes:\r\n", adapter->GetModeList()->Count());
            for (TNodeList<TRenderAdapter::Mode>::Iterator mode = adapter->GetModeList()->Begin();
                 mode != adapter->GetModeList()->End(); ++mode)
            {
                file->CPrintf("    Mode %d: %ux%u , %uhz: 16/32 bit Frame Buffer [%s/%s]\r\n",
                              mode->GetModeIndex(), mode->GetWidth(), mode->GetHeight(),
                              mode->GetRefreshRate());
                for (int i = 0; i < adapter->GetNumSupportedDevices(); i++)
                {
                    TRenderAdapter::Mode::Device* device = mode->GetDevice(i);
                    file->CPrintf("      Supported Device [\"%s\"]: \r\n", device->GetTypeString());
                    file->CPrintf("        Supports Hardware TnL? [\"%s\"]: \r\n",
                                  device->SupportsHardwareTransfomations() == true ? "Yes!" : "No");
                    file->CPrintf("        Supports Windowed? [\"%s\"]: \r\n",
                                  device->CanRenderWindowed() == true ? "Yes!" : "No");
                    file->CPrintf("        Supported Depth/Stencil Modes : ");
                    for (int j = 0; j < 5; j++)
                    {
                        if (device->IsDepthStencilFormatSupported(j) == true)
                            file->CPrintf("\"%s\" ", s_szDepthStencilFormats[j]);
                    }
                    file->CPrintf("\r\n");
                }
            }
        }
        TFile::Destroy(file);
    }
}

const TRenderAdapter::Mode::Device* TRenderInterface::FindDevice(const DisplayParams* params) // 0xb440
{
    for (TNodeList<TRenderAdapter>::Iterator adapter = m_AdapterList.Begin(); adapter != m_AdapterList.End();
         ++adapter)
    {
        for (TNodeList<TRenderAdapter::Mode>::Iterator mode = adapter->GetModeList()->Begin();
             mode != adapter->GetModeList()->End(); ++mode)
        {
            for (int i = 0; i < adapter->GetNumSupportedDevices(); i++)
            {
                TRenderAdapter::Mode::Device* device = mode->GetDevice(i);
                if (device->IsSoftware())
                    continue;

                bool colourOk = true;
                if (params->uiColourDepth == 32)
                {
                    if (!device->GetMode()->Is32Bit())
                        colourOk = false;
                }
                if (params->uiColourDepth == 16)
                {
                    if (!device->GetMode()->Is16Bit())
                        colourOk = false;
                }

                bool windowedOk = true;
                if (params->bWindowed == true)
                {
                    if (!device->CanRenderWindowed())
                        windowedOk = false;
                }

                if (device->GetMode()->GetWidth() == params->uiWidth &&
                    device->GetMode()->GetHeight() == params->uiHeight &&
                    device->IsDepthStencilFormatSupported(params->eDepthStencilFormat) == true &&
                    colourOk == true && windowedOk == true)
                    return device;
            }
        }
    }
    TCString().Format("Unable to find the passed device!\n");
    return 0;
}

TRenderAdapter::Mode::Device::DEPTHSTENCILFORMAT
TRenderInterface::MakeDepthStencilFormat(unsigned int depthBits, unsigned int stencilBits) // 0xb650
{
    typedef TRenderAdapter::Mode::Device::DEPTHSTENCILFORMAT DSF;
#ifndef TRI_MDSF
#define TRI_MDSF 2       // round 60: 53 -> 23
#endif
#if TRI_MDSF == 1
    switch (depthBits)
    {
    case 16: if (stencilBits == 0) return (DSF)0; break;
    case 15: if (stencilBits == 1) return (DSF)1; break;
    case 24:
        if (stencilBits == 1) return (DSF)2;
        if (stencilBits == 8) return (DSF)3;
        break;
    case 32: if (stencilBits == 0) return (DSF)4; break;
    }
    return (DSF)-1;
#elif TRI_MDSF == 2
    if (depthBits == 16)
    {
        if (stencilBits == 0)
            return (DSF)0;
        return (DSF)-1;
    }
    if (depthBits == 15)
    {
        if (stencilBits == 1)
            return (DSF)1;
        return (DSF)-1;
    }
    if (depthBits == 24)
    {
        if (stencilBits == 1)
            return (DSF)2;
        if (stencilBits == 8)
            return (DSF)3;
        return (DSF)-1;
    }
    if (depthBits == 32 && stencilBits == 0)
        return (DSF)4;
    return (DSF)-1;
#elif TRI_MDSF == 3
    DSF format = (DSF)-1;
    if (depthBits == 16)
    {
        if (stencilBits == 0)
            format = (DSF)0;
    }
    else if (depthBits == 15)
    {
        if (stencilBits == 1)
            format = (DSF)1;
    }
    else if (depthBits == 24)
    {
        if (stencilBits == 1)
            format = (DSF)2;
        else if (stencilBits == 8)
            format = (DSF)3;
    }
    else if (depthBits == 32)
    {
        if (stencilBits == 0)
            format = (DSF)4;
    }
    return format;
#else
    if (depthBits == 16)
    {
        if (stencilBits == 0)
            return (DSF)0;
    }
    else if (depthBits == 15)
    {
        if (stencilBits == 1)
            return (DSF)1;
    }
    else if (depthBits == 24)
    {
        if (stencilBits == 1)
            return (DSF)2;
        if (stencilBits == 8)
            return (DSF)3;
    }
    else if (depthBits == 32)
    {
        if (stencilBits == 0)
            return (DSF)4;
    }
    return (DSF)-1;
#endif
}

static int s_iDumpDepth;                                                 // 0x1002c980

static bool __stdcall DumpResourceCallback(TResource* resource, void* userData) // 0xa9a0
{
    for (int i = 0; i < s_iDumpDepth; i++)
        TCString().Format("    ").Print();
    TCString().Format("\"%s\" : class=\"%s\" : UID = %d %s %s\n", resource->GetName(),
                      resource->GetClass().GetName(), resource->GetUId(),
                      resource->IsDying() == true ? "[Dying]" : "", "").Print();
    return true;
}

static bool __stdcall DumpResourceLeave(TResource* resource, void* userData)    // 0xc620
{
    s_iDumpDepth--;
    return true;
}

static bool __stdcall DumpResourceEnter(TResource* resource, void* userData)    // 0xc630
{
    s_iDumpDepth++;
    return true;
}

void TRenderInterface::DumpResourceTree(TResource* resource)            // 0xb6b0
{
    s_iDumpDepth = 0;
    TCString().Format(">--Renderer resource dump Start--\n").Print();
    if (resource)
        m_Resources.Recurse(DumpResourceEnter, DumpResourceLeave, DumpResourceCallback, resource, false, 0);
    else
        m_Resources.Recurse(DumpResourceEnter, DumpResourceLeave, DumpResourceCallback,
                            m_Resources.m_oRoot.m_pChild, true, 0);
    TCString().Format("There are %d resources allocated.\n", m_Resources.Count()).Print();
    TCString().Format("<--Renderer resource dump End--\n").Print();
}

#if defined(TRI_SALC) && TRI_SALC != 0
static inline TResource* ParentRet(TResource* r)
{
    if (r->m_pParent == 0 || r->m_pParent == r->m_pTree->GetRoot())
        return 0;
    return r->m_pParent;
}
#endif

void TRenderInterface::SetAsLastChild(TResource* resource)              // 0xba00
{
#if defined(TRI_SALC) && TRI_SALC == 1
    if (resource && ParentRet(resource))
    {
        TResource* parent = resource->Parent();
#elif defined(TRI_SALC) && TRI_SALC == 2
    if (resource && resource->Parent())
    {
        TResource* parent = resource->m_pParent;
        if (parent == 0 || parent == resource->m_pTree->GetRoot())
            parent = 0;
#elif defined(TRI_SALC) && TRI_SALC == 3
    if (resource && ParentRet(resource))
    {
        TResource* parent = resource->m_pParent;
        if (parent == 0 || parent == resource->m_pTree->GetRoot())
            parent = 0;
#else
    if (resource && resource->Parent())
    {
        TResource* parent = resource->Parent();
#endif
        m_Resources.Remove(*resource, false);
        if (parent)
            m_Resources.Insert(parent, resource);
        else
            m_Resources.InsertAtRoot(resource);
    }
}

void TRenderInterface::DestroyResource(TResource* resource)             // 0xbaf0
{
    if (!(resource->m_iFlags & TResource::FLAG_DYING))
    {
        m_bHasDyingResources = true;
        resource->m_iFlags |= TResource::FLAG_DYING;
        DestroyResourceRecurse(resource->m_pChild);
        resource->Invalidate();
    }
}

void TRenderInterface::FlushDyingResources()                            // 0xbb20
{
    while (m_bHasDyingResources == true)
    {
        m_bHasDyingResources = false;
        DestroyDyingResources(m_Resources.m_oRoot.m_pChild);
    }
}

// callback of FindOrphanedResource / FindNextResource (0xaa90)
struct FindResourceData
{
    int                              m_iCount;
    TResource*                       m_pResult;
    TRenderInterface::FindFirstData* m_pFind;
};

static bool __stdcall FindResourceCallback(TResource* resource, void* userData)
{
    FindResourceData* data = (FindResourceData*)userData;
    if (TSystem::StringCompareNoCase(resource->GetName(), data->m_pFind->m_szName, -1) == 0)
    {
        if (data->m_iCount++ == data->m_pFind->m_iIndex)
        {
            data->m_pFind->m_iIndex++;
            data->m_pResult = resource;
            return false;
        }
    }
    return true;
}

// Splits a resource path ("a/b\\c") into its names (0xc310 ctor, 0xc2d0 GetNextToken).
static char* s_szPathSeparators = "\\/";                                   // 0x1002c600

class TResourcePathTokenizer
{
public:
    TResourcePathTokenizer(const char* path);
    char* GetNextToken();

    char  m_szPath[420];        // +0x000
    char* m_pszToken;           // +0x1A4
};

TResourcePathTokenizer::TResourcePathTokenizer(const char* path)       // 0xc310
{
    TSystem::StringCopy(m_szPath, path, -1);
    m_pszToken = strtok(m_szPath, s_szPathSeparators);
}

char* TResourcePathTokenizer::GetNextToken()                            // 0xc2d0
{
    char* token = m_pszToken;
    if (token)
        m_pszToken = strtok(0, s_szPathSeparators);
    return token;
}

TResource* TRenderInterface::FindResource(const char* name, TResource* parent) // 0xbb70
{
    TResourcePathTokenizer tokens(name);
    TResource* resource = parent ? parent->m_pChild : m_Resources.m_oRoot.m_pChild;
    if (resource)
    {
        TResource* list = resource;
        char* token;
        while ((token = tokens.GetNextToken()) != 0)
        {
            resource = list;
            while (resource)
            {
                if (TSystem::StringCompareNoCase(resource->GetName(), token, -1) == 0)
                    break;
                resource = resource->m_pNext;
                if (resource == list)
                    resource = 0;
            }
            if (!resource)
                return 0;
            list = resource->m_pChild;
        }
    }
    return resource;
}

TResource* TRenderInterface::FindOrphanedResource(const char* name, TResource* parent) // 0xbc30
{
    FindFirstData find;
    find.m_szName = name;
    find.m_pStart = parent;
    find.m_iIndex = 0;
    FindResourceData data;
    data.m_iCount = 0;
    data.m_pResult = 0;
    data.m_pFind = &find;
    if (parent)
        m_Resources.RecurseSimple(FindResourceCallback, parent, false, &data);
    else
        m_Resources.RecurseSimple(FindResourceCallback, m_Resources.m_oRoot.m_pChild, true, &data);
    return data.m_pResult;
}

void TRenderInterface::DestroyDyingResources(TResource* resource)       // 0xbfc0
{
    TResource* start = resource;
    while (resource)
    {
#ifndef TRI_DDR_PLAIN
        TNodeTree<TResource>::TNode* node = resource;
#define DDR_CHILD node->m_pChild
#else
#define DDR_CHILD resource->m_pChild
#endif
        TResource* next = resource->m_pNext;
        if (next == start)
            next = 0;
        if (resource->m_iFlags & TResource::FLAG_DYING)
        {
            if (resource == start)
                start = next;
            DeleteResource(resource);
        }
        else if (DDR_CHILD)
            DestroyDyingResources(DDR_CHILD);
#undef DDR_CHILD
        resource = next;
    }
}

void TRenderInterface::DestroyResourceRecurse(TResource* resource)      // 0xc040
{
#ifndef TRI_DRR
#define TRI_DRR 3       // round 57: the original walks the children through a TNode* local
#endif
#if TRI_DRR & 2
    TResource* last = 0;
    if (resource)
        last = resource->m_pPrev;
#else
    TResource* last = resource ? resource->m_pPrev : 0;
#endif
    while (resource)
    {
#if TRI_DRR & 1
        TNodeTree<TResource>::TNode* node = resource;
#endif
        TResource* next = resource->m_pNext;
        if (resource == last)
            next = 0;
        if (!(resource->m_iFlags & TResource::FLAG_DYING))
        {
            m_bHasDyingResources = true;
            resource->m_iFlags |= TResource::FLAG_DYING;
#if TRI_DRR & 1
            if (node->m_pChild)
                DestroyResourceRecurse(node->m_pChild);
#elif TRI_DRR & 4
            if (resource->Child())
                DestroyResourceRecurse(resource->Child());
#else
            if (resource->m_pChild)
                DestroyResourceRecurse(resource->m_pChild);
#endif
            resource->Invalidate();
        }
        resource = next;
    }
}

void TRenderInterface::DeleteResourceRecurse(TResource* resource)       // 0xc0d0
{
    while (resource)
    {
        TResource* next = resource->m_pNext;
        if (next == resource)
            next = 0;
        DeleteResourceAtomic(resource);
        resource = next;
    }
}

void TRenderInterface::DeleteResourceAtomic(TResource* resource)        // 0xc120
{
    if (resource)
    {
        if (resource->m_pChild)
            DeleteResourceRecurse(resource->m_pChild);
        if (resource->IsCreated() == true)
            resource->OnDestroy();
        m_Resources.Remove(*resource, false);
        if (resource->m_iFlags & TResource::FLAG_INPLACE)
            resource->~TResource();
        else
            delete resource;
    }
}

bool TRenderInterface::CreateSystemResources()                          // 0xbce0
{
    TCString("  Adding base resources\n").Print();

    m_SystemResources[0] = CreateResource(&TNullResource::m_sClass, "VFactories", 0);

    TVertexFactoryFormat sysFormat;
    m_SystemResources[1] = CreateResource(TClass::Find("TVertexFactoryResource", 0), "VFSYSVNDUV1",
                                          m_SystemResources[0]);
    sysFormat.m_uiNumStreams = 1;
    sysFormat.m_aStreamFormats[0].m_uiVertexSize = 24;
    sysFormat.m_aStreamFormats[0].m_uiUnk = 0;
    ((TVertexFactoryResourceInterface*)m_SystemResources[1])->Create(&sysFormat, 11000, 0);

    TVertexFactoryFormat skinFormat;
    m_SystemResources[2] = CreateResource(TClass::Find("TVertexFactoryResource", 0), "VFSKIN",
                                          m_SystemResources[0]);
    skinFormat.m_uiNumStreams = 1;
    skinFormat.m_aStreamFormats[0].m_uiVertexSize = 40;
    skinFormat.m_aStreamFormats[0].m_uiUnk = 0;
    ((TVertexFactoryResourceInterface*)m_SystemResources[2])->Create(&skinFormat, 11000, 0);

    m_SystemResources[7] = CreateResource(&TNullResource::m_sClass, "Shaders", 0);
    m_SystemResources[8] = CreateResource(TClass::Find("TSysShaderHAL", 0), "SHSYS", m_SystemResources[7]);
    m_SystemResources[8]->Create();
    m_SystemResources[9] = CreateResource(TClass::Find("TSkinShaderHAL", 0), "SHSKIN", m_SystemResources[7]);
    m_SystemResources[9]->Create();
    m_SystemResources[11] = CreateResource(TClass::Find("TLineShaderHAL", 0), "SHLINE", m_SystemResources[7]);
    m_SystemResources[11]->Create();

    m_SystemResources[5] = CreateResource(&TNullResource::m_sClass, "IFactories", 0);
    m_SystemResources[6] = CreateResource(TClass::Find("TIndexFactoryResource", 0), "IFSYS",
                                          m_SystemResources[5]);
    ((TIndexFactoryResourceInterface*)m_SystemResources[6])->Create(11000, 0);

    m_SystemResources[12] = CreateResource(&TNullResource::m_sClass, "Storage", 0);
    m_SystemResources[13] = CreateResource(&TNullResource::m_sClass, "Meshes", m_SystemResources[12]);
    m_SystemResources[14] = CreateResource(&TNullResource::m_sClass, "Textures", m_SystemResources[12]);
    m_SystemResources[15] = CreateResource(&TNullResource::m_sClass, "Sounds", m_SystemResources[12]);
    m_SystemResources[16] = CreateResource(&TNullResource::m_sClass, "GFXFiles", m_SystemResources[12]);
    m_SystemResources[17] = CreateResource(&TScene::m_sClass, "Scene", 0);
    m_SystemResources[17]->Create();
    m_SystemResources[18] = CreateResource(TClass::Find("TTextureFactoryHAL", 0), "TextureFactory", 0);
    m_SystemResources[18]->Create();

    // the material library manager's default shaders (+0x14..+0x20)
    m_MaterialLibraryManager.SetShader(0, (TShader*)m_SystemResources[9]);
    m_MaterialLibraryManager.SetShader(1, (TShader*)m_SystemResources[10]);
    m_MaterialLibraryManager.SetShader(2, (TShader*)m_SystemResources[10]);
    m_MaterialLibraryManager.SetShader(3, (TShader*)m_SystemResources[8]);
    return true;
}

static bool s_bFlushShaderSorted;                                       // 0x1002c984

void TRenderInterface::FlushShaders()                                   // 0xbca0
{
    if (m_bInScene == true)
    {
        s_bFlushShaderSorted = false;
        FlushShadersRecurse(m_SystemResources[7]->m_pChild);
        s_bFlushShaderSorted = true;
        FlushShadersRecurse(m_SystemResources[7]->m_pChild);
    }
}

void TRenderInterface::FlushShadersRecurse(TResource* resource)         // 0xc190
{
    TResource* start = resource;
    while (resource)
    {
        if (resource->IsA(TShader::m_sClass))
            ((TShader*)resource)->Flush();
        resource = resource->m_pNext;
        if (resource == start)
            break;
    }
}

void TRenderInterface::DestroyResourceTypeRecurse(const TClass& cls, TResource* resource) // 0xc1f0
{
    TResource* start = resource;
    while (resource)
    {
#ifndef TRI_DDR_PLAIN
        TNodeTree<TResource>::TNode* node = resource;
#define DDR_CHILD node->m_pChild
#else
#define DDR_CHILD resource->m_pChild
#endif
        TResource* next = resource->m_pNext;
        if (next == start)
            next = 0;
        if (DDR_CHILD)
            DestroyResourceTypeRecurse(cls, DDR_CHILD);
#undef DDR_CHILD
        if (!(resource->m_iFlags & TResource::FLAG_DYING) && resource->IsA(cls))
            DestroyResource(resource);
        resource = next;
    }
}

TOSHI_DEFINE_CLASS(TRenderInterface, TObject)

void TRenderInterface::DeleteResource(TResource* resource)              // 0xc350
{
    DeleteResourceRecurse(resource->m_pChild);
    DeleteResourceAtomic(resource);
}

void TRenderInterface::DestroyShaders()                                 // 0xc380
{
    DestroyResourceTypeRecurse(TShader::m_sClass, m_SystemResources[7]);
}

void TRenderInterface::DestroyAllResources()                            // 0xc3a0
{
    DestroyResourceRecurse(m_Resources.m_oRoot.m_pChild);
    FlushDyingResources();
}

void TRenderInterface::DestroySystemResources()                         // 0xc3c0
{
    TCString("  Destroying all left over resources\n").Print();
    DestroyAllResources();
}

void TRenderInterface::ConnectDefaultViewportHandlers(TViewport& viewport) // 0xc400
{
}

TRenderContext* TRenderInterface::SetCurrentRenderContext(TRenderContext* context) // 0xc410
{
    TRenderContext* old = m_pCurrentContext;
    m_pCurrentContext = context;
    return old;
}

TRenderContext* TRenderInterface::GetCurrentRenderContext()             // 0xc420
{
    return m_pCurrentContext;
}

TResource* TRenderInterface::GetSystemResource(SYSRESOURCES resource)   // 0xc430
{
    return m_SystemResources[resource];
}

TResource* TRenderInterface::FindNextResource(FindFirstData& find)      // 0xc440
{
    FindResourceData data;
    data.m_iCount = 0;
    data.m_pResult = 0;
    data.m_pFind = &find;
    if (find.m_pStart)
        m_Resources.RecurseSimple(FindResourceCallback, find.m_pStart, false, &data);
    else
        m_Resources.RecurseSimple(FindResourceCallback, m_Resources.m_oRoot.m_pChild, true, &data);
    return data.m_pResult;
}

void TRenderInterface::FindNextResourceInit(FindFirstData& find, const char* name, TResource* start) // 0xc4a0
{
    find.m_szName = name;
    find.m_pStart = start;
}

TResource* TRenderInterface::FindResource(unsigned int uid, TResource* parent) // 0xc4c0
{
    return 0;
}

TNodeTree<TResource>* TRenderInterface::GetResourceTree()               // 0xc4d0
{
    return &m_Resources;
}

bool TRenderInterface::IsDisplayCreated()                               // 0xc4e0
{
    return m_bDisplayCreated;
}

bool TRenderInterface::IsCreated()                                      // 0xc4f0
{
    return m_bCreated;
}

bool TRenderInterface::IsInScene()                                      // 0xc500
{
    return m_bInScene;
}

TKernelInterface* TRenderInterface::GetKernelInterface()                // 0xc510
{
    return m_pKernel;
}

void TRenderInterface::SetScreenOffset(TVector2& offset)                // 0xc520
{
    m_ScreenOffset = offset;
}

void TRenderInterface::GetScreenOffset(TVector2& offset)                // 0xc540
{
    offset = m_ScreenOffset;
}

TNodeList<TRenderAdapter>* TRenderInterface::GetAdapterList()           // 0xc560
{
    return &m_AdapterList;
}

bool TRenderInterface::EndScene()                                       // 0xc570
{
    return true;
}

bool TRenderInterface::BeginScene()                                     // 0xc580
{
    m_uiFrameCount++;
    return true;
}

bool TRenderInterface::Update(float deltaTime)                          // 0xc590
{
    FlushDyingResources();
    return true;
}

bool TRenderInterface::DestroyDisplay()                                 // 0xc5b0
{
    m_bDisplayCreated = false;
    return true;
}

bool TRenderInterface::CreateDisplay(const DisplayParams* params)       // 0xc5c0
{
    m_bDisplayCreated = true;
    return true;
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TRenderInterface)                          // 0xc5d0-0xc610

} // namespace Toshi

// 0xc640: read by the shader Flush() implementations (first pass unsorted, second pass sorted)
extern "C" TRENDER_API bool __stdcall TRenderInterface_GetFlushShaderSorted()
{
    return Toshi::s_bFlushShaderSorted;
}
