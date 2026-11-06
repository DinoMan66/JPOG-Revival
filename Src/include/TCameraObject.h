// Toshi::TCameraObject / TCameraViewportHandler (TRenderInterface.dll, TCameraObject.cpp,
// 0xea70-0xf4c0). The handler listens to a viewport's update/render events and sets up the
// render context's projection and view matrices from the camera.
#ifndef TCAMERAOBJECT_H
#define TCAMERAOBJECT_H
#include "TSceneObject.h"
#include "TViewport.h"

namespace Toshi {

class TCameraObject;
class TTransformObject;
class TRenderInterface;

class TRENDER_API TCameraViewportHandler                                // 0x54
{
    friend class TCameraObject;
public:
    TCameraViewportHandler(TCameraObject& camera);
    ~TCameraViewportHandler() {}                                        // inline (0xf400)

    TRenderInterface* GetRenderer();
    TCameraObject& GetCamera() { return *m_pCamera; }                   // inline (0xf3d0)

    static bool __stdcall OnBeginRenderEvent(TCameraViewportHandler* handler, TViewport* viewport,
                                             TViewport::BeginRenderEvent* event);
    static bool __stdcall OnRenderEvent(TCameraViewportHandler* handler, TViewport* viewport,
                                        TViewport::RenderEvent* event);
    static bool __stdcall OnEndRenderEvent(TCameraViewportHandler* handler, TViewport* viewport,
                                           TViewport::EndRenderEvent* event);
    static bool __stdcall OnUpdateEvent(TCameraViewportHandler* handler, TViewport* viewport,
                                        TViewport::UpdateEvent* event) { return true; } // inline (0xf3c0)

private:
    void ConnectEventHandlers();
    void DisconnectEventHandlers();

    TCameraObject* m_pCamera;                                                       // +0x00
    TListener<TCameraViewportHandler, TViewport, TViewport::UpdateEvent>      m_oUpdateListener;      // +0x04
    TListener<TCameraViewportHandler, TViewport, TViewport::BeginRenderEvent> m_oBeginRenderListener; // +0x18
    TListener<TCameraViewportHandler, TViewport, TViewport::RenderEvent>      m_oRenderListener;      // +0x2c
    TListener<TCameraViewportHandler, TViewport, TViewport::EndRenderEvent>   m_oEndRenderListener;   // +0x40

    TCameraViewportHandler& operator=(const TCameraViewportHandler&);   // not exported (the copy constructor is)
};

class TRENDER_API TCameraObject : public TSceneObject                   // 0xb4
{
    TOSHI_DECLARE_CLASS(TCameraObject)
    friend class TCameraViewportHandler;
public:
    enum FOVMODE { FOVMODE_HORIZONTAL, FOVMODE_VERTICAL };
    enum PROJTYPE { PROJTYPE_PERSPECTIVE, PROJTYPE_ORTHOGRAPHIC };

    TCameraObject();
    virtual ~TCameraObject();

    virtual bool Create();                                              // slot 3
    virtual void OnDestroy();                                           // slot 9

    float SetFOV(float fov);
    float SetProjectionCentreX(float x);
    float SetProjectionCentreY(float y);
    TViewport* SetViewport(TViewport* viewport);
    bool Enable(bool enable);
    TViewport* GetViewport() const;
    TTransformObject* GetTransformObject();
    float GetProjectionCentreY();
    float GetProjectionCentreX();
    float GetFOV();
    FOVMODE GetFOVMode();
    float GetFar();
    float GetNear();
    TSceneObject* SetLookatTarget(TSceneObject* target);
    void SetProjType(PROJTYPE type);
    FOVMODE SetFOVMode(FOVMODE mode);
    float SetFar(float farClip);
    float SetNear(float nearClip);
    bool IsEnabled() { return m_bEnabled; }                             // inline (0xf3e0)
    PROJTYPE GetProjType() { return m_eProjType; }                      // inline (0xf3f0)

protected:
    TCameraViewportHandler m_oViewportHandler;      // +0x34
    float                  m_fNear;                 // +0x88
    float                  m_fFar;                  // +0x8c
    float                  m_fFOV;                  // +0x90
    FOVMODE                m_eFOVMode;              // +0x94
    PROJTYPE               m_eProjType;             // +0x98
    float                  m_fProjCentreX;          // +0x9c
    float                  m_fProjCentreY;          // +0xa0
    TTransformObject*      m_pTransformObject;      // +0xa4
    TSceneObject*          m_pLookatTarget;         // +0xa8
    TViewport*             m_pViewport;             // +0xac
    bool                   m_bEnabled;              // +0xb0

private:
    TCameraObject(const TCameraObject&);
    TCameraObject& operator=(const TCameraObject&);
};

} // namespace Toshi
#endif
