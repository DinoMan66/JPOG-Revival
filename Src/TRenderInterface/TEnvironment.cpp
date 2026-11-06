// TRenderInterface.dll - TEnvironment.cpp: TAtmosphere and TSun (0x18690-0x190f0 in the original).
// The scattering and attenuation code follows Hoffman & Preetham's ATI sample (2002), which Toshi adopted.
#define TRENDERINTERFACE_EXPORTS
#include <math.h>
#include "TNewDelete.h"
#include "TEnvironment.h"
#include "TMath.h"

namespace Toshi {

// ---------------------------------------------------------------- TAtmosphere
TAtmosphere::TAtmosphere()                                              // 0x18690
    : m_vRayleighColour(1.0f, 1.0f, 1.0f), m_vMieColour(1.0f, 1.0f, 1.0f)
{
    m_fNear = -1.0f;
    m_fFar = 1.0f;
    InitVSConstants();
    CalculateScatteringConstants();
}

void TAtmosphere::SetAtmHGg(float g)                                    // 0x186f0
{
    m_fHGg = g;
#if !defined(TENV_HG) || TENV_HG == 1   // round 42: closest
    m_vVSC_HG.Set((1.0f - g) * (1.0f - g), 1.0f + g * g, 2.0f * g, 0.0f);
#elif TENV_HG == 4
    m_vVSC_HG.Set((1.0f - g) * (1.0f - g), 1.0f + g * g, 2.0 * g, 0.0f);
#elif TENV_HG == 5
    m_vVSC_HG.Set((1.0f - g) * (1.0f - g), 1.0f + g * g, g * 2.0f, 0.0f);
#elif TENV_HG == 6
    m_vVSC_HG.Set((1.0 - g) * (1.0 - g), 1.0 + g * g, 2.0 * g, 0.0f);
#elif TENV_HG == 2
    m_vVSC_HG = TVector4((1.0f - g) * (1.0f - g), 1.0f + g * g, 2.0f * g, 0.0f);
#elif TENV_HG == 3
    float g2 = g * g;
    m_vVSC_HG.Set((1.0f - g) * (1.0f - g), 1.0f + g2, g * 2.0f, 0.0f);
#else
    m_vVSC_HG.x = (1.0f - g) * (1.0f - g);
    m_vVSC_HG.y = 1.0f + g * g;
    m_vVSC_HG.z = 2.0f * g;
    m_vVSC_HG.w = 0.0f;
#endif
}

void TAtmosphere::SetAll(float betaRayMult, float betaMieMult, const TVector3& rayleighColour, const TVector3& mieColour,
                         float inscatteringMult, float extinctionMult, float hgg) // 0x18730
{
    m_fBetaRayMult = betaRayMult;
    m_fBetaMieMult = betaMieMult;
    m_fInscatteringMult = inscatteringMult;
    m_fExtinctionMult = extinctionMult;
    m_vRayleighColour.Multiply(rayleighColour, betaRayMult);
    m_vMieColour.Multiply(mieColour, betaMieMult);
    SetAtmHGg(hgg);
    CalculateVSConstants();
}

void TAtmosphere::CalculateScatteringConstants()                        // 0x18790
{
    float fLambda[3], fLambda2[3], fLambda4[3];
    fLambda[0] = 1.0f / 650e-9f;        // red
    fLambda[1] = 1.0f / 570e-9f;        // green
    fLambda[2] = 1.0f / 475e-9f;        // blue

    for (int i = 0; i < 3; i++)
    {
        fLambda2[i] = fLambda[i] * fLambda[i];
        fLambda4[i] = fLambda2[i] * fLambda2[i];
    }

    TVector3 vLambda2(fLambda2[0], fLambda2[1], fLambda2[2]);
    TVector3 vLambda4(fLambda4[0], fLambda4[1], fLambda4[2]);

    // Rayleigh scattering constants
    const float n = 1.0003f;
    const float N = 2.545e25f;
    const float pn = 0.035f;
    const float fPI = 3.1415927f;
    const float fTemp = fPI * fPI * (n * n - 1.0f) * (n * n - 1.0f) * (6.0f + 3.0f * pn) / (6.0f - 7.0f * pn) / N;
    const float fBeta = 8.0f * fTemp * fPI / 3.0f;
    m_vBetaRayleigh = vLambda4 * fBeta;
    const float fBetaDash = fTemp / 2.0f;
    m_vBetaDashRayleigh = vLambda4 * fBetaDash;

    // Mie scattering constants
    const float T = 2.0f;
    const float c = (6.544f * T - 6.51f) * 1e-17f;
    const float fTemp2 = 0.434f * c * (2.0f * fPI) * (2.0f * fPI) * 0.5f;
    m_vBetaDashMie = vLambda2 * fTemp2;

    const float K[3] = { 0.685f, 0.679f, 0.670f };
    const float fTemp3 = 0.434f * c * fPI * (2.0f * fPI) * (2.0f * fPI);
    TVector3 vBetaMieTemp(K[0] * fLambda2[0], K[1] * fLambda2[1], K[2] * fLambda2[2]);
    m_vBetaMie = vBetaMieTemp * fTemp3;

    m_vVSC_BETA_1_PLUS_2.x = m_vBetaMie.x + m_vBetaRayleigh.x;
    m_vVSC_BETA_1_PLUS_2.y = m_vBetaMie.y + m_vBetaRayleigh.y;
    m_vVSC_BETA_1_PLUS_2.z = m_vBetaMie.z + m_vBetaRayleigh.z;
    m_vVSC_BETA_1_PLUS_2.w = 1.0f;
    m_vVSC_ONE_OVER_BETA_1_PLUS_2.Set(1.0f, 1.0f, 1.0f, 1.0f);
    m_vVSC_ONE_OVER_BETA_1_PLUS_2.Divide(m_vVSC_BETA_1_PLUS_2);
}

void TAtmosphere::CalculateVSConstants()                                // 0x18970
{
    TVector3 vBetaMie;
    vBetaMie.Multiply(m_vBetaMie, m_vMieColour);
    TVector3 vBetaRay;
    vBetaRay.Multiply(m_vBetaRayleigh, m_vRayleighColour);

    m_vVSC_BETA_1_PLUS_2.x = vBetaMie.x + vBetaRay.x;
    m_vVSC_BETA_1_PLUS_2.y = vBetaMie.y + vBetaRay.y;
    m_vVSC_BETA_1_PLUS_2.z = vBetaMie.z + vBetaRay.z;
    m_vVSC_BETA_1_PLUS_2.w = 1.0f;
    m_vVSC_ONE_OVER_BETA_1_PLUS_2.Set(1.0f, 1.0f, 1.0f, 1.0f);
    m_vVSC_ONE_OVER_BETA_1_PLUS_2.Divide(m_vVSC_BETA_1_PLUS_2);

    m_vVSC_BETA_DASH_1.x = m_vBetaDashMie.x * m_fBetaMieMult;
    m_vVSC_BETA_DASH_1.y = m_vBetaDashMie.y * m_fBetaMieMult;
    m_vVSC_BETA_DASH_1.z = m_vBetaDashMie.z * m_fBetaMieMult;
    m_vVSC_BETA_DASH_2.x = m_vBetaDashRayleigh.x * m_fBetaRayMult;
    m_vVSC_BETA_DASH_2.y = m_vBetaDashRayleigh.y * m_fBetaRayMult;
    m_vVSC_BETA_DASH_2.z = m_vBetaDashRayleigh.z * m_fBetaRayMult;
}

void TAtmosphere::InitVSConstants()                                     // 0x18a50
{
    TVector4 one(1.0f, 1.0f, 1.0f, 1.0f);
    m_vVSC_HG = TVector4::VEC_ZERO;
    m_vVSC_ONE_OVER_BETA_1_PLUS_2 = one;
    m_vVSC_BETA_1_PLUS_2 = one;
    m_vVSC_BETA_DASH_1 = one;
    m_vVSC_BETA_DASH_2 = one;
}

// ---------------------------------------------------------------- TSun
TSun::TSun()                                                            // 0x18b10
{
    m_fTheta = -0.1f;
    m_fIntensity = 1.0f;
    m_vBiasColour.Set(1.0f, 1.0f, 1.0f);
    m_fBiasInterp = 0.0f;
    SetTheta(0.0f);
}

void TSun::SetTheta(float theta)                                        // 0x18b60
{
#ifdef TENV_ST
    if (m_fTheta == theta)
        return;
    {
#else
    if (m_fTheta != theta)
    {
#endif
        m_fTheta = theta;
        ComputeAttenuation(theta, m_vAttenuation, 2);
        m_vColor.Lerp3(m_vAttenuation, *(TVector4*)&m_vBiasColour, m_fBiasInterp);
        m_vColor.w = m_vAttenuation.w;
        m_vColorWithIntensity.x = m_vColor.x * m_fIntensity;
        m_vColorWithIntensity.y = m_vColor.y * m_fIntensity;
        m_vColorWithIntensity.z = m_vColor.z * m_fIntensity;
        m_vColorWithIntensity.w = m_vColor.w;
        m_vDirection.Set(0.0f, cos(m_fTheta), sin(m_fTheta));
        static float s_fRotation = TMath::PI / 180.0f * 61.9657f;
        m_vDirection.RotateY(s_fRotation);
    }
}

void TSun::SetIntensity(float intensity)                                // 0x18c20
{
    if (m_fIntensity == intensity)      // early return (round 42)
        return;
    m_fIntensity = intensity;
    m_vColorWithIntensity.x = m_vColor.x * intensity;
    m_vColorWithIntensity.y = m_vColor.y * intensity;
    m_vColorWithIntensity.z = m_vColor.z * intensity;
    m_vColorWithIntensity.w = m_vColor.w;
}

void TSun::Set(float theta, float intensity, const TVector3& biasColour, float biasInterp) // 0x18c60
{
    m_fTheta = theta;
    m_vBiasColour = biasColour;
    m_fBiasInterp = biasInterp;
    m_fIntensity = intensity;
    ComputeAttenuation(theta, m_vAttenuation, 2);
    m_vColor.Lerp3(m_vAttenuation, *(TVector4*)&m_vBiasColour, m_fBiasInterp);
    m_vColor.w = m_vAttenuation.w;
    m_vColorWithIntensity.x = m_vColor.x * m_fIntensity;
    m_vColorWithIntensity.y = m_vColor.y * m_fIntensity;
    m_vColorWithIntensity.z = m_vColor.z * m_fIntensity;
    m_vColorWithIntensity.w = m_vColor.w;
    m_vDirection.Set(0.0f, cos(m_fTheta), sin(m_fTheta));
    static float s_fRotation = TMath::PI / 180.0f * 61.9657f;
    m_vDirection.RotateY(s_fRotation);
}

void TSun::SetBiasColour(const TVector3& colour, float interp)          // 0x18d40
{
    m_vBiasColour = colour;
    m_fBiasInterp = interp;
#ifdef TENV_SB
    m_vColor.Lerp3(m_vAttenuation, *(TVector4*)&m_vBiasColour, m_fBiasInterp);
#else
    m_vColor.Lerp3(m_vAttenuation, *(TVector4*)&m_vBiasColour, interp);
#endif
    m_vColor.w = m_vAttenuation.w;
    m_vColorWithIntensity.x = m_vColor.x * m_fIntensity;
    m_vColorWithIntensity.y = m_vColor.y * m_fIntensity;
    m_vColorWithIntensity.z = m_vColor.z * m_fIntensity;
    m_vColorWithIntensity.w = m_vColor.w;
}

void TSun::ComputeAttenuation(float theta, TVector4& out, int turbidity) // 0x18da0
{
    float fBeta = 0.04608365822050f * turbidity - 0.04586025928522f;
    float fTauR, fTauA, fTau[3];
#if TENV_CA == 1
    float m = 1.0f / (cos(theta) + 0.15f * (float)pow(93.885f - theta / TMath::PI * 180.0f, -1.253f));
#elif TENV_CA == 2
    float m = 1.0f / ((float)cos(theta) + 0.15f * (float)pow(93.885f - theta / TMath::PI * 180.0f, -1.253f));
#else
    float m = 1.0f / (cos(theta) + 0.15f * pow(93.885f - theta / TMath::PI * 180.0f, -1.253f)); // relative optical mass
#endif
    float fLambda[3];
    fLambda[0] = 0.65f;     // red (um)
    fLambda[1] = 0.57f;     // green
    fLambda[2] = 0.475f;    // blue

    for (int i = 0; i < 3; i++)
    {
        // Rayleigh scattering
        fTauR = exp(-m * 0.008735f * pow(fLambda[i], float(-4.08f)));
        // aerosol (water + dust) attenuation
        const float fAlpha = 1.3f;
        fTauA = exp(-m * fBeta * pow(fLambda[i], -fAlpha));
        fTau[i] = fTauR * fTauA;
    }
    out = TVector4(fTau[0], fTau[1], fTau[2], 1.0f);
}

TOSHI_DEFINE_CLASS(TAtmosphere, TObject)
TOSHI_DEFINE_CLASS(TSun, TObject)

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TSun)                             // 0x18f70-0x18fe0

void TAtmosphere::SetAtmExtinctionMult(float mult)                      // 0x18ff0
{
    m_fExtinctionMult = mult;
    CalculateVSConstants();
}

void TAtmosphere::SetAtmInscatteringMult(float mult)                    // 0x19010
{
    m_fInscatteringMult = mult;
    CalculateVSConstants();
}

void TAtmosphere::SetAtmBetaMieMult(float mult)                         // 0x19030
{
    m_fBetaMieMult = mult;
    CalculateVSConstants();
}

void TAtmosphere::SetAtmBetaRayMult(float mult)                         // 0x19050
{
    m_fBetaRayMult = mult;
    CalculateVSConstants();
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TAtmosphere)                      // 0x19070-0x190e0

} // namespace Toshi
