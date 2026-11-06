// Toshi::TGFXFileInstanceObject (TRenderInterface.dll, TGFXFileInstanceObject.cpp, 0xf7d0-0xff00).
// One placed instance of a TGFXFileObject's model: owns a TModelInstance and a TTransformObject
// parent, picks the LOD from the camera distance and can fade out with distance.
#ifndef TGFXFILEINSTANCEOBJECT_H
#define TGFXFILEINSTANCEOBJECT_H
#include "TSceneObject.h"

namespace Toshi {

class TGFXFileObject;
class TModelInstance;
class TTransformObject;

class TRENDER_API TGFXFileInstanceObject : public TSceneObject         // 0x54
{
    TOSHI_DECLARE_CLASS(TGFXFileInstanceObject)
public:
    enum STATE { STATE_VISIBLE = 1, STATE_DISTANCEALPHAFADE = 2 };

    TGFXFileInstanceObject();
    virtual ~TGFXFileInstanceObject();

    virtual void DestroyResource();                                     // slot 6
    virtual void OnDestroy();                                           // slot 9
    virtual bool Update(TScene& scene);                                 // slot 10
    virtual bool Render(TScene& scene);                                 // slot 11
    virtual bool Create(TGFXFileObject* gfxFile);                       // slot 14

    void SetTextureStagesA(signed char* stages);
    void GetTextureStages(signed char* stages);
    void SetTextureAlphasA(unsigned char* alphas);
    void GetTextureAlphas(unsigned char* alphas);
    void SetTextureAlphas(unsigned char a0, unsigned char a1, unsigned char a2, unsigned char a3);
    void SetTextureStages(signed char s0, signed char s1, signed char s2, signed char s3);

    void SetFade(float start, float end) { m_fFadeStart = start; m_fFadeEnd = end; }
    TTransformObject* GetTransformObject() { return m_pTransformObject; }
    bool GetDistanceAlphaFadeState()
    {
        if (m_uiState & STATE_DISTANCEALPHAFADE)
            return true;
        return false;
    }
    bool EnableDistanceAlphaFade(bool enable)
    {
        bool old = GetDistanceAlphaFadeState();
        unsigned int state = m_uiState & ~STATE_DISTANCEALPHAFADE;
        m_uiState = state | (enable == true ? STATE_DISTANCEALPHAFADE : 0);
        return old;
    }
    bool IsVisibile()
    {
        if (m_uiState & STATE_VISIBLE)
            return true;
        return false;
    }
    bool SetVisible(bool visible)
    {
        bool old = IsVisibile();
        unsigned int state = m_uiState & ~STATE_VISIBLE;
        m_uiState = state | (visible == true ? STATE_VISIBLE : 0);
        return old;
    }
    TModelInstance* GetModelInstance() { return m_pModelInstance; }
    TGFXFileObject* GetGFXFile() { return m_pGFXFile; }
    static void __stdcall ForceLowLOD(bool force) { sm_bForceLowLOD = force; }

protected:
    TGFXFileObject*   m_pGFXFile;           // +0x34
    TModelInstance*   m_pModelInstance;     // +0x38
    TTransformObject* m_pTransformObject;   // +0x3c
    unsigned int      m_uiState;            // +0x40
    signed char       m_acTextureStages[4]; // +0x44
    unsigned char     m_aucTextureAlphas[4];// +0x48
    float             m_fFadeStart;         // +0x4c
    float             m_fFadeEnd;           // +0x50

private:
    static bool sm_bForceLowLOD;

    TGFXFileInstanceObject(const TGFXFileInstanceObject&);              // not exported
    TGFXFileInstanceObject& operator=(const TGFXFileInstanceObject&);
};

} // namespace Toshi
#endif
