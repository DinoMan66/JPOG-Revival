// TRenderInterface.dll - TRenderContext.cpp (0x9c90-0xa9a0 in the original).
#define TRENDERINTERFACE_EXPORTS
#include <math.h>
#include "TNewDelete.h"
#include "TRenderContext.h"
#include "TRenderInterface.h"
#include "TMath.h"

namespace Toshi {

TRenderContext::TRenderContext(TRenderInterface& renderer)              // 0x9c90
    : m_pRenderer(&renderer)
    , m_eFlags(FLAG_DIRTY)
    , m_iClipFlags(0x3F)
    , m_iGuardFlags(0)
    , m_eProjType(PROJTYPE_PERSPECTIVE)
    , m_pSkeletonInstance(0)
    , m_oAmbientColour(1.0f, 1.0f, 1.0f, 0.0f)
    , m_oFogColour(1.0f, 1.0f, 1.0f, 0.0f)
    , m_fFogDistanceStart(10.0f)
    , m_fFogDistanceEnd(1000.0f)
    , m_oSun(1.28f, 1.0f)
    , m_oAtmosphere(2.0f, 1.7f, 2.25f, 0.5f, -0.2f)
{
    double fNear;
    if (renderer.GetCurrentDevice() != 0)
    {
        unsigned int width = renderer.GetCurrentDevice()->GetMode()->GetWidth();
        unsigned int height = renderer.GetCurrentDevice()->GetMode()->GetHeight();
        m_oViewportParams.fX = 0.0f;
        m_oViewportParams.fY = 0.0f;
        m_oViewportParams.fWidth = (float)width;
        m_oViewportParams.fHeight = (float)height;
        m_oViewportParams.fMinZ = 0.0f;
        fNear = 1.0f;
        m_oViewportParams.fMaxZ = 1.0f;
    }
    else
    {
        m_oViewportParams.fX = 0.0f;
        m_oViewportParams.fY = 0.0f;
        m_oViewportParams.fWidth = 640.0f;
        m_oViewportParams.fHeight = 480.0f;
        m_oViewportParams.fMinZ = 0.0f;
        fNear = 1.0f;
        m_oViewportParams.fMaxZ = 1.0f;
    }

    // projection from a 90 degree field of view, near 1, far 1000
    float fFOV = TMath::PI / 180.0f * 90.0f;
    double fWidth = m_oViewportParams.fWidth;
    double fHeight = m_oViewportParams.fHeight;
    double fHalfHeight = fHeight * 0.5f;
    double fProj = fHalfHeight * (fNear / tan(fFOV * 0.5f));
    m_oProjParams.m_Centre.x = (float)(fWidth * 0.5f);
    m_oProjParams.m_Centre.y = (float)fHalfHeight;
    m_oProjParams.m_Proj.x = (float)fProj;
    m_oProjParams.m_Proj.y = (float)fProj;
    m_oProjParams.m_fNearClip = (float)fNear;
    m_oProjParams.m_fFarClip = 1000.0f;

    m_oModelViewMatrix.Identity();
    m_oWorldViewMatrix.Identity();
    m_eFlags |= FLAG_DIRTY_WORLDMODELMATRIX | FLAG_DIRTY_VIEWMODELMATRIX;
    m_oAmbientColour.Set(0.0f, 0.0f, 0.0f, 0.0f);
}

void TRenderContext::SetModelViewMatrix(const TMatrix44& matrix)        // 0x9f90
{
    m_eFlags |= FLAG_DIRTY_WORLDMODELMATRIX | FLAG_DIRTY_VIEWMODELMATRIX;
    m_oModelViewMatrix = matrix;
    m_eFlags &= ~(FLAG_HAS_MODELWORLDMATRIX | FLAG_UNK10);
}

void TRenderContext::SetWorldViewMatrix(const TMatrix44& matrix)        // 0xa020
{
    m_eFlags |= FLAG_DIRTY_WORLDMODELMATRIX;
    m_oWorldViewMatrix = matrix;
    m_eFlags &= ~(FLAG_HAS_MODELWORLDMATRIX | FLAG_HAS_VIEWWORLDMATRIX | FLAG_UNK20 | FLAG_HAS_WORLDPLANES | FLAG_HAS_WORLDGUARDPLANES);
}

const TMatrix44& TRenderContext::GetModelWorldMatrix()                  // 0xa0d0
{
    if (!(m_eFlags & FLAG_HAS_MODELWORLDMATRIX))
    {
        m_oModelWorldMatrix.Multiply(GetViewWorldMatrix(), m_oModelViewMatrix);
        m_eFlags |= FLAG_HAS_MODELWORLDMATRIX;
    }
    return m_oModelWorldMatrix;
}

const TMatrix44& TRenderContext::GetViewWorldMatrix()                   // 0xa110
{
    if (!(m_eFlags & FLAG_HAS_VIEWWORLDMATRIX))
    {
        m_oViewWorldMatrix.InvertOrthogonal(m_oWorldViewMatrix);
        m_eFlags |= FLAG_HAS_VIEWWORLDMATRIX;
    }
    return m_oViewWorldMatrix;
}

const TPlane* TRenderContext::GetWorldPlanes()                          // 0xa150
{
    if (!(m_eFlags & FLAG_HAS_WORLDPLANES))
    {
        const TMatrix44& viewWorld = GetViewWorldMatrix();
#if TRC_WP == 1
        TPlane* out = m_aWorldPlanes;
        const TPlane* in = m_aViewPlanes;
        for (int i = 0; i < 6; i++)
        {
            TMatrix44::TransformPlaneOrthogonal(*out, viewWorld, *in);
            out++;
            in++;
        }
#elif TRC_WP == 2
        for (unsigned int i = 0; i < 6; i++)
            TMatrix44::TransformPlaneOrthogonal(m_aWorldPlanes[i], viewWorld, m_aViewPlanes[i]);
#elif TRC_WP == 3
        for (const TPlane* in = m_aViewPlanes; in < m_aViewPlanes + 6; in++)
            TMatrix44::TransformPlaneOrthogonal(m_aWorldPlanes[in - m_aViewPlanes], viewWorld, *in);
#else
        for (int i = 0; i < 6; i++)
            TMatrix44::TransformPlaneOrthogonal(m_aWorldPlanes[i], viewWorld, m_aViewPlanes[i]);
#endif
        m_eFlags |= FLAG_HAS_WORLDPLANES;
    }
    return m_aWorldPlanes;
}

const TPlane* TRenderContext::GetWorldGuardPlanes()                     // 0xa1d0
{
    if (!(m_eFlags & FLAG_HAS_WORLDGUARDPLANES))
    {
        const TMatrix44& viewWorld = GetViewWorldMatrix();
        for (int i = 0; i < 6; i++)
            TMatrix44::TransformPlaneOrthogonal(m_aWorldGuardPlanes[i], viewWorld, m_aGuardPlanes[i]);
        m_eFlags |= FLAG_HAS_WORLDGUARDPLANES;
    }
    return m_aWorldGuardPlanes;
}

void TRenderContext::GetViewPlanes(TPlane* planes, int count) const     // 0xa250
{
    for (int i = 0; i < count; i++)
        planes[i] = m_aViewPlanes[i];
}

void TRenderContext::GetGuardPlanes(TPlane* planes, int count) const    // 0xa360
{
    for (int i = 0; i < count; i++)
        planes[i] = m_aGuardPlanes[i];
}

void TRenderContext::GetWorldPlanes(TPlane* planes, int count)          // 0xa470
{
    const TPlane* worldPlanes = GetWorldPlanes();
    for (int i = 0; i < count; i++)
        planes[i] = worldPlanes[i];
}

void TRenderContext::GetWorldGuardPlanes(TPlane* planes, int count)     // 0xa550
{
    const TPlane* worldPlanes = GetWorldGuardPlanes();
    for (int i = 0; i < count; i++)
        planes[i] = worldPlanes[i];
}

const TMatrix44& TRenderContext::GetViewModelMatrix()                   // 0xa630
{
    if (m_eFlags & FLAG_DIRTY_VIEWMODELMATRIX)
    {
        m_oViewModelMatrix.Invert(m_oModelViewMatrix);
        m_eFlags &= ~FLAG_DIRTY_VIEWMODELMATRIX;
    }
    return m_oViewModelMatrix;
}

const TMatrix44& TRenderContext::GetWorldModelMatrix()                  // 0xa670
{
    if (m_eFlags & FLAG_DIRTY_WORLDMODELMATRIX)
    {
        m_oWorldModelMatrix.Invert(GetModelWorldMatrix());
        m_eFlags &= ~FLAG_DIRTY_WORLDMODELMATRIX;
    }
    return m_oWorldModelMatrix;
}

int __stdcall TRenderContext::CullSphereToFrustum(const TSphere* sphere, const TPlane* planes, int clipFlags, int clipFlagsMask) // 0xa6b0
{
    int leftPlanes = clipFlags & clipFlagsMask;
    int planeFlag = 1;
    while (leftPlanes != 0)
    {
        if (leftPlanes & planeFlag)
        {
            float dist = TVector4::DotProduct3(sphere->AsVector4(), planes->AsVector4()) - planes->GetD();
            if (sphere->GetRadius() < dist)
                return -1;
            if (dist < -sphere->GetRadius())
                clipFlags &= ~planeFlag;
            leftPlanes &= ~planeFlag;
        }
        planeFlag <<= 1;
        planes++;
    }
    return clipFlags;
}

int __stdcall TRenderContext::CullAABoxToFrustum(const TAABox* box, const TPlane* planes, int clipFlags, int clipFlagsMask) // 0xa740
{
    int leftPlanes = clipFlags & clipFlagsMask;
    int planeFlag = 1;
    while (leftPlanes != 0)
    {
        if (leftPlanes & planeFlag)
        {
            TVector3 absNormal;
            absNormal.Abs(planes->AsNormal());
            double dist = TVector3::DotProduct(*(const TVector3*)box, planes->AsNormal()) - planes->GetD();
            float extent = TVector3::DotProduct(*((const TVector3*)box + 1), absNormal);
            if (dist > extent)
                return -1;
            if (dist < -extent)
                clipFlags &= ~planeFlag;
            leftPlanes &= ~planeFlag;
        }
        planeFlag <<= 1;
        planes++;
    }
    return clipFlags;
}

bool __stdcall TRenderContext::CullSphereToFrustumSimple(const TSphere& sphere, const TPlane* planes, int count) // 0xa7f0
{
    for (int i = 0; i < 6; i++)
    {
#ifdef TRC_CS_B
        if (sphere.GetRadius() < TVector4::DotProduct3(sphere.AsVector4(), planes->AsVector4()) - planes->GetD())
            return false;
#else
        float dist = TVector4::DotProduct3(sphere.AsVector4(), planes->AsVector4()) - planes->GetD();
        if (dist > sphere.GetRadius())
            return false;
#endif
        planes++;
    }
    return true;
}

bool TRenderContext::IsDirty()                                          // 0xa840
{
    return (m_eFlags & FLAG_DIRTY) != 0;
}

void TRenderContext::SetDirty(bool dirty)                               // 0xa850
{
    if (dirty)
    {
        m_eFlags |= FLAG_DIRTY;
        m_eFlags &= ~(FLAG_UNK10 | FLAG_UNK20 | FLAG_HAS_WORLDPLANES | FLAG_HAS_WORLDGUARDPLANES);
    }
    else
        m_eFlags &= ~FLAG_DIRTY;
}

void TRenderContext::EnableFog(bool enable)                             // 0xa880
{
    if (enable)
        m_eFlags |= FLAG_FOG;
    else
        m_eFlags &= ~FLAG_FOG;
}

void TRenderContext::SetFogDistance(float start, float end)             // 0xa8a0
{
    m_fFogDistanceStart = start;
    m_fFogDistanceEnd = end;
    SetDirty(true);
}

void TRenderContext::SetProjType(PROJTYPE type)                         // 0xa8d0
{
    m_eProjType = type;
    SetDirty(true);
}

void TRenderContext::SetProjectionParameters(const PROJECTIONPARAMS& params) // 0xa8f0
{
    m_oProjParams = params;
    SetDirty(true);
}

void TRenderContext::SetViewportParameters(const VIEWPORTPARAMS& params) // 0xa930
{
    m_oViewportParams = params;
    SetDirty(true);
}

TRenderContext::~TRenderContext()                                       // 0xa970
{
}

} // namespace Toshi
