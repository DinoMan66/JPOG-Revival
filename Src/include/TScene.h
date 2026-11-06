// Toshi::TScene (TRenderInterface.dll, TScene.cpp, 0x10920-0x10ef0). The members below are known
// from the inline accessors (0x6cd0-0x6de0) and their users; TScene.cpp is not decompiled yet.
#ifndef TSCENE_H
#define TSCENE_H
#include "TResource.h"
#include "TVector.h"
#include "TStack.h"

namespace Toshi {

class TCameraObject;
class TNullSceneResource;
class TViewport;

class TRENDER_API TScene : public TResource                             // 0x580+
{
    TOSHI_DECLARE_CLASS(TScene)
public:
    enum DEFCAMERA { DEFCAMERA_NUMOF = 5 };

    TScene();
    virtual ~TScene();

    virtual bool Create();                                              // slot 3

    bool Begin();
    bool Update(float deltaTime);
    bool Render();
    bool RenderScene();
    bool End();
    bool Recurse(float deltaTime);
    TCameraObject* CreateCamera(const char* name, TResource* parent);
    TViewport* CreateViewport(const char* name, TResource* parent, bool connectDefaultHandlers);

    void SetAmbientColour(float r, float g, float b) { m_oAmbientColour.Set(r, g, b, 0.0f); }
    void SetAmbientColour(const TVector4& colour) { m_oAmbientColour = colour; }
    void GetAmbientColour(TVector4& colour) const { colour = m_oAmbientColour; }
    const TVector4& GetAmbientColour() const { return m_oAmbientColour; }
    void SetCurrentCamera(TCameraObject* camera) { m_pCurrentCamera = camera; }
    TCameraObject* GetCurrentCamera() { return m_pCurrentCamera; }
    TNullSceneResource* GetViewportRoot() { return m_pViewportRoot; }
    TCameraObject* GetDefaultCamera(DEFCAMERA camera) { return m_apDefaultCameras[camera]; }
    TStack<TMatrix44, 20>& GetRenderMatrixStack() { return m_oRenderMatrixStack; }
    float GetDeltaTime() const { return m_fDeltaTime; }
    float GetAbsoluteTime() const { return m_fAbsoluteTime; }
    unsigned int GetSceneStamp() { return m_uiSceneStamp; }

    int                   m_iNumNonSceneObjects;    // +0x30 (counted by TSceneObject::Recurse; name unknown)
    unsigned int          m_auiUnk34[3];            // +0x34
    bool                  m_bInScene;               // +0x40 (between Begin and End)
    unsigned int          m_uiSceneStamp;           // +0x44
    float                 m_fDeltaTime;             // +0x48
    float                 m_fAbsoluteTime;          // +0x4c
    TStack<TMatrix44, 20> m_oRenderMatrixStack;     // +0x50
    TNullSceneResource*   m_pViewportRoot;          // +0x554
    TCameraObject*        m_apDefaultCameras[DEFCAMERA_NUMOF]; // +0x558
    TCameraObject*        m_pCurrentCamera;         // +0x56c
    TVector4              m_oAmbientColour;         // +0x570

private:
    TScene(const TScene&);              // not exported: no copy constructor in the original
    TScene& operator=(const TScene&);
};

} // namespace Toshi
#endif
