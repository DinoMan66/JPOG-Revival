// TRenderInterface.dll - TViewport.cpp (0x12190-0x12bd0 in the original). The function order
// follows the original: the getters come last, so the earlier functions call them out of line.
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TViewport.h"
#include "TScene.h"
#include "TRenderInterface.h"
#include "TRenderContext.h"

namespace Toshi {

TViewport::TViewport()                                                  // 0x12190
    : m_pRenderContext(0),
      m_oBeginUpdateEmitter(this), m_oUpdateEmitter(this), m_oEndUpdateEmitter(this),
      m_oBeginRenderEmitter(this), m_oRenderEmitter(this), m_oEndRenderEmitter(this),
      m_oDestroyEmitter(this), m_oChangeEmitter(this)
{
    m_aBackgroundColour[0] = 0;
    m_aBackgroundColour[1] = 0;
    m_aBackgroundColour[2] = 0;
    m_aBackgroundColour[3] = 0;
    m_bAllowBackgroundClear = true;
    m_bAllowDepthClear = true;
    m_bEnableDefaultBeginRender = true;
    m_bEnabled = true;
}

TViewport::~TViewport()                                                 // 0x12250
{
    DeleteHandlers();
    if (m_pRenderContext)
    {
        delete m_pRenderContext;
        m_pRenderContext = 0;
    }
}

bool TViewport::Create(bool connectDefaultHandlers)                     // 0x12340
{
    m_pRenderContext = GetRenderer()->CreateRenderContext();
    SetWidth(-1.0f);
    SetHeight(-1.0f);
    if (connectDefaultHandlers == true)
        GetRenderer()->ConnectDefaultViewportHandlers(*this);
    return TResource::Create();
}

bool TViewport::Update(TScene& scene)                                   // 0x123a0
{
    if (m_bEnabled == true)
    {
        if (GetRenderer())
        {
            if (GetWidth() < 0.0f)
                SetWidth((float)GetRenderer()->GetCurrentDevice()->GetMode()->GetWidth());
            if (GetHeight() < 0.0f)
                SetHeight((float)GetRenderer()->GetCurrentDevice()->GetMode()->GetHeight());
        }
        BeginUpdateEvent beginUpdate;
        m_oBeginUpdateEmitter.Throw(beginUpdate);
        UpdateEvent update;
        m_oUpdateEmitter.Throw(update);
        EndUpdateEvent endUpdate;
        m_oEndUpdateEmitter.Throw(endUpdate);
    }
    return TSceneObject::Update(scene);
}

bool TViewport::Render(TScene& scene)                                   // 0x124c0
{
    if (m_bEnabled == true)
    {
        if (GetRenderer())
        {
            if (GetWidth() < 0.0f)
                SetWidth((float)GetRenderer()->GetCurrentDevice()->GetMode()->GetWidth());
            if (GetHeight() < 0.0f)
                SetHeight((float)GetRenderer()->GetCurrentDevice()->GetMode()->GetHeight());
        }
        TRenderContext* oldContext = GetRenderer()->GetCurrentRenderContext();
        GetRenderer()->SetCurrentRenderContext(m_pRenderContext);
        m_pRenderContext->SetAmbientColour(scene.GetAmbientColour());
        BeginRenderEvent beginRender;
        m_oBeginRenderEmitter.Throw(beginRender);
        RenderEvent render(scene);
        m_oRenderEmitter.Throw(render);
        EndRenderEvent endRender;
        m_oEndRenderEmitter.Throw(endRender);
        GetRenderer()->SetCurrentRenderContext(oldContext);
    }
    return TSceneObject::Render(scene);
}

float TViewport::SetX(float value)                                       // 0x12640
{
    TRenderContext::VIEWPORTPARAMS params = m_pRenderContext->GetViewportParameters();
    float old = params.fX;
    params.fX = value;
    m_pRenderContext->SetViewportParameters(params);
    TNullEvent event;
    m_oChangeEmitter.Throw(event);
    return old;
}

float TViewport::SetY(float value)                                       // 0x126b0
{
    TRenderContext::VIEWPORTPARAMS params = m_pRenderContext->GetViewportParameters();
    float old = params.fY;
    params.fY = value;
    m_pRenderContext->SetViewportParameters(params);
    TNullEvent event;
    m_oChangeEmitter.Throw(event);
    return old;
}

float TViewport::SetWidth(float value)                                   // 0x12720
{
    TRenderContext::VIEWPORTPARAMS params = m_pRenderContext->GetViewportParameters();
    float old = params.fWidth;
    params.fWidth = value;
    m_pRenderContext->SetViewportParameters(params);
    TNullEvent event;
    m_oChangeEmitter.Throw(event);
    return old;
}

float TViewport::SetHeight(float value)                                  // 0x12790
{
    TRenderContext::VIEWPORTPARAMS params = m_pRenderContext->GetViewportParameters();
    float old = params.fHeight;
    params.fHeight = value;
    m_pRenderContext->SetViewportParameters(params);
    TNullEvent event;
    m_oChangeEmitter.Throw(event);
    return old;
}

float TViewport::SetMinZ(float value)                                    // 0x12800
{
    TRenderContext::VIEWPORTPARAMS params = m_pRenderContext->GetViewportParameters();
    float old = params.fMinZ;
    params.fMinZ = value;
    m_pRenderContext->SetViewportParameters(params);
    TNullEvent event;
    m_oChangeEmitter.Throw(event);
    return old;
}

float TViewport::SetMaxZ(float value)                                    // 0x12870
{
    TRenderContext::VIEWPORTPARAMS params = m_pRenderContext->GetViewportParameters();
    float old = params.fMaxZ;
    params.fMaxZ = value;
    m_pRenderContext->SetViewportParameters(params);
    TNullEvent event;
    m_oChangeEmitter.Throw(event);
    return old;
}

void TViewport::SetBackgroundColour(unsigned char r, unsigned char g, unsigned char b, unsigned char a) // 0x128e0
{
    m_aBackgroundColour[0] = r;
    m_aBackgroundColour[1] = g;
    m_aBackgroundColour[2] = b;
    m_aBackgroundColour[3] = a;
    TNullEvent event;
    m_oChangeEmitter.Throw(event);
}

TOSHI_DEFINE_CLASS(TViewport, TSceneObject)                            // 0x12940

bool TViewport::EnableDefaultBeginRender(bool value)
{
    bool old = m_bEnableDefaultBeginRender;
    m_bEnableDefaultBeginRender = value;
    return old;
}

bool TViewport::AllowDepthClear(bool value)
{
    bool old = m_bAllowDepthClear;
    m_bAllowDepthClear = value;
    return old;
}

bool TViewport::AllowBackgroundClear(bool value)
{
    bool old = m_bAllowBackgroundClear;
    m_bAllowBackgroundClear = value;
    return old;
}

void TViewport::DeleteHandlers()                                        // 0x129f0
{
    m_oHandlers.DeleteAll();
}

void TViewport::AddHandler(THandler& handler)                           // 0x12a70
{
    m_oHandlers.InsertTail(&handler);
}

void TViewport::GetBackgroundColour(unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a) // 0x12aa0
{
    r = m_aBackgroundColour[0];
    g = m_aBackgroundColour[1];
    b = m_aBackgroundColour[2];
    a = m_aBackgroundColour[3];
}

float TViewport::GetMaxZ() { return m_pRenderContext->GetViewportParameters().fMaxZ; }       // 0x12ae0
float TViewport::GetMinZ() { return m_pRenderContext->GetViewportParameters().fMinZ; }       // 0x12af0
float TViewport::GetHeight() { return m_pRenderContext->GetViewportParameters().fHeight; }   // 0x12b00
float TViewport::GetWidth() { return m_pRenderContext->GetViewportParameters().fWidth; }     // 0x12b10
float TViewport::GetY() { return m_pRenderContext->GetViewportParameters().fY; }             // 0x12b20
float TViewport::GetX() { return m_pRenderContext->GetViewportParameters().fX; }             // 0x12b30

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TViewport)                        // 0x12b40-0x12bb0

} // namespace Toshi
