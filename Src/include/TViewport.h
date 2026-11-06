// Toshi::TViewport (TRenderInterface.dll, TViewport.cpp, 0x12190-0x12bd0): a scene object with
// its own render context, a list of handlers and eight emitters (update and render phases plus
// destroy/change). The event classes, THandler and the accessors are inline (0xf5b0-0xf7c8).
#ifndef TVIEWPORT_H
#define TVIEWPORT_H
#include "TSceneObject.h"
#include "TNodeList.h"
#include "TObject.h"

namespace Toshi {

class TScene;
class TRenderContext;

class TRENDER_API TViewport : public TSceneObject                       // 0xb0
{
    TOSHI_DECLARE_CLASS(TViewport)
public:
    class TRENDER_API BeginUpdateEvent { public: BeginUpdateEvent() : m_pUnk(0) {} void* m_pUnk; };
    class TRENDER_API UpdateEvent      { public: UpdateEvent() : m_pUnk(0) {} void* m_pUnk; };
    class TRENDER_API EndUpdateEvent   { public: EndUpdateEvent() : m_pUnk(0) {} void* m_pUnk; };
    class TRENDER_API BeginRenderEvent { public: BeginRenderEvent() : m_pUnk(0) {} void* m_pUnk; };
    class TRENDER_API RenderEvent      { public: RenderEvent(TScene& scene) : m_rScene(scene) {} TScene& m_rScene; };
    class TRENDER_API EndRenderEvent   { public: EndRenderEvent() : m_pUnk(0) {} void* m_pUnk; };

    class TRENDER_API THandler : public TNodeList<THandler>::TNode      // 0x14
    {
    public:
        THandler(TViewport& viewport)
        {
            m_pViewport = &viewport;
            viewport.AddHandler(*this);
        }
        virtual ~THandler() {}
        TViewport& GetViewport() { return *m_pViewport; }

        TViewport* m_pViewport;     // +0x10

    private:
        THandler& operator=(const THandler&);   // not exported (the copy constructor is)
    };

    TViewport();
    virtual ~TViewport();

    virtual bool Update(TScene& scene);                                 // slot 10
    virtual bool Render(TScene& scene);                                 // slot 11
    virtual bool Create(bool connectDefaultHandlers);                   // slot 14

    float SetX(float x);
    float SetY(float y);
    float SetWidth(float width);
    float SetHeight(float height);
    float SetMinZ(float minZ);
    float SetMaxZ(float maxZ);
    void SetBackgroundColour(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    bool EnableDefaultBeginRender(bool enable);
    bool AllowDepthClear(bool allow);
    bool AllowBackgroundClear(bool allow);
    void DeleteHandlers();
    void AddHandler(THandler& handler);
    void GetBackgroundColour(unsigned char& r, unsigned char& g, unsigned char& b, unsigned char& a);
    float GetMaxZ();
    float GetMinZ();
    float GetHeight();
    float GetWidth();
    float GetY();
    float GetX();

    TEmitter<TViewport, TNullEvent>& GetChangeEmitter() { return m_oChangeEmitter; }
    TEmitter<TViewport, TNullEvent>& GetDestroyEmitter() { return m_oDestroyEmitter; }
    TEmitter<TViewport, EndRenderEvent>& GetEndRenderEmitter() { return m_oEndRenderEmitter; }
    TEmitter<TViewport, RenderEvent>& GetRenderEmitter() { return m_oRenderEmitter; }
    TEmitter<TViewport, BeginRenderEvent>& GetBeginRenderEmitter() { return m_oBeginRenderEmitter; }
    TEmitter<TViewport, EndUpdateEvent>& GetEndUpdateEmitter() { return m_oEndUpdateEmitter; }
    TEmitter<TViewport, UpdateEvent>& GetUpdateEmitter() { return m_oUpdateEmitter; }
    TEmitter<TViewport, BeginUpdateEvent>& GetBeginUpdateEmitter() { return m_oBeginUpdateEmitter; }
    bool IsEnabled() { return m_bEnabled; }
    void Enable(bool enable) { m_bEnabled = enable; }
    bool IsEnableDefaultBeginRender() { return m_bEnableDefaultBeginRender; }
    bool IsDepthClearAllowed() { return m_bAllowDepthClear; }
    bool IsBackgroundClearAllowed() { return m_bAllowBackgroundClear; }
    TNodeList<THandler>& GetHandlers() { return m_oHandlers; }
    TRenderContext* GetRenderContext() { return m_pRenderContext; }

protected:
    TRenderContext*                       m_pRenderContext;         // +0x34
    TNodeList<THandler>                   m_oHandlers;              // +0x38
    TEmitter<TViewport, BeginUpdateEvent> m_oBeginUpdateEmitter;    // +0x48
    TEmitter<TViewport, UpdateEvent>      m_oUpdateEmitter;         // +0x54
    TEmitter<TViewport, EndUpdateEvent>   m_oEndUpdateEmitter;      // +0x60
    TEmitter<TViewport, BeginRenderEvent> m_oBeginRenderEmitter;    // +0x6c
    TEmitter<TViewport, RenderEvent>      m_oRenderEmitter;         // +0x78
    TEmitter<TViewport, EndRenderEvent>   m_oEndRenderEmitter;      // +0x84
    TEmitter<TViewport, TNullEvent>       m_oDestroyEmitter;        // +0x90
    TEmitter<TViewport, TNullEvent>       m_oChangeEmitter;         // +0x9c
    unsigned char                         m_aBackgroundColour[4];   // +0xa8 (r, g, b, a)
    bool                                  m_bAllowBackgroundClear;  // +0xac
    bool                                  m_bAllowDepthClear;       // +0xad
    bool                                  m_bEnableDefaultBeginRender; // +0xae
    bool                                  m_bEnabled;               // +0xaf

private:
    TViewport(const TViewport&);
    TViewport& operator=(const TViewport&);
};

} // namespace Toshi
#endif
