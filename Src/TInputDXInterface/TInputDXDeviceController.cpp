// Z:\Toshi\Source\TInputDX\TInputDXDeviceController.cpp  (reconstruction)
#define TINPUTDX_EXPORTS
#include "TNewDelete.h"
#include "TMath.h"
#include <string.h>
#include "TInputDXInterface.h"

namespace Toshi {

const int TInputDXDeviceController::STICK_CENTRE   = 32768;
const int TInputDXDeviceController::STICK_DEADZONE = 7500;
const int TInputDXDeviceController::STICK_SATURATE = 32000;
const int TInputDXDeviceController::STICK_RANGE    = 24500;

TInputDXDeviceController::TInputDXDeviceController()      // 0x1080
{
    m_poDXInputDevice = 0;
    memset(&m_oJoyState, 0, sizeof(DIJOYSTATE));
    memset(&m_oPrevJoyState, 0, sizeof(DIJOYSTATE));
    m_bUpdated = false;
}

bool TInputDXDeviceController::Acquire()                    // 0x1160
{
    m_bUpdated = false;
    if (m_poDXInputDevice->Acquire() >= 0)
    {
        if (!m_acquired)
        {
            TInputDeviceChangeEvent ev(this, TInputDeviceChangeEvent::TYPE_0);
            m_inputInterface->m_emitterB.Throw(ev);
        }
        m_acquired = true;
        return true;
    }
    if (m_acquired)
    {
        TInputDeviceChangeEvent ev(this, TInputDeviceChangeEvent::TYPE_1);
        m_inputInterface->m_emitterB.Throw(ev);
    }
    m_acquired = false;
    return false;
}

bool TInputDXDeviceController::Unacquire()                  // 0x11e0
{
    m_acquired = false;
    m_bUpdated = false;
    if (m_poDXInputDevice->Unacquire() < 0)
        return false;
    return true;
}

void TInputDXDeviceController::Update(float)                // 0x1210
{
    if (m_poDXInputDevice->Poll() < 0)
        return;
    m_oPrevJoyState = m_oJoyState;
    if (m_poDXInputDevice->GetDeviceState(sizeof(DIJOYSTATE), &m_oJoyState) < 0)
        return;
    m_afSticks[0] = NormaliseStick(m_oJoyState.lX);
    m_afSticks[1] = NormaliseStick(m_oJoyState.lY);
    m_afSticks[2] = NormaliseStick(m_oJoyState.lZ);
    m_afSticks[3] = NormaliseStick(m_oJoyState.lRz);
    UpdateVirtualButtons();
    m_bUpdated = true;
}

int TInputDXDeviceController::ProcessEvents(Emitter& emitter, float dt)    // 0x12c0
{
    if (!m_bUpdated)
        return 0;
    int count = ProcessRepeats(emitter, dt);
    count += ProcessVirtualButtons(emitter, dt);
    for (int i = 0; i < 16; i++)
    {
        if ((m_oJoyState.rgbButtons[i] & 0x80) && !(m_oPrevJoyState.rgbButtons[i] & 0x80))
        {
            TInputInterface::InputEvent ev(this, TranslateDXButtonToDoodad(i), TInputInterface::EVENT_0);
            emitter.Throw(ev);
            count++;
        }
        else if (!(m_oJoyState.rgbButtons[i] & 0x80) && (m_oPrevJoyState.rgbButtons[i] & 0x80))
        {
            TInputInterface::InputEvent ev(this, TranslateDXButtonToDoodad(i), TInputInterface::EVENT_1);
            emitter.Throw(ev);
            count++;
        }
    }
    TInputInterface::InputEvent left(this, 0x10019, (TInputInterface::EventType)3, m_afSticks[0], m_afSticks[1]);
    emitter.Throw(left);
    TInputInterface::InputEvent right(this, 0x1001a, (TInputInterface::EventType)3, m_afSticks[2], m_afSticks[3]);
    emitter.Throw(right);
    return count + 2;
}

bool TInputDXDeviceController::IsDown(int doodad) const     // 0x1410
{
    int button = TranslateDoodadToDXButton(doodad);
    if (button < 0)
        return TInputDeviceController::IsDown(doodad);
    return (m_oJoyState.rgbButtons[button] & 0x80) != 0;
}

bool TInputDXDeviceController::WasDown(int doodad) const    // 0x1460
{
    int button = TranslateDoodadToDXButton(doodad);
    if (button < 0)
        return TInputDeviceController::WasDown(doodad);
    return (m_oPrevJoyState.rgbButtons[button] & 0x80) != 0;
}

int TInputDXDeviceController::GetAxisInt(int doodad, int axis) const       // 0x14b0
{
    // NOT MATCHING YET: the binary dispatches with je to both cases and keeps "return 0" inline
    // (see notes); every switch/if form tried so far compiles to a jne chain.
    int value;
    switch (doodad)
    {
    case 0x10019:
        if (axis == 0)      value = m_oJoyState.lX - 32000;
        else if (axis == 1) value = m_oJoyState.lY - 32000;
        else return 0;
        break;
    case 0x1001a:
        if (axis == 0)      value = m_oJoyState.lZ - 32000;
        else if (axis == 1) value = m_oJoyState.lRz - 32000;
        else return 0;
        break;
    default:
        return 0;
    }
    if (value <= STICK_DEADZONE && value >= -STICK_DEADZONE)
        return 0;
    return value;
}

const bool TInputDXDeviceController::BindToDIDevice(HWND, const DIDEVICEINSTANCEA* instance, IDirectInputDevice8A* device)  // 0x1520
{
    Initialise();
    m_oDeviceInstance = *instance;
    m_poDXInputDevice = device;
    m_oDIDevCaps.dwSize = sizeof(DIDEVCAPS);
    device->GetCapabilities(&m_oDIDevCaps);
    if (m_poDXInputDevice->SetDataFormat(&c_dfDIJoystick) != DI_OK)
        return false;
    m_poDXInputDevice->EnumObjects(EnumerateObjectCallback, this, DIDFT_AXIS | DIDFT_BUTTON);
    m_bUpdated = false;
    return true;
}

int TInputDXDeviceController::TranslateDXButtonToDoodad(int button) const  // 0x15b0
{
    switch (button)
    {
        case 0x0:
            return 0x10009;
        case 0x1:
            return 0x1000a;
        case 0x2:
            return 0x1000b;
        case 0x3:
            return 0x1000c;
        case 0x4:
            return 0x1000e;
        case 0x5:
            return 0x10010;
        case 0x6:
            return 0x1000d;
        case 0x7:
            return 0x1000f;
        case 0x8:
            return 0x10005;
        case 0x9:
            return 0x10007;
        case 0xa:
            return 0x10008;
        case 0xb:
            return 0x10006;
        case 0xc:
            return 0x10001;
        case 0xd:
            return 0x10002;
        case 0xe:
            return 0x10003;
        case 0xf:
            return 0x10004;
    }
    return 0;
}

int TInputDXDeviceController::TranslateDoodadToDXButton(int doodad) const  // 0x1650
{
    switch (doodad)
    {
        case 0x10001:
            return 0xc;
        case 0x10002:
            return 0xd;
        case 0x10003:
            return 0xe;
        case 0x10004:
            return 0xf;
        case 0x10005:
            return 0x8;
        case 0x10006:
            return 0xb;
        case 0x10007:
            return 0x9;
        case 0x10008:
            return 0xa;
        case 0x10009:
            return 0;
        case 0x1000a:
            return 0x1;
        case 0x1000b:
            return 0x2;
        case 0x1000c:
            return 0x3;
        case 0x1000d:
            return 0x6;
        case 0x1000e:
            return 0x4;
        case 0x1000f:
            return 0x7;
        case 0x10010:
            return 0x5;
    }
    return -1;
}

BOOL __stdcall TInputDXDeviceController::EnumerateObjectCallback(const DIDEVICEOBJECTINSTANCEA*, void*)   // 0x1700
{
    return DIENUM_CONTINUE;
}

float __stdcall TInputDXDeviceController::NormaliseStick(int value)       // 0x1710
{
    int v = value - STICK_CENTRE;
    if (v >= 0)
    {
        v = TMath::Max(v, STICK_DEADZONE);
        v = TMath::Min(v, STICK_SATURATE);
        v = v - STICK_DEADZONE;
    }
    else
    {
        v = TMath::Min(v, -STICK_DEADZONE);
        v = TMath::Max(v, -STICK_SATURATE);
        v = v + STICK_DEADZONE;
    }
    return v * (1.0f / STICK_RANGE);
}

TOSHI_DEFINE_CLASS(TInputDXDeviceController, TInputDeviceController)

float TInputDXDeviceController::GetAxisFloat(int doodad, int axis) const   // 0x1810
{
    return GetAxisInt(doodad, axis) * (1.0f / 32000.0f);
}

int TInputDXDeviceController::GetAxisCount()                // 0x1840
{
    return 2;
}

int TInputDXDeviceController::GetButtonCount()              // 0x1850
{
    return 12;
}

void TInputDXDeviceController::Release()                    // 0x1860
{
    if (m_poDXInputDevice)
    {
        m_poDXInputDevice->Release();
        m_poDXInputDevice = 0;
    }
}

bool TInputDXDeviceController::Deinitialise()               // 0x1880
{
    return true;
}

bool TInputDXDeviceController::Initialise()                 // 0x1890
{
    return true;
}

TInputDXDeviceController::~TInputDXDeviceController()      // 0x18a0
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TInputDXDeviceController)

} // namespace Toshi
