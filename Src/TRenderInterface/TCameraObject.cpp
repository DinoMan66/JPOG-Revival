// TRenderInterface.dll - TCameraObject.cpp: TCameraViewportHandler, TCameraObject (0xea70-0xf4c0)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TCameraObject.h"
#include "TTransformObject.h"
#include "TViewport.h"
#include "TScene.h"
#include "TRenderInterface.h"
#include "TRenderContext.h"
#include "TMath.h"
#include <math.h>

namespace Toshi {

static inline float DegToRad(float degrees) { return TMath::PI / 180.0f * degrees; }

// ---- TCameraViewportHandler
TCameraViewportHandler::TCameraViewportHandler(TCameraObject& camera)   // 0xeaf0
    : m_pCamera(&camera)
{
}

TRenderInterface* TCameraViewportHandler::GetRenderer()                 // 0xeb30
{
    if (GetCamera().GetViewport())
        return GetCamera().GetViewport()->GetRenderer();
    return 0;
}

bool __stdcall TCameraViewportHandler::OnBeginRenderEvent(TCameraViewportHandler* handler, TViewport* viewport,
                                                          TViewport::BeginRenderEvent* event) // 0xeb60
{
    TCameraObject& camera = *handler->m_pCamera;
    TRenderContext* context = viewport->GetRenderContext();
    if (!camera.m_bEnabled)
        return true;

    TRenderContext::PROJECTIONPARAMS params = context->GetProjectionParameters();
    params.m_Centre.x = camera.GetProjectionCentreX() * context->GetViewportParameters().fWidth;
    params.m_Centre.y = camera.GetProjectionCentreY() * context->GetViewportParameters().fHeight;
    if (camera.GetFOVMode() == TCameraObject::FOVMODE_HORIZONTAL)
    {
        params.m_Proj.x = params.m_Proj.y =
            0.5f * context->GetViewportParameters().fWidth / tan(camera.GetFOV() * 0.5f);
        if (camera.m_eProjType == TCameraObject::PROJTYPE_PERSPECTIVE)
            params.m_Proj.y *= TRenderInterface::GetRenderer()->GetPixelAspectRatio();
    }
    else
    {
        params.m_Proj.y = params.m_Proj.x =
            0.5f * context->GetViewportParameters().fHeight / tan(camera.GetFOV() * 0.5f);
        if (camera.m_eProjType == TCameraObject::PROJTYPE_PERSPECTIVE)
            params.m_Proj.x /= TRenderInterface::GetRenderer()->GetPixelAspectRatio();
    }
    params.m_fNearClip = camera.GetNear();
    params.m_fFarClip = camera.GetFar();
    context->SetProjectionParameters(params);

    if (camera.m_eProjType == TCameraObject::PROJTYPE_PERSPECTIVE)
        context->SetProjType(TRenderContext::PROJTYPE_PERSPECTIVE);
    else if (camera.m_eProjType == TCameraObject::PROJTYPE_ORTHOGRAPHIC)
        context->SetProjType(TRenderContext::PROJTYPE_ORTHOGRAPHIC);

    TMatrix44 matrix;
    camera.GetTransformObject()->GetLocalMatrix(matrix);
    matrix.InvertOrthogonal();
    context->SetWorldViewMatrix(matrix);
    context->SetModelViewMatrix(matrix);
    context->Update();
    return true;
}

bool __stdcall TCameraViewportHandler::OnRenderEvent(TCameraViewportHandler* handler, TViewport* viewport,
                                                     TViewport::RenderEvent* event) // 0xed00
{
    TCameraObject* camera = handler->m_pCamera;
    TRenderContext* context = viewport->GetRenderContext();
    if (!camera->m_bEnabled)
        return true;

    event->m_rScene.GetRenderMatrixStack().Push(context->GetWorldViewMatrix());
    TCameraObject* oldCamera = event->m_rScene.GetCurrentCamera();
    event->m_rScene.SetCurrentCamera(camera);
    event->m_rScene.RenderScene();
    event->m_rScene.SetCurrentCamera(oldCamera);
    event->m_rScene.GetRenderMatrixStack().Pop();
    return true;
}

bool __stdcall TCameraViewportHandler::OnEndRenderEvent(TCameraViewportHandler* handler, TViewport* viewport,
                                                        TViewport::EndRenderEvent* event) // 0xee30
{
    if (!handler->m_pCamera->m_bEnabled)
        return true;
    handler->GetRenderer()->FlushShaders();
    return true;
}

void TCameraViewportHandler::ConnectEventHandlers()                     // 0xee70
{
    if (m_pCamera->GetViewport())
    {
        m_oUpdateListener.Connect(&m_pCamera->GetViewport()->GetUpdateEmitter(), this, OnUpdateEvent, 0);
        m_oBeginRenderListener.Connect(&m_pCamera->GetViewport()->GetBeginRenderEmitter(), this,
                                       OnBeginRenderEvent, 0);
        m_oRenderListener.Connect(&m_pCamera->GetViewport()->GetRenderEmitter(), this, OnRenderEvent, 0);
        m_oEndRenderListener.Connect(&m_pCamera->GetViewport()->GetEndRenderEmitter(), this,
                                     OnEndRenderEvent, 0);
    }
}

void TCameraViewportHandler::DisconnectEventHandlers()                  // 0xef00
{
    m_oUpdateListener.Disconnect();
    m_oBeginRenderListener.Disconnect();
    m_oRenderListener.Disconnect();
    m_oEndRenderListener.Disconnect();
}

// ---- TCameraObject
TCameraObject::TCameraObject()                                          // 0xef30
    : m_oViewportHandler(*this), m_fNear(1.0f), m_fFar(100.0f), m_fFOV(DegToRad(90.0f)),
      m_eFOVMode(FOVMODE_HORIZONTAL), m_eProjType(PROJTYPE_PERSPECTIVE), m_fProjCentreX(0.5f),
      m_fProjCentreY(0.5f), m_pTransformObject(0), m_pLookatTarget(0), m_pViewport(0), m_bEnabled(true)
{
}

bool TCameraObject::Create()                                            // 0xefc0
{
    m_pTransformObject = (TTransformObject*)GetRenderer()->CreateResource(&TTransformObject::m_sClass, 0, Parent());
    m_pTransformObject->Create();
    SetParent(m_pTransformObject);
    return TResource::Create();
}

float TCameraObject::SetFOV(float fov)                                  // 0xf010
{
    float old = m_fFOV;
    fov = TMath::Max(DegToRad(1.0f), fov);
    m_fFOV = TMath::Min(DegToRad(180.0f), fov);
    return old;
}

float TCameraObject::SetProjectionCentreX(float x)                      // 0xf080
{
    float old = m_fProjCentreX;
    m_fProjCentreX = TMath::Max(TMath::Min(x, 1.0f), 0.0f);
    return old;
}

float TCameraObject::SetProjectionCentreY(float y)                      // 0xf0d0
{
    float old = m_fProjCentreY;
    m_fProjCentreY = TMath::Max(TMath::Min(y, 1.0f), 0.0f);
    return old;
}

TViewport* TCameraObject::SetViewport(TViewport* viewport)              // 0xf120
{
    if (m_pViewport)
        m_oViewportHandler.DisconnectEventHandlers();
    m_pViewport = viewport;
    if (m_pViewport)
        m_oViewportHandler.ConnectEventHandlers();
    return m_pViewport;
}

TOSHI_DEFINE_CLASS(TCameraObject, TSceneObject)                        // 0xf170

bool TCameraObject::Enable(bool enable)                                 // 0xf1c0
{
    bool old = m_bEnabled;
    m_bEnabled = enable;
    return old;
}

TViewport* TCameraObject::GetViewport() const { return m_pViewport; }                   // 0xf1e0
TTransformObject* TCameraObject::GetTransformObject() { return m_pTransformObject; }    // 0xf1f0
float TCameraObject::GetProjectionCentreY() { return m_fProjCentreY; }                  // 0xf200
float TCameraObject::GetProjectionCentreX() { return m_fProjCentreX; }                  // 0xf210
float TCameraObject::GetFOV() { return m_fFOV; }                                        // 0xf220
TCameraObject::FOVMODE TCameraObject::GetFOVMode() { return m_eFOVMode; }               // 0xf230
float TCameraObject::GetFar() { return m_fFar; }                                        // 0xf240
float TCameraObject::GetNear() { return m_fNear; }                                      // 0xf250

TSceneObject* TCameraObject::SetLookatTarget(TSceneObject* target)      // 0xf260
{
    TSceneObject* old = m_pLookatTarget;
    m_pLookatTarget = target;
    return old;
}

void TCameraObject::SetProjType(PROJTYPE type)                          // 0xf280
{
    m_eProjType = type;
}

TCameraObject::FOVMODE TCameraObject::SetFOVMode(FOVMODE mode)          // 0xf290
{
    FOVMODE old = m_eFOVMode;
    m_eFOVMode = mode;
    return old;
}

float TCameraObject::SetFar(float farClip)                              // 0xf2b0
{
    float old = m_fFar;
    m_fFar = farClip;
    return old;
}

float TCameraObject::SetNear(float nearClip)                            // 0xf2d0
{
    float old = m_fNear;
    m_fNear = nearClip;
    return old;
}

void TCameraObject::OnDestroy()                                         // 0xf2f0
{
}

TCameraObject::~TCameraObject()                                         // 0xf300
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TCameraObject)                    // 0xf340-0xf3b0

// 0xf3c0-0xf4b2 (OnUpdateEvent, GetCamera, IsEnabled, GetProjType, the handler's destructor and
// copy constructor) are inline: the compiler emits them at the end of this file.

} // namespace Toshi
