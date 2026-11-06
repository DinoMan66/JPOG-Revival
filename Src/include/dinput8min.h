// Minimal DirectInput 8 declarations (written for this project; only what the Toshi input
// code uses). Layouts and vtable order follow the DirectX 8 SDK's dinput.h, so code that
// calls through these interfaces compiles to the same instructions.
// The real header is not needed: the VM has no DirectX 8 SDK installed.
#ifndef DINPUT8MIN_H
#define DINPUT8MIN_H
#include <windows.h>
#include <objbase.h>

#define DIRECTINPUT_VERSION 0x0800

// ---------------------------------------------------------------- device / object descriptions
typedef struct DIDEVICEINSTANCEA {
    DWORD   dwSize;
    GUID    guidInstance;
    GUID    guidProduct;
    DWORD   dwDevType;
    CHAR    tszInstanceName[MAX_PATH];
    CHAR    tszProductName[MAX_PATH];
    GUID    guidFFDriver;
    WORD    wUsagePage;
    WORD    wUsage;
} DIDEVICEINSTANCEA, *LPDIDEVICEINSTANCEA;          // 580 bytes
typedef const DIDEVICEINSTANCEA* LPCDIDEVICEINSTANCEA;

typedef struct DIDEVICEOBJECTINSTANCEA {
    DWORD   dwSize;
    GUID    guidType;
    DWORD   dwOfs;
    DWORD   dwType;
    DWORD   dwFlags;
    CHAR    tszName[MAX_PATH];
    DWORD   dwFFMaxForce;
    DWORD   dwFFForceResolution;
    WORD    wCollectionNumber;
    WORD    wDesignatorIndex;
    WORD    wUsagePage;
    WORD    wUsage;
    DWORD   dwDimension;
    WORD    wExponent;
    WORD    wReportId;
} DIDEVICEOBJECTINSTANCEA, *LPDIDEVICEOBJECTINSTANCEA;
typedef const DIDEVICEOBJECTINSTANCEA* LPCDIDEVICEOBJECTINSTANCEA;

typedef struct DIDEVCAPS {
    DWORD   dwSize;
    DWORD   dwFlags;
    DWORD   dwDevType;
    DWORD   dwAxes;
    DWORD   dwButtons;
    DWORD   dwPOVs;
    DWORD   dwFFSamplePeriod;
    DWORD   dwFFMinTimeResolution;
    DWORD   dwFirmwareRevision;
    DWORD   dwHardwareRevision;
    DWORD   dwFFDriverVersion;
} DIDEVCAPS, *LPDIDEVCAPS;                          // 44 bytes

typedef struct DIDEVICEOBJECTDATA {
    DWORD       dwOfs;
    DWORD       dwData;
    DWORD       dwTimeStamp;
    DWORD       dwSequence;
    UINT        uAppData;                           // UINT_PTR in the SDK
} DIDEVICEOBJECTDATA, *LPDIDEVICEOBJECTDATA;        // 20 bytes
typedef const DIDEVICEOBJECTDATA* LPCDIDEVICEOBJECTDATA;

typedef struct DIJOYSTATE {
    LONG    lX, lY, lZ;
    LONG    lRx, lRy, lRz;
    LONG    rglSlider[2];
    DWORD   rgdwPOV[4];
    BYTE    rgbButtons[32];
} DIJOYSTATE, *LPDIJOYSTATE;                        // 80 bytes

typedef struct DIMOUSESTATE {
    LONG    lX, lY, lZ;
    BYTE    rgbButtons[4];
} DIMOUSESTATE, *LPDIMOUSESTATE;                    // 16 bytes

// ---------------------------------------------------------------- properties
typedef struct DIPROPHEADER {
    DWORD   dwSize;
    DWORD   dwHeaderSize;
    DWORD   dwObj;
    DWORD   dwHow;
} DIPROPHEADER, *LPDIPROPHEADER;
typedef const DIPROPHEADER* LPCDIPROPHEADER;

typedef struct DIPROPDWORD {
    DIPROPHEADER diph;
    DWORD        dwData;
} DIPROPDWORD, *LPDIPROPDWORD;

#define MAKEDIPROP(prop)        (*(const GUID*)(prop))
#define DIPROP_BUFFERSIZE       MAKEDIPROP(1)
#define DIPROP_AXISMODE         MAKEDIPROP(2)
#define DIPROP_GRANULARITY      MAKEDIPROP(3)
#define DIPROPAXISMODE_REL      1
#define DIPH_DEVICE             0
#define DIPH_BYOFFSET           1

typedef struct _DIDATAFORMAT {
    DWORD   dwSize;
    DWORD   dwObjSize;
    DWORD   dwFlags;
    DWORD   dwDataSize;
    DWORD   dwNumObjs;
    void*   rgodf;
} DIDATAFORMAT, *LPDIDATAFORMAT;
typedef const DIDATAFORMAT* LPCDIDATAFORMAT;

extern "C" const DIDATAFORMAT c_dfDIJoystick;
extern "C" const DIDATAFORMAT c_dfDIKeyboard;
extern "C" const DIDATAFORMAT c_dfDIMouse;

// ---------------------------------------------------------------- constants
#define DI8DEVCLASS_ALL         0
#define DI8DEVTYPE_MOUSE        0x12
#define DI8DEVTYPE_KEYBOARD     0x13
#define DIDEVTYPE_HID           0x00010000
#define DIEDFL_ATTACHEDONLY     0x00000001
#define DIDFT_ALL               0x00000000
#define DIDFT_AXIS              0x00000003
#define DIDFT_BUTTON            0x0000000C
#define DIENUM_STOP             0
#define DIENUM_CONTINUE         1
#define DISCL_EXCLUSIVE         0x00000001
#define DISCL_NONEXCLUSIVE      0x00000002
#define DISCL_FOREGROUND        0x00000004
#define DISCL_BACKGROUND        0x00000008
#define DISCL_NOWINKEY          0x00000010
#define DIGDD_PEEK              0x00000001
#define DIMOFS_X                0
#define DIMOFS_Y                4
#define DIMOFS_Z                8
#define DIMOFS_BUTTON0          12
#define DIMOFS_BUTTON1          13
#define DIMOFS_BUTTON2          14
#define DI_OK                   S_OK
#define S_FALSE_                ((HRESULT)1L)
#define DI_NOEFFECT             S_FALSE_
#define DIERR_INPUTLOST         ((HRESULT)0x8007001EL)
#define DIERR_OBJECTNOTFOUND    ((HRESULT)0x80070002L)

typedef BOOL (FAR PASCAL* LPDIENUMDEVICESCALLBACKA)(LPCDIDEVICEINSTANCEA, LPVOID);
typedef BOOL (FAR PASCAL* LPDIENUMDEVICEOBJECTSCALLBACKA)(LPCDIDEVICEOBJECTINSTANCEA, LPVOID);

// ---------------------------------------------------------------- interfaces
struct IDirectInputDevice8A : public IUnknown
{
    virtual HRESULT __stdcall GetCapabilities(LPDIDEVCAPS) = 0;                                   // 0x0c
    virtual HRESULT __stdcall EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD) = 0;     // 0x10
    virtual HRESULT __stdcall GetProperty(REFGUID, LPDIPROPHEADER) = 0;                           // 0x14
    virtual HRESULT __stdcall SetProperty(REFGUID, LPCDIPROPHEADER) = 0;                          // 0x18
    virtual HRESULT __stdcall Acquire() = 0;                                                      // 0x1c
    virtual HRESULT __stdcall Unacquire() = 0;                                                    // 0x20
    virtual HRESULT __stdcall GetDeviceState(DWORD, LPVOID) = 0;                                  // 0x24
    virtual HRESULT __stdcall GetDeviceData(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) = 0;     // 0x28
    virtual HRESULT __stdcall SetDataFormat(LPCDIDATAFORMAT) = 0;                                 // 0x2c
    virtual HRESULT __stdcall SetEventNotification(HANDLE) = 0;                                   // 0x30
    virtual HRESULT __stdcall SetCooperativeLevel(HWND, DWORD) = 0;                               // 0x34
    virtual HRESULT __stdcall GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD) = 0;         // 0x38
    virtual HRESULT __stdcall GetDeviceInfo(LPDIDEVICEINSTANCEA) = 0;                             // 0x3c
    virtual HRESULT __stdcall RunControlPanel(HWND, DWORD) = 0;                                   // 0x40
    virtual HRESULT __stdcall Initialize(HINSTANCE, DWORD, REFGUID) = 0;                          // 0x44
    virtual HRESULT __stdcall CreateEffect(REFGUID, LPVOID, LPVOID*, LPUNKNOWN) = 0;              // 0x48
    virtual HRESULT __stdcall EnumEffects(LPVOID, LPVOID, DWORD) = 0;                             // 0x4c
    virtual HRESULT __stdcall GetEffectInfo(LPVOID, REFGUID) = 0;                                 // 0x50
    virtual HRESULT __stdcall GetForceFeedbackState(LPDWORD) = 0;                                 // 0x54
    virtual HRESULT __stdcall SendForceFeedbackCommand(DWORD) = 0;                                // 0x58
    virtual HRESULT __stdcall EnumCreatedEffectObjects(LPVOID, LPVOID, DWORD) = 0;                // 0x5c
    virtual HRESULT __stdcall Escape(LPVOID) = 0;                                                 // 0x60
    virtual HRESULT __stdcall Poll() = 0;                                                         // 0x64
    virtual HRESULT __stdcall SendDeviceData(DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD) = 0;   // 0x68
    virtual HRESULT __stdcall EnumEffectsInFile(LPCSTR, LPVOID, LPVOID, DWORD) = 0;               // 0x6c
    virtual HRESULT __stdcall WriteEffectToFile(LPCSTR, DWORD, LPVOID, DWORD) = 0;                // 0x70
    virtual HRESULT __stdcall BuildActionMap(LPVOID, LPCSTR, DWORD) = 0;                          // 0x74
    virtual HRESULT __stdcall SetActionMap(LPVOID, LPCSTR, DWORD) = 0;                            // 0x78
    virtual HRESULT __stdcall GetImageInfo(LPVOID) = 0;                                           // 0x7c
};
typedef IDirectInputDevice8A* LPDIRECTINPUTDEVICE8A;

struct IDirectInput8A : public IUnknown
{
    virtual HRESULT __stdcall CreateDevice(REFGUID, LPDIRECTINPUTDEVICE8A*, LPUNKNOWN) = 0;       // 0x0c
    virtual HRESULT __stdcall EnumDevices(DWORD, LPDIENUMDEVICESCALLBACKA, LPVOID, DWORD) = 0;    // 0x10
    virtual HRESULT __stdcall GetDeviceStatus(REFGUID) = 0;                                       // 0x14
    virtual HRESULT __stdcall RunControlPanel(HWND, DWORD) = 0;                                   // 0x18
    virtual HRESULT __stdcall Initialize(HINSTANCE, DWORD) = 0;                                   // 0x1c
    virtual HRESULT __stdcall FindDevice(REFGUID, LPCSTR, LPGUID) = 0;                            // 0x20
    virtual HRESULT __stdcall EnumDevicesBySemantics(LPCSTR, LPVOID, LPVOID, LPVOID, DWORD) = 0;  // 0x24
    virtual HRESULT __stdcall ConfigureDevices(LPVOID, LPVOID, DWORD, LPVOID) = 0;                // 0x28
};
typedef IDirectInput8A* LPDIRECTINPUT8A;

extern "C" const GUID IID_IDirectInput8A;
extern "C" const GUID GUID_SysMouse;
extern "C" const GUID GUID_SysKeyboard;

extern "C" HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf,
                                             LPVOID* ppvOut, LPUNKNOWN punkOuter);

#endif
