// Toshi::TRenderInterface (TRenderInterface.dll) - the abstract renderer. Size 0x174.
// vtable: [0] GetClass [1] Delete [2] dtor [3..9] pure (with exported bodies) [10..31] see below.
// Layout from the constructor (0xab70) and the accessors; names follow OpenToshi where they agree.
#ifndef TRENDERINTERFACE_H
#define TRENDERINTERFACE_H
#include "TRenderAPI.h"
#include "TObject.h"
#include "TVector.h"
#include "TNodeList.h"
#include "TNodeTree.h"
#include "TResource.h"
#include "TRenderAdapter.h"
#include "TAnimation.h"
#include "TShader.h"
#include "TMaterialLibrary.h"

namespace Toshi {

class TKernelInterface;
class TRenderContext;
class TRenderCapture;
class TDebugText;
class TTextureResource;
class TViewport;
class TModel;

class TRENDER_API TRenderInterface : public TObject
{
    TOSHI_DECLARE_CLASS(TRenderInterface)
public:
    class DisplayParams
    {
    public:
        unsigned int uiWidth;               // +0x00
        unsigned int uiHeight;              // +0x04
        unsigned int uiColourDepth;         // +0x08
        unsigned int eDepthStencilFormat;   // +0x0C
        bool         bWindowed;             // +0x10
    };
    class FindFirstData
    {
    public:
        const char* m_szName;       // +0
        TResource*  m_pStart;       // +4
        int         m_iIndex;       // +8 (how many matches to skip)
    };
    class TRENDER_API BeginSceneEvent { };
    class TRENDER_API EndSceneEvent { };
    enum SYSRESOURCES { SYSRESOURCE_NUMOF = 19 };
    enum PRIMITIVETYPE { };

    TRenderInterface();
    virtual ~TRenderInterface();

    virtual bool CreateDisplay(const DisplayParams* params) = 0;                    // slot 3
    virtual bool DestroyDisplay() = 0;                                              // slot 4
    virtual bool Update(float deltaTime) = 0;                                       // slot 5
    virtual bool BeginScene() = 0;                                                  // slot 6
    virtual bool EndScene() = 0;                                                    // slot 7
    virtual TRenderAdapter::Mode::Device* GetCurrentDevice() const = 0;             // slot 8
    virtual const DisplayParams* GetCurrentDisplayParams() const = 0;               // slot 9
    virtual bool Create(TKernelInterface* kernel);                                  // slot 10
    virtual bool Destroy();                                                         // slot 11
    virtual void RenderIndexPrimitive(PRIMITIVETYPE type, unsigned int a, unsigned int b,
                                      const unsigned short* indices, const void* vertices, unsigned short stride) {} // slot 12
    virtual void DumpStats();                                                       // slot 13
    virtual void GetScreenOffset(TVector2& offset);                                 // slot 14
    virtual void SetScreenOffset(TVector2& offset);                                 // slot 15
    virtual float GetScreenAspectRatio() { return 4.0f / 3.0f; }                    // slot 16
    virtual float GetPixelAspectRatio() { return 1.0f; }                            // slot 17
    virtual bool SetPixelAspectRatio(float ratio) { return false; }                 // slot 18
    virtual bool IsTextureFormatSupported(TTEXTURERESOURCEFORMAT format) { return true; } // slot 19
    virtual bool Supports32BitTextures() { return false; }                          // slot 20
    virtual TRenderContext* CreateRenderContext() = 0;                              // slot 21
    virtual TRenderCapture* CreateCapture() = 0;                                    // slot 22
    virtual void DestroyCapture(TRenderCapture* capture) = 0;                       // slot 23
    virtual void SetLightDirectionMatrix(const TMatrix44& matrix) { m_LightDirection = matrix; } // slot 24
    virtual void SetLightColourMatrix(const TMatrix44& matrix) { m_LightColour = matrix; }       // slot 25
    virtual void ConnectDefaultViewportHandlers(TViewport& viewport);               // slot 26
    virtual TModel* CreateModel(const char* name, bool load) = 0;                   // slot 27
    virtual void FailedD3DMessageBox() {}                                           // slot 28
    virtual void FailedDisplayMessageBox() {}                                       // slot 29
protected:
    virtual bool CreateSystemResources();                                           // slot 30
    virtual void DestroySystemResources();                                          // slot 31

public:
    // ---- defined in the class body (inline/COMDAT region 0x6e10-0x7340 in the original)
    unsigned int GetRenderFrameCount() { return m_uiFrameCount; }
    TKeyframeLibraryManager& GetKeyframeLibraryManager() { return m_KeyframeLibraryManager; }
    TDebugText* GetDebugText() { return m_pDebugText; }
    TEmitter<TRenderInterface, EndSceneEvent>* GetEndSceneEmitter() { return &m_EndSceneEmitter; }
    TEmitter<TRenderInterface, BeginSceneEvent>* GetBeginSceneEmitter() { return &m_BeginSceneEmitter; }
    void GetLightColourMatrix(TMatrix44& matrix) const { matrix = m_LightColour; }
    const TMatrix44& GetLightColourMatrix() const { return m_LightColour; }
    void GetLightDirectionMatrix(TMatrix44& matrix) const { matrix = m_LightDirection; }
    const TMatrix44& GetLightDirectionMatrix() const { return m_LightDirection; }
    TTextureResource* GetInvalidTexture() { return m_pInvalidTexture; }
    static unsigned int __stdcall MakeRGBA(float r, float g, float b, float a)
    {
        return ((unsigned int)(a * 255.0f) << 24) | ((unsigned int)(r * 255.0f) << 16) |
               ((unsigned int)(g * 255.0f) << 8) | (unsigned int)(b * 255.0f);
    }
    static unsigned int __stdcall MakeRGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a)
    {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    static unsigned int __stdcall MakeRGBA(int r, int g, int b, int a)
    {
        return (a << 24) | (r << 16) | (g << 8) | b;
    }
    static TRenderInterface* __stdcall GetRenderer() { return s_Interface; }
    TMaterialLibraryManager* GetMaterialLibraryManager() { return &m_MaterialLibraryManager; }
    void DestroyMaterialLibraryManager() { m_MaterialLibraryManager.Destroy(); }
    void CreateMaterialLibraryManager() { m_MaterialLibraryManager.Create(); }

    // ---- TRenderInterface.cpp
    const TRenderAdapter::Mode::Device* FindDevice(const DisplayParams* params);
    TRenderAdapter::Mode::Device::DEPTHSTENCILFORMAT MakeDepthStencilFormat(unsigned int depthBits, unsigned int stencilBits);
    void DumpResourceTree(TResource* resource);
    TResource* CreateResource(const TClass* cls, const char* name, TResource* parent);
    TResource* CreateResourceInPlace(const TClass* cls, void* memory, const char* name, TResource* parent);
    void SetAsLastChild(TResource* resource);
    void DestroyResource(TResource* resource);
    void FlushDyingResources();
    TResource* FindResource(const char* name, TResource* parent);
    TResource* FindOrphanedResource(const char* name, TResource* parent);
    void FlushShaders();
    TRenderContext* SetCurrentRenderContext(TRenderContext* context);
    TRenderContext* GetCurrentRenderContext();
    TResource* GetSystemResource(SYSRESOURCES resource);
    TResource* FindNextResource(FindFirstData& data);
    void FindNextResourceInit(FindFirstData& data, const char* name, TResource* start);
    TResource* FindResource(unsigned int uid, TResource* parent);
    TNodeTree<TResource>* GetResourceTree();
    bool IsDisplayCreated();
    bool IsCreated();
    bool IsInScene();
    TKernelInterface* GetKernelInterface();
    TNodeList<TRenderAdapter>* GetAdapterList();
private:
    void DestroyDyingResources(TResource* resource);
    void DestroyResourceRecurse(TResource* resource);
    void DeleteResourceRecurse(TResource* resource);
    void DeleteResourceAtomic(TResource* resource);
    void FlushShadersRecurse(TResource* resource);
    void DestroyResourceTypeRecurse(const TClass& cls, TResource* resource);
    void DeleteResource(TResource* resource);
protected:
    void DestroyShaders();
    void DestroyAllResources();
public:

    // ---- members
    int                       m_iUnk4;                    // +0x04
    bool                      m_bInScene;                 // +0x08
    bool                      m_bCreated;                 // +0x09
    bool                      m_bDisplayCreated;          // +0x0A
    TVector2                  m_ScreenOffset;             // +0x0C
    TRenderContext*           m_pCurrentContext;          // +0x14
    TRenderContext*           m_pCreatedContext;          // +0x18
    TKernelInterface*         m_pKernel;                  // +0x1C
    TResource*                m_SystemResources[SYSRESOURCE_NUMOF]; // +0x20
    TMatrix44                 m_LightDirection;           // +0x6C
    TMatrix44                 m_LightColour;              // +0xAC
    TDebugText*               m_pDebugText;               // +0xEC
    TTextureResource*         m_pInvalidTexture;          // +0xF0
    TNodeList<TRenderAdapter> m_AdapterList;              // +0xF4
    TNodeTree<TResource>      m_Resources;                // +0x104
    unsigned int              m_uiNextUId;                // +0x11C
    unsigned int              m_uiFrameCount;             // +0x120
    bool                      m_bHasDyingResources;       // +0x124
    TMaterialLibraryManager   m_MaterialLibraryManager;   // +0x128
    TKeyframeLibraryManager   m_KeyframeLibraryManager;   // +0x150
    TEmitter<TRenderInterface, BeginSceneEvent> m_BeginSceneEmitter; // +0x15C
    TEmitter<TRenderInterface, EndSceneEvent>   m_EndSceneEmitter;   // +0x168

private:
    static TRenderInterface* s_Interface;
};

} // namespace Toshi
#endif
