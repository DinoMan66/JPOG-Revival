// TSun / TAtmosphere (TRenderInterface.dll, TEnvironment.cpp in the original). Daylight model after
// Preetham / Hoffman ("Rendering Outdoor Light Scattering in Real Time", ATI 2002): sun colour from
// atmospheric attenuation, Rayleigh/Mie scattering constants for the vertex shader.
// Members defined in the class bodies sit in the original's inline/COMDAT region (0x7650-0x7b10).
#ifndef TENVIRONMENT_H
#define TENVIRONMENT_H
#include "TRenderAPI.h"
#include "TObject.h"
#include "TVector.h"

namespace Toshi {

class TRENDER_API TSun : public TObject             // 0x58
{
    TOSHI_DECLARE_CLASS(TSun)
public:
    TSun();
    TSun(float theta, float intensity)
    {
        m_fTheta = 0.0f;
        m_fIntensity = intensity;
        SetTheta(theta);
    }
    // no user-declared destructor: the implicit one skips the vtable store (round 42)

    void SetTheta(float theta);
    void SetIntensity(float intensity);
    void Set(float theta, float intensity, const TVector3& biasColour, float biasInterp);
    void SetBiasColour(const TVector3& colour, float interp);

    float GetTheta() const { return m_fTheta; }
    float GetIntensity() const { return m_fIntensity; }
    const TVector4& GetColor() const { return m_vColor; }
    void GetColorAndIntensity(TVector4& out) const
    {
        out = m_vColor;
        out.w = m_fIntensity;
    }
    const TVector4& GetColorWithIntensity() const { return m_vColorWithIntensity; }
    const TVector3& GetDirection() const { return m_vDirection; }
    const TVector3& GetBiasColour() const { return m_vBiasColour; }
    float GetBiasInterp() const { return m_fBiasInterp; }

private:
    void ComputeAttenuation(float theta, TVector4& out, int turbidity);

public:
    float    m_fTheta;                  // +0x04
    float    m_fIntensity;              // +0x08
    TVector3 m_vDirection;              // +0x0C
    TVector4 m_vAttenuation;            // +0x18
    TVector4 m_vColorWithIntensity;     // +0x28
    TVector3 m_vBiasColour;             // +0x38 (read as a TVector4 by Lerp3: w is m_fBiasInterp)
    float    m_fBiasInterp;             // +0x44
    TVector4 m_vColor;                  // +0x48
};

class TRENDER_API TAtmosphere : public TObject      // 0xB8
{
    TOSHI_DECLARE_CLASS(TAtmosphere)
public:
    TAtmosphere();
    TAtmosphere(float betaRayMult, float betaMieMult, float inscatteringMult, float extinctionMult, float hgg)
        : m_vRayleighColour(1.0f, 1.0f, 1.0f), m_vMieColour(1.0f, 1.0f, 1.0f)
    {
        InitVSConstants();
        CalculateScatteringConstants();
        SetAll(betaRayMult, betaMieMult, m_vRayleighColour, m_vMieColour, inscatteringMult, extinctionMult, hgg);
        m_fNear = -10000.0f;
        m_fFar = 10000.0f;
    }
    // no user-declared destructor (round 42)

    void SetAtmHGg(float g);
    void SetAll(float betaRayMult, float betaMieMult, const TVector3& rayleighColour, const TVector3& mieColour,
                float inscatteringMult, float extinctionMult, float hgg);
    void SetAtmExtinctionMult(float mult);
    void SetAtmInscatteringMult(float mult);
    void SetAtmBetaMieMult(float mult);
    void SetAtmBetaRayMult(float mult);

    float GetAtmBetaRayMult() const { return m_fBetaRayMult; }
    float GetAtmBetaMieMult() const { return m_fBetaMieMult; }
    float GetAtmInscatteringMult() const { return m_fInscatteringMult; }
    float GetAtmExtinctionMult() const { return m_fExtinctionMult; }
    float GetAtmHGg() const { return m_fHGg; }
    const TVector3& GetBetaRayleigh() const { return m_vBetaRayleigh; }
    const TVector3& GetBetaDashRayleigh() const { return m_vBetaDashRayleigh; }
    const TVector3& GetBetaMie() const { return m_vBetaMie; }
    const TVector3& GetBetaDashMie() const { return m_vBetaDashMie; }
    const TVector4& GetVSC_HG() const { return m_vVSC_HG; }
    const TVector4& GetVSC_ONE_OVER_BETA_1_PLUS_2() const { return m_vVSC_ONE_OVER_BETA_1_PLUS_2; }
    const TVector4& GetVSC_BETA_1_PLUS_2() const { return m_vVSC_BETA_1_PLUS_2; }
    const TVector4& GetVSC_BETA_DASH_1() const { return m_vVSC_BETA_DASH_1; }
    const TVector4& GetVSC_BETA_DASH_2() const { return m_vVSC_BETA_DASH_2; }
    void SetNear(float value) { m_fNear = value; }
    void SetFar(float value) { m_fFar = value; }
    float GetNear() const { return m_fNear; }
    float GetFar() const { return m_fFar; }

private:
    void CalculateScatteringConstants();
    void CalculateVSConstants();
    void InitVSConstants();

public:
    TVector3 m_vBetaMie;                        // +0x04
    TVector3 m_vBetaDashMie;                    // +0x10
    TVector3 m_vBetaDashRayleigh;               // +0x1C
    TVector3 m_vBetaRayleigh;                   // +0x28
    float    m_fNear;                           // +0x34
    float    m_fFar;                            // +0x38
    float    m_fBetaRayMult;                    // +0x3C
    float    m_fBetaMieMult;                    // +0x40
    float    m_fInscatteringMult;               // +0x44
    float    m_fExtinctionMult;                 // +0x48
    float    m_fHGg;                            // +0x4C
    TVector3 m_vRayleighColour;                 // +0x50 (Rayleigh colour * ray multiplier)
    TVector3 m_vMieColour;                      // +0x5C (Mie colour * mie multiplier)
    TVector4 m_vVSC_HG;                         // +0x68
    TVector4 m_vVSC_ONE_OVER_BETA_1_PLUS_2;     // +0x78
    TVector4 m_vVSC_BETA_1_PLUS_2;              // +0x88
    TVector4 m_vVSC_BETA_DASH_1;                // +0x98
    TVector4 m_vVSC_BETA_DASH_2;                // +0xA8
};

} // namespace Toshi
#endif
