// TRenderInterface.dll - TTerrainObject.cpp (0x112b0-0x118f0 in the original)
#define TRENDERINTERFACE_EXPORTS
#include <math.h>
#include "TNewDelete.h"
#include "TTerrainObject.h"
#include "TScene.h"
#include "TCameraObject.h"
#include "TMath.h"

namespace Toshi {

// Builds the six view-frustum planes (near, far, left, right, top, bottom) from the camera position,
// its up and forward vectors and the vertical half field of view (4:3 aspect).
static void BuildFrustumPlanes(TPlane* planes, const TVector3& pos, float nearClip, float farClip,
                               float halfFOV, const TVector3& up, const TVector3& forward) // 0x112b0
{
    float halfFOVH = TMath::ATan(tan(halfFOV) * 1.3333334f);
    TVector3 right;
    right.CrossProduct(up, forward);
    TVector3 back;
    back.Negate(forward);

    TVector3 nearPoint = pos + forward * nearClip;
    planes[0].SetNormal(forward);
    planes[0].SetD(TVector3::DotProduct(forward, nearPoint));

    TVector3 farPoint = pos + forward * farClip;
    planes[1].SetNormal(back);
    planes[1].SetD(TVector3::DotProduct(back, farPoint));

    TVector3 normal;
    normal.CrossProduct(forward * cos(halfFOVH) + right * sin(halfFOVH), up);
    planes[2].SetNormal(normal);
    planes[2].SetD(TVector3::DotProduct(normal, pos));

    normal.CrossProduct(right * sin(halfFOVH) - forward * cos(halfFOVH), up);
    planes[3].SetNormal(normal);
    planes[3].SetD(TVector3::DotProduct(normal, pos));

    normal.CrossProduct(right, forward * cos(halfFOV) + up * sin(halfFOV));
    planes[4].SetNormal(normal);
    planes[4].SetD(TVector3::DotProduct(normal, pos));

    normal.CrossProduct(right, up * sin(halfFOV) - forward * cos(halfFOV));
    planes[5].SetNormal(normal);
    planes[5].SetD(TVector3::DotProduct(normal, pos));
}

bool TTerrainObject::Render(TScene& scene)                              // 0x116a0
{
    if (m_pTerrain)
    {
        TMatrix44 view;
        view.InvertOrthogonal(scene.GetRenderMatrixStack().Top());
#ifdef TTO_ASSIGN
        TVector3 pos;
        pos = view.GetTranslation3();
        TVector3 up;
        up.Negate(*(const TVector3*)&view.m_f21);
        TVector3 forward;
        forward = *(const TVector3*)&view.m_f31;
#else
        TVector3 pos = view.GetTranslation3();
        TVector3 up;
        up.Negate(*(const TVector3*)&view.m_f21);
        TVector3 forward = *(const TVector3*)&view.m_f31;
#endif
        TCameraObject* camera = scene.GetCurrentCamera();
        TPlane planes[6];
        BuildFrustumPlanes(planes, pos, camera->GetNear(), camera->GetFar(), camera->GetFOV() * 0.5f, up, forward);
    }
    return TSceneObject::Render(scene);
}

TOSHI_DEFINE_CLASS(TTerrainObject, TSceneObject)                        // 0x117a0

bool TTerrainObject::SetTerrain(TTerrainInterface* terrain)            // 0x11800
{
    m_pTerrain = terrain;
    return true;
}

bool TTerrainObject::Create(TTerrainInterface* terrain)                // 0x11810
{
    SetTerrain(terrain);
    return TResource::Create();
}

TTerrainObject::~TTerrainObject()                                       // 0x11830
{
}

TTerrainObject::TTerrainObject()                                        // 0x11840
{
    m_pTerrain = 0;
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TTerrainObject)                   // 0x11860-0x118d0

} // namespace Toshi
