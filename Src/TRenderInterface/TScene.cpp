// TRenderInterface.dll - TScene.cpp (0x10920-0x10ef0 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TScene.h"
#include "TSceneObject.h"
#include "TCameraObject.h"
#include "TViewport.h"
#include "TRenderInterface.h"

namespace Toshi {

static int s_iDumpSceneOnBegin;                                         // 0x1002c9e0 (debug switches)
static int s_iDumpAllOnBegin;                                           // 0x1002c9e4

TScene::TScene()                                                        // 0x10920
{
    m_iNumNonSceneObjects = 0;
    m_auiUnk34[0] = 0;
    m_auiUnk34[1] = 0;
    m_auiUnk34[2] = 0;
    m_bInScene = false;
    m_uiSceneStamp = 0;
    m_fDeltaTime = 0.0f;
    m_fAbsoluteTime = 0.0f;
    m_oRenderMatrixStack.Reset();
    m_oRenderMatrixStack.Top().Identity();
    for (int i = 0; i < DEFCAMERA_NUMOF; i++)
        m_apDefaultCameras[i] = 0;
    m_oAmbientColour.Set(1.0f, 1.0f, 1.0f, 0.0f);
}

bool TScene::Begin()                                                    // 0x109b0
{
    if (s_iDumpSceneOnBegin)
        GetRenderer()->DumpResourceTree(this);
    if (s_iDumpAllOnBegin)
        GetRenderer()->DumpResourceTree(0);
    m_bInScene = true;
    m_oRenderMatrixStack.Reset();
    m_oRenderMatrixStack.Top().Identity();
    m_uiSceneStamp++;
    return true;
}

bool TScene::Update(float deltaTime)                                    // 0x10a10
{
    if (m_bInScene == true)
    {
        m_fDeltaTime = deltaTime;
        m_fAbsoluteTime += deltaTime;
        TResource* first = m_pChild;
        TResource* child = first;
        while (child)
        {
            TResource* next = child->m_pNext;
            if (next == first)
                next = 0;
            if (child->IsSceneObject() == true)
                ((TSceneObject*)child)->Update(*this);
            child = next;
        }
    }
    return true;
}

bool TScene::Render()                                                   // 0x10aa0
{
    if (m_bInScene == true)
    {
        m_pCurrentCamera = 0;
        m_pViewportRoot->Render(*this);
    }
    return true;
}

bool TScene::RenderScene()                                              // 0x10ad0
{
    if (m_bInScene == true && GetRenderer()->GetCurrentRenderContext())
    {
        TResource* first = m_pChild;
        TResource* child = first;
        while (child)
        {
            TResource* next = child->m_pNext;
            if (next == first)
                next = 0;
            if (child->IsSceneObject() == true)
                ((TSceneObject*)child)->Render(*this);
            child = next;
        }
    }
    return true;
}

bool TScene::End()                                                      // 0x10b50
{
    m_bInScene = false;
    return true;
}

bool TScene::Recurse(float deltaTime)                                   // 0x10b60
{
    TResource* first = m_pChild;
    TResource* child = first;
    while (child)
    {
        TResource* next = child->m_pNext;
        if (next == first)
            next = 0;
        if (child->IsA(TSceneObject::m_sClass))
            ((TSceneObject*)child)->Recurse(*this);
        child = next;
    }
    return true;
}

bool TScene::Create()                                                   // 0x10bd0
{
    TResource* viewportNull = GetRenderer()->CreateResource(&TNullResource::m_sClass, "ViewportNULL", this);
    m_pViewportRoot = (TNullSceneResource*)GetRenderer()->CreateResource(&TNullSceneResource::m_sClass,
                                                                          "Viewports", viewportNull);
    m_apDefaultCameras[0] = CreateCamera("CamLeft", this);
    m_apDefaultCameras[1] = CreateCamera("CamRight", this);
    m_apDefaultCameras[2] = CreateCamera("CamTop", this);
    m_apDefaultCameras[3] = CreateCamera("CamBottom", this);
    m_apDefaultCameras[4] = CreateCamera("CamUser", this);
    return TResource::Create();
}

TCameraObject* TScene::CreateCamera(const char* name, TResource* parent) // 0x10c80
{
    if (!parent)
        parent = this;
    TCameraObject* camera = (TCameraObject*)GetRenderer()->CreateResource(&TCameraObject::m_sClass, name, parent);
    camera->Create();
    return camera;
}

TViewport* TScene::CreateViewport(const char* name, TResource* parent, bool connectDefaultHandlers) // 0x10cc0
{
    if (!parent)
        parent = m_pViewportRoot;
    TViewport* viewport = (TViewport*)GetRenderer()->CreateResource(&TViewport::m_sClass, name, parent);
    viewport->Create(connectDefaultHandlers);
    return viewport;
}

TOSHI_DEFINE_CLASS(TScene, TResource)                                  // 0x10d10

TScene::~TScene()                                                       // 0x10d60
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TScene)                           // 0x10d70-0x10de0

} // namespace Toshi
