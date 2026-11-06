// TRenderInterface.dll - TGFXFileInstanceObject.cpp (0xf7d0-0xff00 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TGFXFileInstanceObject.h"
#include "TGFXFileObject.h"
#include "TTransformObject.h"
#include "TCameraObject.h"
#include "TScene.h"
#include "TModel.h"
#include "TRenderInterface.h"
#include "TRenderContext.h"
#include "TMath.h"

namespace Toshi {

// System resource 9 (a platform shader in TRenderD3DInterface.dll): only what this file uses.
class TGFXInstanceShaderI : public TShader
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
    virtual int SetTextureStages(int s0, int s1, int s2, int s3) = 0;   // slot 36
    virtual int SetTextureAlphas(int a0, int a1, int a2, int a3) = 0;   // slot 37
    virtual void RestoreTextureAlphas(int alphas) = 0;                  // slot 38

    int   TextureStagesState() { return *(int*)((char*)this + 0x30); }
    void  SetTextureStagesState(int state) { *(int*)((char*)this + 0x30) = state; }
    float& Alpha() { return *(float*)((char*)this + 0x38); }
};

bool TGFXFileInstanceObject::sm_bForceLowLOD;                           // 0x1002c9c0

TGFXFileInstanceObject::TGFXFileInstanceObject()                        // 0xf850
{
    m_pGFXFile = 0;
    m_pModelInstance = 0;
    m_pTransformObject = 0;
    m_uiState = STATE_VISIBLE | STATE_DISTANCEALPHAFADE;
}

bool TGFXFileInstanceObject::Create(TGFXFileObject* gfxFile)            // 0xf880
{
    if (IsCreated() == true)
        return true;

    m_pGFXFile = gfxFile;
    m_pTransformObject = (TTransformObject*)GetRenderer()->CreateResource(&TTransformObject::m_sClass, 0, Parent());
    m_pTransformObject->Create();
    SetParent(m_pTransformObject);
    m_pModelInstance = m_pGFXFile->GetModel()->CreateInstance();
    m_acTextureStages[0] = 0;
    m_acTextureStages[1] = -1;
    m_acTextureStages[2] = -1;
    m_acTextureStages[3] = -1;
    m_aucTextureAlphas[0] = 0x80;
    m_aucTextureAlphas[1] = 0;
    m_aucTextureAlphas[2] = 0;
    m_aucTextureAlphas[3] = 0;
    m_fFadeStart = 0.0f;
    m_fFadeEnd = 0.0f;
    return TResource::Create();
}

bool TGFXFileInstanceObject::Render(TScene& scene)                      // 0xf940
{
    TRenderInterface* renderer = TRenderInterface::GetRenderer();
    TRenderContext* context = renderer->GetCurrentRenderContext();
    TGFXInstanceShaderI* shader = (TGFXInstanceShaderI*)renderer->GetSystemResource((TRenderInterface::SYSRESOURCES)9);

    float radius = m_pModelInstance->m_pModel->m_fCoarseBoundingRadius;
    TSphere sphere(m_pTransformObject->GetTranslate(), radius);
    int clipFlags = context->GetClipFlags();

    if (!IsVisibile())
        return false;
    if (radius > 0.0f && !TRenderContext::CullSphereToFrustumSimple(sphere, context->GetWorldPlanes(), 6))
        return false;

    TCameraObject* camera = scene.GetCurrentCamera();
    TVector3 toCamera;
    toCamera.Subtract(sphere.GetOrigin(), camera->GetTransformObject()->GetTranslate());
    float distance = toCamera.Magnitude();

    if (m_fFadeStart == m_fFadeEnd)
    {
        m_fFadeStart = camera->GetNear();
        m_fFadeEnd = camera->GetFar();
    }

    int lod = TMath::TruncToInt((TMath::Min(m_fFadeEnd, distance) - camera->GetNear()) *
                                m_pModelInstance->m_pModel->m_iLODCount / (m_fFadeEnd - camera->GetNear()));
    if (lod >= m_pModelInstance->m_pModel->m_iLODCount)
        lod = m_pModelInstance->m_pModel->m_iLODCount - 1;
    m_pModelInstance->m_iLOD = lod;

    float alpha;
    if (GetDistanceAlphaFadeState() == true)
    {
        if (m_fFadeStart > distance)
            alpha = 1.0f;
        else if (m_fFadeEnd > m_fFadeStart)
        {
            if (m_fFadeEnd < distance)
                return false;
            alpha = (m_fFadeEnd - distance) / (m_fFadeEnd - m_fFadeStart);
        }
        else
            alpha = 1.0f;
    }
    else
        alpha = 1.0f;

    float oldAlpha = shader->Alpha();
    shader->Alpha() = alpha;
    int oldStages = shader->SetTextureStages(m_acTextureStages[0], m_acTextureStages[1],
                                             m_acTextureStages[2], m_acTextureStages[3]);
    int oldAlphas = shader->SetTextureAlphas(m_aucTextureAlphas[0], m_aucTextureAlphas[1],
                                             m_aucTextureAlphas[2], m_aucTextureAlphas[3]);
    m_pTransformObject->Push(scene);
    context->SetClipFlags(clipFlags);
    m_pModelInstance->Render();
    context->SetClipFlags(clipFlags);
    m_pTransformObject->Pop(scene);
    shader->Alpha() = oldAlpha;
    shader->SetTextureStagesState(oldStages);
    shader->RestoreTextureAlphas(oldAlphas);
    return true;
}

void TGFXFileInstanceObject::SetTextureStagesA(signed char* stages)     // 0xfbe0
{
    m_acTextureStages[0] = stages[0];
    m_acTextureStages[1] = stages[1];
    m_acTextureStages[2] = stages[2];
    m_acTextureStages[3] = stages[3];
}

void TGFXFileInstanceObject::GetTextureStages(signed char* stages)      // 0xfc00
{
    stages[0] = m_acTextureStages[0];
    stages[1] = m_acTextureStages[1];
    stages[2] = m_acTextureStages[2];
    stages[3] = m_acTextureStages[3];
}

void TGFXFileInstanceObject::SetTextureAlphasA(unsigned char* alphas)   // 0xfc20
{
    m_aucTextureAlphas[0] = alphas[0];
    m_aucTextureAlphas[1] = alphas[1];
    m_aucTextureAlphas[2] = alphas[2];
    m_aucTextureAlphas[3] = alphas[3];
}

void TGFXFileInstanceObject::GetTextureAlphas(unsigned char* alphas)    // 0xfc40
{
    alphas[0] = m_aucTextureAlphas[0];
    alphas[1] = m_aucTextureAlphas[1];
    alphas[2] = m_aucTextureAlphas[2];
    alphas[3] = m_aucTextureAlphas[3];
}

TOSHI_DEFINE_CLASS(TGFXFileInstanceObject, TSceneObject)                // 0xfc60

void TGFXFileInstanceObject::SetTextureAlphas(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3) // 0xfcc0
{
    m_aucTextureAlphas[0] = a0;
    m_aucTextureAlphas[1] = a1;
    m_aucTextureAlphas[2] = a2;
    m_aucTextureAlphas[3] = a3;
}

void TGFXFileInstanceObject::SetTextureStages(signed char s0, signed char s1, signed char s2, signed char s3) // 0xfce0
{
    m_acTextureStages[0] = s0;
    m_acTextureStages[1] = s1;
    m_acTextureStages[2] = s2;
    m_acTextureStages[3] = s3;
}

void TGFXFileInstanceObject::DestroyResource()                          // 0xfd00
{
    m_pTransformObject->DestroyResource();
    TResource::DestroyResource();
}

bool TGFXFileInstanceObject::Update(TScene& scene)                      // 0xfd20
{
    m_pModelInstance->Update(scene.GetDeltaTime());
    return true;
}

void TGFXFileInstanceObject::OnDestroy()                                // 0xfd40
{
    if (m_pModelInstance)
    {
        m_pModelInstance->Delete();
        m_pModelInstance = 0;
    }
}

TGFXFileInstanceObject::~TGFXFileInstanceObject()                       // 0xfd60
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TGFXFileInstanceObject)           // 0xfd70-0xfde0

} // namespace Toshi
