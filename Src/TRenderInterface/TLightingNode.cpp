// TRenderInterface.dll - TLightingNode.cpp (0x10310-0x108a0 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TLightingNode.h"
#include "TRenderInterface.h"
#include "TRenderContext.h"

namespace Toshi {

TLightingNode::TLightingNode()                                          // 0x10390
{
    m_oAmbientColour.Set(0.0f, 0.0f, 0.0f, 1.0f);
}

bool TLightingNode::Render(TScene& scene)                               // 0x103d0
{
#ifndef TLN_EARLY
    if (m_uiFlags & FLAG_RENDER)
    {
        TRenderInterface* renderer = GetRenderer();
        TRenderContext* context = GetRenderer()->GetCurrentRenderContext();

        TMatrix44 oldLightDirection = renderer->GetLightDirectionMatrix();
        TMatrix44 oldLightColour = renderer->GetLightColourMatrix();
        TVector4 oldAmbientColour = context->GetAmbientColour();

        renderer->SetLightDirectionMatrix(m_oLightDirection);
        renderer->SetLightColourMatrix(m_oLightColour);
        context->SetAmbientColour(m_oAmbientColour);

        bool result = TSceneObject::Render(scene);

        renderer->SetLightDirectionMatrix(oldLightDirection);
        renderer->SetLightColourMatrix(oldLightColour);
        context->SetAmbientColour(oldAmbientColour);
        return result;
    }
    return true;
#else
    if (!(m_uiFlags & FLAG_RENDER))
        return true;

    TRenderInterface* renderer = GetRenderer();
    TRenderContext* context = GetRenderer()->GetCurrentRenderContext();

    TMatrix44 oldLightDirection = renderer->GetLightDirectionMatrix();
    TMatrix44 oldLightColour = renderer->GetLightColourMatrix();
    TVector4 oldAmbientColour = context->GetAmbientColour();

    renderer->SetLightDirectionMatrix(m_oLightDirection);
    renderer->SetLightColourMatrix(m_oLightColour);
    context->SetAmbientColour(m_oAmbientColour);

    bool result = TSceneObject::Render(scene);

    renderer->SetLightDirectionMatrix(oldLightDirection);
    renderer->SetLightColourMatrix(oldLightColour);
    context->SetAmbientColour(oldAmbientColour);
    return result;
#endif
}

TOSHI_DEFINE_CLASS(TLightingNode, TSceneObject)                         // 0x10620

TLightingNode::~TLightingNode()                                         // 0x10680
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TLightingNode)                    // 0x10690-0x10700

} // namespace Toshi
