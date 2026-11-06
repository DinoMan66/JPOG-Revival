// Z:\Toshi\Source\TInputDX\TInputDXInterface.cpp  (reconstruction)
#define TINPUTDX_EXPORTS
#include "TNewDelete.h"
#include "TInputDXInterface.h"

namespace Toshi {

TInputDXInterface* TInputDXInterface::ms_pTheInterface;

bool TInputDXInterface::Initialise()                        // 0x37f0
{
    if (DirectInput8Create(GetModuleHandleA(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8A,
                           (void**)&m_poDirectInput, NULL) < 0)
        return false;
    m_hMainWindow = FindWindowA("TRenderD3DInterface", NULL);
    if (m_poDirectInput->EnumDevices(DI8DEVCLASS_ALL, EnumerateDeviceCallback, this, DIEDFL_ATTACHEDONLY) != DI_OK)
    {
        Deinitialise();
        return false;
    }
    ms_pTheInterface = this;
    return true;
}

bool TInputDXInterface::Deinitialise()                      // 0x3860
{
    TInputInterface::Deinitialise();
    if (m_poDirectInput)
    {
        m_poDirectInput->Release();
        m_poDirectInput = 0;
    }
    ms_pTheInterface = 0;
    return true;
}

BOOL __stdcall TInputDXInterface::EnumerateDeviceCallback(const DIDEVICEINSTANCEA* instance, void* context)  // 0x3890
{
    TInputDXInterface* self = (TInputDXInterface*)context;
    IDirectInputDevice8A* device;

    switch ((BYTE)instance->dwDevType)
    {
    case DI8DEVTYPE_MOUSE:
    {
        TInputDXDeviceMouse* mouse = self->GetDeviceByIndex<TInputDXDeviceMouse>(0);
        bool created = false;
        if (self->m_poDirectInput->CreateDevice(GUID_SysMouse, &device, NULL) != DI_OK)
            return DIENUM_CONTINUE;
        if (mouse == 0)
        {
            mouse = new TInputDXDeviceMouse;
            if (mouse == 0)
            {
                device->Release();
                return DIENUM_CONTINUE;
            }
            created = true;
        }
        else if (mouse->m_oDeviceInstance.dwDevType & DIDEVTYPE_HID)
        {
            device->Release();
            return DIENUM_CONTINUE;
        }
        if (!mouse->BindToDIDevice(self->m_hMainWindow, instance, device))
        {
            if (!created)
                self->RemoveDevice(mouse);
            if (mouse)
                delete mouse;
            return DIENUM_CONTINUE;
        }
        DIPROPDWORD buffer;
        buffer.diph.dwSize = sizeof(DIPROPDWORD);
        buffer.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        buffer.diph.dwObj = 0;
        buffer.diph.dwHow = DIPH_DEVICE;
        buffer.dwData = TInputDXDeviceMouse::sm_ciMouseBufferSize;
        if (device->SetProperty(DIPROP_BUFFERSIZE, &buffer.diph) < 0)
            return DIENUM_STOP;
        if (created == true)
        {
            mouse->Initialise();
            mouse->Acquire();
            self->AddDevice(mouse);
        }
        return DIENUM_CONTINUE;
    }
    case DI8DEVTYPE_KEYBOARD:
    {
        if (self->m_poDirectInput->CreateDevice(instance->guidInstance, &device, NULL) != DI_OK)
            return DIENUM_CONTINUE;
        TInputDXDeviceKeyboard* keyboard = new TInputDXDeviceKeyboard;
        if (keyboard == 0)
        {
            device->Release();
            return DIENUM_CONTINUE;
        }
        if (!keyboard->BindToDIDevice(self->m_hMainWindow, instance, device))
        {
            if (keyboard)
                delete keyboard;
            return DIENUM_CONTINUE;
        }
        DIPROPDWORD buffer;
        buffer.diph.dwSize = sizeof(DIPROPDWORD);
        buffer.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        buffer.diph.dwObj = 0;
        buffer.diph.dwHow = DIPH_DEVICE;
        buffer.dwData = TInputDXDeviceKeyboard::sm_ciKeyboardBufferSize;
        if (device->SetProperty(DIPROP_BUFFERSIZE, &buffer.diph) < 0)
            return DIENUM_STOP;
        HRESULT hr;
        if (self->GetExclusiveMode() == true)
            hr = device->SetCooperativeLevel(self->m_hMainWindow, DISCL_FOREGROUND | DISCL_EXCLUSIVE | DISCL_NOWINKEY);
        else
            hr = device->SetCooperativeLevel(self->m_hMainWindow, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        if (hr != DI_OK)
        {
            if (keyboard)
                delete keyboard;
            return DIENUM_CONTINUE;
        }
        keyboard->Initialise();
        keyboard->Acquire();
        self->AddDevice(keyboard);
        return DIENUM_CONTINUE;
    }
    }
    return DIENUM_CONTINUE;
}

TOSHI_DEFINE_CLASS(TInputDXInterface, TInputInterface)

IDirectInput8A* TInputDXInterface::GetDirectInput() const   // 0x3ba0
{
    return m_poDirectInput;
}

TInputDXInterface::~TInputDXInterface()                    // 0x3bb0
{
    Deinitialise();
}

TInputDXInterface::TInputDXInterface()                     // 0x3bd0
{
    m_poDirectInput = 0;
    m_hMainWindow = 0;
    ms_pTheInterface = 0;
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TInputDXInterface)

} // namespace Toshi
