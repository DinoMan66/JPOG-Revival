// Toshi::TRenderContext (TRenderInterface.dll, TRenderContext.cpp) - per-viewport render state. Size 0x484.
// Header-defined members sit in the original's inline/COMDAT region (0x73b0-0x7650). Structure follows
// OpenToshi's TRenderContext; layout and flag values are JPOG's (constructor at 0x9c90).
#ifndef TRENDERCONTEXT_H
#define TRENDERCONTEXT_H
#include "TRenderAPI.h"
#include "TVector.h"
#include "TEnvironment.h"

namespace Toshi {

class TRenderInterface;
class TSkeletonInstance;

class TRENDER_API TRenderContext
{
public:
    enum FLAG
    {
        FLAG_DIRTY                  = 0x001,
        FLAG_FOG                    = 0x002,
        FLAG_HAS_MODELWORLDMATRIX   = 0x004,
        FLAG_HAS_VIEWWORLDMATRIX    = 0x008,
        FLAG_UNK10                  = 0x010,
        FLAG_UNK20                  = 0x020,
        FLAG_HAS_WORLDPLANES        = 0x040,
        FLAG_HAS_WORLDGUARDPLANES   = 0x080,
        FLAG_DIRTY_WORLDMODELMATRIX = 0x100,
        FLAG_DIRTY_VIEWMODELMATRIX  = 0x200
    };
    enum PROJTYPE { PROJTYPE_PERSPECTIVE, PROJTYPE_ORTHOGRAPHIC };

    struct VIEWPORTPARAMS
    {
        float fX, fY, fWidth, fHeight, fMinZ, fMaxZ;
    };
    struct PROJECTIONPARAMS
    {
        TVector2 m_Centre;
        TVector2 m_Proj;
        float    m_fNearClip;
        float    m_fFarClip;
    };

    TRenderContext(TRenderInterface& renderer);
    virtual ~TRenderContext();                                          // slot 0
    virtual void SetModelViewMatrix(const TMatrix44& matrix);           // slot 1
    virtual void SetWorldViewMatrix(const TMatrix44& matrix);           // slot 2
    virtual void Update() = 0;                                          // slot 3 (pure; name unknown)

    const TMatrix44& GetModelWorldMatrix();
    const TMatrix44& GetViewWorldMatrix();
    const TMatrix44& GetViewModelMatrix();
    const TMatrix44& GetWorldModelMatrix();
    const TPlane* GetWorldPlanes();
    const TPlane* GetWorldGuardPlanes();
    void GetViewPlanes(TPlane* planes, int count) const;
    void GetGuardPlanes(TPlane* planes, int count) const;
    void GetWorldPlanes(TPlane* planes, int count);
    void GetWorldGuardPlanes(TPlane* planes, int count);
    static int __stdcall CullSphereToFrustum(const TSphere* sphere, const TPlane* planes, int clipFlags, int clipFlagsMask);
    static int __stdcall CullAABoxToFrustum(const TAABox* box, const TPlane* planes, int clipFlags, int clipFlagsMask);
    static bool __stdcall CullSphereToFrustumSimple(const TSphere& sphere, const TPlane* planes, int count);
    void EnableFog(bool enable);
    void SetFogDistance(float start, float end);
    void SetProjType(PROJTYPE type);
    void SetProjectionParameters(const PROJECTIONPARAMS& params);
    void SetViewportParameters(const VIEWPORTPARAMS& params);

    TSun& GetSun() { return m_oSun; }
    TAtmosphere& GetAtmosphere() { return m_oAtmosphere; }
    const TPlane* GetGuardPlanes() const { return m_aGuardPlanes; }
    const TPlane* GetViewPlanes() const { return m_aViewPlanes; }
    bool IsFogEnabled() const { return (m_eFlags & FLAG_FOG) != 0; }
    void GetFogDistance(float& start, float& end) const
    {
        start = m_fFogDistanceStart;
        end = m_fFogDistanceEnd;
    }
    void SetFogColour(float r, float g, float b) { m_oFogColour.Set(r, g, b, 0.0f); }
    void SetFogColour(const TVector4& colour) { m_oFogColour = colour; }
    void GetFogColour(TVector4& colour) const { colour = m_oFogColour; }
    const TVector4& GetFogColour() const { return m_oFogColour; }
    void SetAmbientColour(float r, float g, float b) { m_oAmbientColour.Set(r, g, b, 0.0f); }
    void SetAmbientColour(const TVector4& colour) { m_oAmbientColour = colour; }
    void GetAmbientColour(TVector4& colour) const { colour = m_oAmbientColour; }
    const TVector4& GetAmbientColour() const { return m_oAmbientColour; }
    TSkeletonInstance* GetSkeletonInstance() { return m_pSkeletonInstance; }
    void SetSkeletonInstance(TSkeletonInstance* instance) { m_pSkeletonInstance = instance; }
    int GetGuardFlags() const { return m_iGuardFlags; }
    void SetClipFlags(int flags) { m_iClipFlags = flags; }
    int GetClipFlags() const { return m_iClipFlags; }
    TRenderInterface& GetRenderer() const { return *m_pRenderer; }
    const PROJECTIONPARAMS& GetProjectionParameters() const { return m_oProjParams; }
    const VIEWPORTPARAMS& GetViewportParameters() const { return m_oViewportParams; }
    const TMatrix44& GetWorldViewMatrix() const { return m_oWorldViewMatrix; }
    const TMatrix44& GetModelViewMatrix() const { return m_oModelViewMatrix; }
    PROJTYPE GetProjType() const { return m_eProjType; }

protected:
    void SetFlag(FLAG flag, bool enable)
    {
        if (enable)
            m_eFlags |= flag;
        else
            m_eFlags &= ~flag;
    }
    bool GetFlag(FLAG flag) const { return (m_eFlags & flag) != 0; }
    bool IsDirty();
    void SetDirty(bool dirty);

public:
    TRenderInterface*  m_pRenderer;             // +0x004
    unsigned int       m_eFlags;                // +0x008
    int                m_iClipFlags;            // +0x00C
    int                m_iGuardFlags;           // +0x010
    PROJTYPE           m_eProjType;             // +0x014
    VIEWPORTPARAMS     m_oViewportParams;       // +0x018
    PROJECTIONPARAMS   m_oProjParams;           // +0x030
    TSkeletonInstance* m_pSkeletonInstance;     // +0x048
    TMatrix44          m_oModelViewMatrix;      // +0x04C
    TMatrix44          m_oWorldViewMatrix;      // +0x08C
    TMatrix44          m_oModelWorldMatrix;     // +0x0CC
    TMatrix44          m_oViewWorldMatrix;      // +0x10C
    TVector4           m_oAmbientColour;        // +0x14C
    TVector4           m_oFogColour;            // +0x15C
    float              m_fFogDistanceStart;     // +0x16C
    float              m_fFogDistanceEnd;       // +0x170
    TPlane             m_aViewPlanes[6];        // +0x174
    TPlane             m_aWorldPlanes[6];       // +0x1D4
    TPlane             m_aGuardPlanes[6];       // +0x234
    TPlane             m_aWorldGuardPlanes[6];  // +0x294
    TSun               m_oSun;                  // +0x2F4
    TAtmosphere        m_oAtmosphere;           // +0x34C
    TMatrix44          m_oWorldModelMatrix;     // +0x404
    TMatrix44          m_oViewModelMatrix;      // +0x444

private:
    TRenderContext(const TRenderContext&);              // no copy exported by the original
    TRenderContext& operator=(const TRenderContext&);
};

} // namespace Toshi
#endif
