// Toshi DirectInput 8 back-end - reconstruction of TInputDXInterface.dll
// Sources (from the game's file list): Z:\Toshi\Source\TInputDX\
//   TInputDXDeviceController.cpp  TInputDXDeviceKeyboard.cpp  TInputDXDeviceMouse.cpp  TInputDXInterface.cpp
//
// Vtables (see tools\vtables.py):
//   Controller: TInputDeviceController slots + 23 Initialise 24 Deinitialise
//   Keyboard:   TInputDeviceKeyboard slots   + 26 Initialise 27 Deinitialise 28 RefreshDirect
//   Mouse:      TInputDeviceMouse slots      + 20 Initialise 21 Deinitialise 22 RefreshDirect 23 WasDown
//   Interface:  TInputInterface slots        + 12 RefreshDirect
#ifndef TINPUTDXINTERFACE_H
#define TINPUTDXINTERFACE_H
#include "dinput8min.h"
#include "TInputInterface.h"
#include "TLString.h"

#ifdef TINPUTDX_EXPORTS
#define TINPUTDX_API __declspec(dllexport)
#else
#define TINPUTDX_API __declspec(dllimport)
#endif

namespace Toshi {

// ------------------------------------------------------------------ TInputDXDeviceController
class TINPUTDX_API TInputDXDeviceController : public TInputDeviceController
{
public:
    TOSHI_DECLARE_CLASS(TInputDXDeviceController)

    TInputDXDeviceController();
    virtual ~TInputDXDeviceController();
    virtual bool  Acquire();                                                        // 3
    virtual bool  Unacquire();                                                      // 4
    virtual void  Release();                                                        // 5
    virtual void  Update(float dt);                                                 // 6
    virtual int   ProcessEvents(Emitter& emitter, float dt);                        // 8
    virtual int   GetButtonCount();                                                 // 9
    virtual int   GetAxisCount();                                                   // 10
    virtual bool  IsDown(int doodad) const;                                         // 17
    virtual int   GetAxisInt(int doodad, int axis) const;                           // 18
    virtual float GetAxisFloat(int doodad, int axis) const;                         // 19
    virtual bool  WasDown(int doodad) const;                                        // 21
    virtual bool  Initialise();                                                     // 23
    virtual bool  Deinitialise();                                                   // 24

    const bool BindToDIDevice(HWND hwnd, const DIDEVICEINSTANCEA* instance, IDirectInputDevice8A* device);
    int TranslateDXButtonToDoodad(int button) const;
    int TranslateDoodadToDXButton(int doodad) const;

private:
    static BOOL __stdcall EnumerateObjectCallback(const DIDEVICEOBJECTINSTANCEA* object, void* context);
    static float __stdcall NormaliseStick(int value);

    static const int STICK_CENTRE;
    static const int STICK_DEADZONE;
    static const int STICK_SATURATE;
    static const int STICK_RANGE;

public:
    DIDEVICEINSTANCEA     m_oDeviceInstance;    // +0x054
    DIDEVCAPS             m_oDIDevCaps;         // +0x298
    IDirectInputDevice8A* m_poDXInputDevice;    // +0x2c4
    DIJOYSTATE            m_oJoyState;          // +0x2c8
    DIJOYSTATE            m_oPrevJoyState;      // +0x318
    float                 m_afSticks[4];        // +0x368  normalised lX, lY, lZ, lRz
    bool                  m_bUpdated;           // +0x378                    sizeof == 0x37c
};

// ------------------------------------------------------------------ TInputDXDeviceKeyboard
class TINPUTDX_API TInputDXDeviceKeyboard : public TInputDeviceKeyboard
{
    friend class TInputDXInterface;
public:
    TOSHI_DECLARE_CLASS(TInputDXDeviceKeyboard)

    TInputDXDeviceKeyboard();
    virtual ~TInputDXDeviceKeyboard();
    virtual bool  Acquire();                                                        // 3
    virtual bool  Unacquire();                                                      // 4
    virtual void  Release();                                                        // 5
    virtual void  Update(float dt);                                                 // 6
    virtual bool  Flush();                                                          // 7
    virtual int   ProcessEvents(Emitter& emitter, float dt);                        // 8
    virtual int   GetButtonCount();                                                 // 9
    virtual bool  IsDown(int doodad) const;                                         // 17
    virtual bool  IsShiftDown() const;                                              // 20
    virtual bool  IsControlDown() const;                                            // 21
    virtual bool  IsAltDown() const;                                                // 22
    virtual bool  WasDown(int doodad) const;                                        // 23
    virtual unsigned short TranslateDoodadToCharacter(int doodad) const;            // 24
    virtual bool  Initialise();                                                     // 26
    virtual bool  Deinitialise();                                                   // 27
    virtual void  RefreshDirect();                                                  // 28

    TLString GetLocalisedKeyName(int doodad) const;

protected:
    void ClearAll();
    bool HandleKeyChange(Emitter& emitter, int key, unsigned char state);
    void SetUp(int key);
    void SetDown(int key);
    void Clear(int key);
    static int __stdcall TranslateDoodadToDX(int doodad);
    static int __stdcall TranslateDXToDoodad(int key);

private:
    const bool BindToDIDevice(HWND hwnd, const DIDEVICEINSTANCEA* instance, IDirectInputDevice8A* device);
    static BOOL __stdcall EnumObjectCallback(const DIDEVICEOBJECTINSTANCEA* object, void* context);

    static const int sm_ciKeyboardBufferSize;

public:
    int                   m_iBuffer;            // +0x06c  which of the two state buffers is current
    char*                 m_pcCurrentState;     // +0x070
    char*                 m_pcPreviousState;    // +0x074
    char                  m_acBuffers[2][256];  // +0x078
    TLString              m_aKeyNames[128];     // +0x278
    bool                  m_bUpdated;           // +0x478
    DIDEVICEINSTANCEA     m_oDeviceInstance;    // +0x47c
    DIDEVCAPS             m_oDIDevCaps;         // +0x6c0
    IDirectInputDevice8A* m_poDXInputDevice;    // +0x6ec
    bool                  m_bRefreshed;         // +0x6f0                    sizeof == 0x6f4
};

// ------------------------------------------------------------------ TInputDXDeviceMouse
class TINPUTDX_API TInputDXDeviceMouse : public TInputDeviceMouse
{
    friend class TInputDXInterface;
public:
    TOSHI_DECLARE_CLASS(TInputDXDeviceMouse)

    TInputDXDeviceMouse();
    virtual ~TInputDXDeviceMouse();
    virtual bool  Acquire();                                                        // 3
    virtual bool  Unacquire();                                                      // 4
    virtual void  Release();                                                        // 5
    virtual void  Update(float dt);                                                 // 6
    virtual bool  Flush();                                                          // 7
    virtual int   ProcessEvents(Emitter& emitter, float dt);                        // 8
    virtual int   GetButtonCount();                                                 // 9
    virtual int   GetAxisCount();                                                   // 10
    virtual bool  IsDown(int doodad) const;                                         // 17
    virtual int   GetAxisInt(int doodad, int axis) const;                           // 18
    virtual float GetAxisFloat(int doodad, int axis) const;                         // 19
    virtual bool  Initialise();                                                     // 20
    virtual bool  Deinitialise();                                                   // 21
    virtual void  RefreshDirect();                                                  // 22
    virtual bool  WasDown(int doodad) const;                                        // 23

    static const int sm_ciMouseBufferSize;

private:
    const bool BindToDIDevice(HWND hwnd, const DIDEVICEINSTANCEA* instance, IDirectInputDevice8A* device);

public:
    bool                  m_bSwapButtons;       // +0x03c
    int                   m_iButtons;           // +0x040  bit n = button n down
    int                   m_iPrevButtons;       // +0x044
    int                   m_iX, m_iY, m_iZ;     // +0x048  accumulated position
    int                   m_iDX, m_iDY, m_iDZ;  // +0x054  movement since the last Update
    float                 m_fUnk60[3];          // +0x060
    int                   m_iUnk6c, m_iUnk70;   // +0x06c
    int                   m_iUnk74, m_iUnk78;   // +0x074  set to -1
    int                   m_iWheelGranularity;  // +0x07c
    DIDEVICEINSTANCEA     m_oDeviceInstance;    // +0x080
    DIDEVCAPS             m_oDIDevCaps;         // +0x2c4
    IDirectInputDevice8A* m_poDXInputDevice;    // +0x2f0                    sizeof == 0x2f4
};

// ------------------------------------------------------------------ TInputDXInterface
class TINPUTDX_API TInputDXInterface : public TInputInterface
{
public:
    TOSHI_DECLARE_CLASS(TInputDXInterface)

    TInputDXInterface();
    virtual ~TInputDXInterface();
    virtual bool Initialise();                                                      // 3
    virtual bool Deinitialise();                                                    // 4
    virtual void RefreshDirect() {}                                                 // 12

    static TInputDXInterface* __stdcall GetInterface() { return ms_pTheInterface; }
    HWND GetMainWindow() const { return m_hMainWindow; }
    void SetMainWindow(HWND hwnd) { m_hMainWindow = hwnd; }

private:
    IDirectInput8A* GetDirectInput() const;
    static BOOL __stdcall EnumerateDeviceCallback(const DIDEVICEINSTANCEA* instance, void* context);

    static TInputDXInterface* ms_pTheInterface;

public:
    IDirectInput8A* m_poDirectInput;            // +0x34
    HWND            m_hMainWindow;              // +0x38                     sizeof == 0x3c
};

} // namespace Toshi
#endif
