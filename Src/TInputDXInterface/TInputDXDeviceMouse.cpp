// Z:\Toshi\Source\TInputDX\TInputDXDeviceMouse.cpp  (reconstruction)
#define TINPUTDX_EXPORTS
#include "TNewDelete.h"
#include <string.h>
#include "TInputDXInterface.h"

namespace Toshi {

bool TInputDXDeviceMouse::Initialise()                      // 0x2dd0
{
    m_bSwapButtons = false;
    m_iButtons = 0;
    m_iPrevButtons = 0;
    m_iX = 0;
    m_iY = 0;
    m_iZ = 0;
    m_iDX = 0;
    m_iDY = 0;
    m_iDZ = 0;
    m_fUnk60[0] = 0.0f;
    m_fUnk60[1] = 0.0f;
    m_fUnk60[2] = 0.0f;
    m_iUnk6c = 0;
    m_iUnk70 = 0;
    m_iUnk74 = -1;
    m_iUnk78 = -1;

    DIPROPDWORD granularity;
    granularity.diph.dwSize = sizeof(DIPROPDWORD);
    granularity.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    granularity.diph.dwObj = DIMOFS_Z;
    granularity.diph.dwHow = DIPH_BYOFFSET;
    HRESULT hr = m_poDXInputDevice->GetProperty(DIPROP_GRANULARITY, &granularity.diph);
    switch (hr)
    {
    case DIERR_OBJECTNOTFOUND: m_iWheelGranularity = 0; break;
    case DI_OK:                m_iWheelGranularity = granularity.dwData; break;
    default:                   m_iWheelGranularity = 0; break;
    }
    return true;
}

bool TInputDXDeviceMouse::Acquire()                         // 0x2e80
{
    HRESULT hr = m_poDXInputDevice->Acquire();
    if (hr < 0)
    {
        m_acquired = false;
        return false;
    }
    if (hr == DI_NOEFFECT)
    {
        m_acquired = true;
        return true;
    }
    m_acquired = true;
    RefreshDirect();
    return true;
}

bool TInputDXDeviceMouse::Flush()                           // 0x2ec0
{
    if (!m_acquired)
        return false;
    DWORD count = INFINITE;
    if (m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &count, 0) < 0)
        return false;
    return true;
}

void TInputDXDeviceMouse::RefreshDirect()                   // 0x2f00
{
    if (!m_acquired)
        return;
    m_iButtons = 0;
    m_iPrevButtons = 0;
    unsigned char state[16];
    memset(state, 0, sizeof(state));
    if (m_poDXInputDevice->Poll() < 0)
        return;
    if (m_poDXInputDevice->GetDeviceState(sizeof(DIMOUSESTATE), state) < 0)
        return;
    if (state[0] & 0x80)
    {
        if (m_bSwapButtons) m_iButtons |= 2;
        else                m_iButtons |= 1;
    }
    if (state[1] & 0x80)
    {
        if (m_bSwapButtons) m_iButtons |= 1;
        else                m_iButtons |= 2;
    }
    if (state[2] & 0x80)
        m_iButtons |= 4;
}

void TInputDXDeviceMouse::Update(float)                     // 0x2fd0
{
    if (!m_acquired)
        return;
    m_iDX = 0;
    m_iDY = 0;
    m_iDZ = 0;
    if (m_poDXInputDevice->Poll() < 0)
        return;
    DIMOUSESTATE state;
    if (m_poDXInputDevice->GetDeviceState(sizeof(DIMOUSESTATE), &state) < 0)
        return;
    m_iDX += state.lX;
    m_iX += state.lX;
    m_iDY += state.lY;
    m_iY += state.lY;
    m_iDZ += state.lZ;
    m_iZ += state.lZ;
}

int TInputDXDeviceMouse::ProcessEvents(Emitter& emitter, float dt)         // 0x3040
{
    m_iPrevButtons = m_iButtons;
    int events = ProcessRepeats(emitter, dt);
    DWORD count = 32;
    DIDEVICEOBJECTDATA data[32];
    memset(data, 0, sizeof(data));
    HRESULT hr = m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), data, &count, 0);
    if (hr < 0 && hr == DIERR_INPUTLOST)
    {
        Unacquire();
        Acquire();
        if (m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), data, &count, 0) != DI_OK)
            return 0;
    }
    for (DWORD i = 0; i < count; i++)
    {
        int button = -1;
        switch (data[i].dwOfs)
        {
        case DIMOFS_BUTTON0: button = m_bSwapButtons ? 1 : 0; break;
        case DIMOFS_BUTTON1: button = m_bSwapButtons ? 0 : 1; break;
        case DIMOFS_BUTTON2: button = 2; break;
        }
        if (button >= 0)
        {
            if (data[i].dwData & 0x80)
            {
                TInputInterface::InputEvent ev(this, 0x30001 + button, TInputInterface::EVENT_0);
                emitter.Throw(ev);
                events++;
                m_iButtons |= 1 << button;
            }
            else
            {
                TInputInterface::InputEvent ev(this, 0x30001 + button, TInputInterface::EVENT_1);
                emitter.Throw(ev);
                events++;
                m_iButtons &= ~(1 << button);
            }
        }
    }
    if (m_iDX != 0 || m_iDY != 0)
    {
        emitter.Throw(TInputInterface::InputEvent(this, 0x3000b, (TInputInterface::EventType)4, m_iDX, m_iDY));
        events++;
    }
    if (m_iDZ != 0)
    {
        emitter.Throw(TInputInterface::InputEvent(this, 0x3000c, (TInputInterface::EventType)4, m_iDZ * (1.0f / 120.0f), 0.0f));
        events++;
        int clicks = m_iDZ / m_iWheelGranularity;
        if (clicks < 0)
        {
            events += 1 - clicks;
            emitter.Throw(TInputInterface::InputEvent(this, 0x3000a, TInputInterface::EVENT_0));
            while (++clicks < 0)
                emitter.Throw(TInputInterface::InputEvent(this, 0x3000a, TInputInterface::EVENT_2));
            emitter.Throw(TInputInterface::InputEvent(this, 0x3000a, TInputInterface::EVENT_1));
        }
        if (clicks > 0)
        {
            events += clicks + 1;
            emitter.Throw(TInputInterface::InputEvent(this, 0x30009, TInputInterface::EVENT_0));
            while (--clicks > 0)
                emitter.Throw(TInputInterface::InputEvent(this, 0x30009, TInputInterface::EVENT_2));
            emitter.Throw(TInputInterface::InputEvent(this, 0x30009, TInputInterface::EVENT_1));
        }
    }
    return events;
}

const bool TInputDXDeviceMouse::BindToDIDevice(HWND hwnd, const DIDEVICEINSTANCEA* instance, IDirectInputDevice8A* device)  // 0x3420
{
    Release();
    m_oDeviceInstance = *instance;
    m_poDXInputDevice = device;
    m_oDIDevCaps.dwSize = sizeof(DIDEVCAPS);
    device->GetCapabilities(&m_oDIDevCaps);
    if (m_poDXInputDevice->SetDataFormat(&c_dfDIMouse) != DI_OK)
        return false;
    if (hwnd)
        m_poDXInputDevice->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
    DIPROPDWORD axisMode;
    axisMode.diph.dwSize = sizeof(DIPROPDWORD);
    axisMode.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    axisMode.diph.dwObj = 0;
    axisMode.diph.dwHow = DIPH_DEVICE;
    axisMode.dwData = DIPROPAXISMODE_REL;
    if (m_poDXInputDevice->SetProperty(DIPROP_AXISMODE, &axisMode.diph) != DI_OK)
        return false;
    return true;
}

TOSHI_DEFINE_CLASS(TInputDXDeviceMouse, TInputDeviceMouse)

float TInputDXDeviceMouse::GetAxisFloat(int doodad, int axis) const
{
    if (doodad == 0x3000b)
    {
        if (axis == 0) return (float)m_iDX;
        if (axis == 1) return (float)m_iDY;
    }
    return 0.0f;
}

int TInputDXDeviceMouse::GetAxisInt(int doodad, int axis) const
{
    if (doodad == 0x3000b)
    {
        if (axis == 0) return m_iDX;
        if (axis == 1) return m_iDY;
    }
    return 0;
}

bool TInputDXDeviceMouse::WasDown(int doodad) const         // 0x35b0
{
    if (doodad >= 0x30001 && doodad <= 0x30008)
        return (m_iPrevButtons & (1 << (doodad - 0x30001))) != 0;
    return false;
}

bool TInputDXDeviceMouse::IsDown(int doodad) const          // 0x35f0
{
    if (doodad >= 0x30001 && doodad <= 0x30008)
        return (m_iButtons & (1 << (doodad - 0x30001))) != 0;
    return false;
}

int TInputDXDeviceMouse::GetAxisCount()                     // 0x3630
{
    return m_oDIDevCaps.dwAxes;
}

int TInputDXDeviceMouse::GetButtonCount()                   // 0x3640
{
    return m_oDIDevCaps.dwButtons;
}

void TInputDXDeviceMouse::Release()                         // 0x3650
{
    if (m_poDXInputDevice)
    {
        m_poDXInputDevice->Release();
        m_poDXInputDevice = 0;
    }
}

bool TInputDXDeviceMouse::Unacquire()                       // 0x3670
{
    m_acquired = false;
    if (m_poDXInputDevice->Unacquire() < 0)
        return false;
    return true;
}

bool TInputDXDeviceMouse::Deinitialise()                    // 0x3690
{
    Release();
    return true;
}

TInputDXDeviceMouse::~TInputDXDeviceMouse()                // 0x36a0
{
    Deinitialise();
}

TInputDXDeviceMouse::TInputDXDeviceMouse()                 // 0x36c0
{
    m_poDXInputDevice = 0;
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TInputDXDeviceMouse)

const int TInputDXDeviceMouse::sm_ciMouseBufferSize = 32;

} // namespace Toshi
