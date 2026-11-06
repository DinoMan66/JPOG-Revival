// Z:\Toshi\Source\TInputDX\TInputDXDeviceKeyboard.cpp  (reconstruction)
#define TINPUTDX_EXPORTS
#include "TNewDelete.h"
#include <string.h>
#include "TInputDXInterface.h"

namespace Toshi {

TInputDXDeviceKeyboard::TInputDXDeviceKeyboard()            // 0x1a40
    : m_iBuffer(0), m_pcCurrentState(0), m_pcPreviousState(0),
      m_poDXInputDevice(0), m_bRefreshed(false)
{
}

bool TInputDXDeviceKeyboard::Initialise()                   // 0x1a90
{
    m_iBuffer = 0;
    for (int i = 0; i < 256; i++)
    {
        m_acBuffers[0][i] = 0;
        m_acBuffers[1][i] = 0;
    }
    m_pcCurrentState = m_acBuffers[m_iBuffer];
    m_pcPreviousState = m_acBuffers[1 - m_iBuffer];
    return true;
}

bool TInputDXDeviceKeyboard::Acquire()                      // 0x1b00
{
    m_bUpdated = false;
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

bool TInputDXDeviceKeyboard::Unacquire()                    // 0x1b50
{
    m_bUpdated = false;
    m_acquired = false;
    if (m_poDXInputDevice->Unacquire() < 0)
        return false;
    return true;
}

bool TInputDXDeviceKeyboard::Flush()                        // 0x1b80
{
    if (!m_acquired)
        return false;
    DWORD count = INFINITE;
    if (m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &count, 0) < 0)
    {
        m_bUpdated = false;
        return false;
    }
    return true;
}

void TInputDXDeviceKeyboard::RefreshDirect()                // 0x1bd0
{
    if (!m_acquired)
        return;
    for (int i = 0; i < 256; i++)
        m_pcPreviousState[i] = m_pcCurrentState[i];
    if (m_poDXInputDevice->Poll() < 0)
    {
        m_bUpdated = false;
        return;
    }
    if (m_poDXInputDevice->GetDeviceState(256, m_pcCurrentState) != DI_OK)
    {
        m_bUpdated = false;
        return;
    }
    DWORD count = 32;
    m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &count, 0);
    m_bRefreshed = true;
    m_bUpdated = true;
}

int TInputDXDeviceKeyboard::ProcessEvents(Emitter& emitter, float dt)      // 0x1cb0
{
    if (!m_bUpdated)
        return 0;
    if (m_bRefreshed)
    {
        for (int i = 0; i < 256; i++)
        {
            if (m_pcCurrentState[i] != m_pcPreviousState[i])
                HandleKeyChange(emitter, i, m_pcCurrentState[i]);
        }
        m_bRefreshed = false;
    }
    DWORD count = 32;
    DIDEVICEOBJECTDATA data[32];
    memset(data, 0, sizeof(data));
    if (m_poDXInputDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), data, &count, 0) < 0)
    {
        m_bUpdated = false;
        Unacquire();
        Acquire();
        return 0;
    }
    int events = ProcessRepeats(emitter, dt);
    events += ProcessVirtualButtons(emitter, dt);
    for (DWORD j = 0; j < count; j++)
    {
        if (HandleKeyChange(emitter, data[j].dwOfs, (unsigned char)data[j].dwData))
            events++;
    }
    UpdateVirtualButtons();
    return events;
}


bool TInputDXDeviceKeyboard::IsDown(int doodad) const       // 0x1e20
{
    switch (doodad)
    {
    case 0x20002:
        return m_bUpdated && IsShiftDown();
    case 0x20003:
        return m_bUpdated && IsControlDown();
    case 0x20004:
        return m_bUpdated && IsAltDown();
    }
    int key = TranslateDoodadToDX(doodad);
    if (m_bUpdated && (m_pcCurrentState[key] & 0x80))
        return true;
    return false;
}

bool TInputDXDeviceKeyboard::WasDown(int doodad) const      // 0x1f00
{
    switch (doodad)
    {
    case 0x20002:
        return m_bUpdated && ((m_pcPreviousState[0x2a] & 0x80) && (m_pcPreviousState[0x36] & 0x80));
    case 0x20003:
        return m_bUpdated && ((m_pcPreviousState[0x1d] & 0x80) && (m_pcPreviousState[0x9d] & 0x80));
    case 0x20004:
        return m_bUpdated && ((m_pcPreviousState[0x38] & 0x80) && (m_pcPreviousState[0xb8] & 0x80));
    }
    int key = TranslateDoodadToDX(doodad);
    if (m_bUpdated && (m_pcPreviousState[key] & 0x80))
        return true;
    return false;
}

bool TInputDXDeviceKeyboard::IsShiftDown() const            // 0x1ff0
{
    if ((m_pcCurrentState[0x2a] | m_pcCurrentState[0x36]) & 0x80)
        return true;
    return false;
}

bool TInputDXDeviceKeyboard::IsControlDown() const          // 0x2010
{
    if ((m_pcCurrentState[0x1d] | m_pcCurrentState[0x9d]) & 0x80)
        return true;
    return false;
}

bool TInputDXDeviceKeyboard::IsAltDown() const              // 0x2040
{
    if ((m_pcCurrentState[0x38] | m_pcCurrentState[0xb8]) & 0x80)
        return true;
    return false;
}

TLString TInputDXDeviceKeyboard::GetLocalisedKeyName(int doodad) const     // 0x2070
{
    if (doodad >= 0x20000 && doodad < 0x20080)
        return m_aKeyNames[doodad - 0x20000];
    return TLString();
}

void TInputDXDeviceKeyboard::ClearAll()                     // 0x20b0
{
    for (int i = 0; i < 256; i++)
        m_pcCurrentState[i] = 0;
}

bool TInputDXDeviceKeyboard::HandleKeyChange(Emitter& emitter, int key, unsigned char state)  // 0x20e0
{
    int doodad = TranslateDXToDoodad(key);
    if (doodad == 0x20013 && IsAltDown())
        return false;
    if (state & 0x80)
    {
        m_pcCurrentState[key] = (char)0x80;
        m_pcPreviousState[key] = 0;
        TInputInterface::InputEvent ev(this, doodad, TInputInterface::EVENT_0);
        emitter.Throw(ev);
    }
    else
    {
        m_pcCurrentState[key] = 0;
        m_pcPreviousState[key] = (char)0x80;
        TInputInterface::InputEvent ev(this, doodad, TInputInterface::EVENT_1);
        emitter.Throw(ev);
    }
    return true;
}

int __stdcall TInputDXDeviceKeyboard::TranslateDoodadToDX(int doodad)    // 0x21a0
{
    switch (doodad)
    {
        case 0x20005:
            return 0x1;
        case 0x20006:
            return 0x2;
        case 0x20007:
            return 0x3;
        case 0x20008:
            return 0x4;
        case 0x20009:
            return 0x5;
        case 0x2000a:
            return 0x6;
        case 0x2000b:
            return 0x7;
        case 0x2000c:
            return 0x8;
        case 0x2000d:
            return 0x9;
        case 0x2000e:
            return 0xa;
        case 0x2000f:
            return 0xb;
        case 0x20010:
            return 0xc;
        case 0x20011:
            return 0xd;
        case 0x20012:
            return 0xe;
        case 0x20013:
            return 0xf;
        case 0x20014:
            return 0x10;
        case 0x20015:
            return 0x11;
        case 0x20016:
            return 0x12;
        case 0x20017:
            return 0x13;
        case 0x20018:
            return 0x14;
        case 0x20019:
            return 0x15;
        case 0x2001a:
            return 0x16;
        case 0x2001b:
            return 0x17;
        case 0x2001c:
            return 0x18;
        case 0x2001d:
            return 0x19;
        case 0x2001e:
            return 0x1a;
        case 0x2001f:
            return 0x1b;
        case 0x20020:
            return 0x1c;
        case 0x20021:
            return 0x1d;
        case 0x20022:
            return 0x1e;
        case 0x20023:
            return 0x1f;
        case 0x20024:
            return 0x20;
        case 0x20025:
            return 0x21;
        case 0x20026:
            return 0x22;
        case 0x20027:
            return 0x23;
        case 0x20028:
            return 0x24;
        case 0x20029:
            return 0x25;
        case 0x2002a:
            return 0x26;
        case 0x2002b:
            return 0x27;
        case 0x2002c:
            return 0x28;
        case 0x2002d:
            return 0x29;
        case 0x2002e:
            return 0x2a;
        case 0x2002f:
            return 0x2b;
        case 0x20030:
            return 0x2c;
        case 0x20031:
            return 0x2d;
        case 0x20032:
            return 0x2e;
        case 0x20033:
            return 0x2f;
        case 0x20034:
            return 0x30;
        case 0x20035:
            return 0x31;
        case 0x20036:
            return 0x32;
        case 0x20037:
            return 0x33;
        case 0x20038:
            return 0x34;
        case 0x20039:
            return 0x35;
        case 0x2003a:
            return 0x36;
        case 0x2003b:
            return 0x37;
        case 0x2003c:
            return 0x38;
        case 0x2003d:
            return 0x39;
        case 0x2003e:
            return 0x3a;
        case 0x2003f:
            return 0x3b;
        case 0x20040:
            return 0x3c;
        case 0x20041:
            return 0x3d;
        case 0x20042:
            return 0x3e;
        case 0x20043:
            return 0x3f;
        case 0x20044:
            return 0x40;
        case 0x20045:
            return 0x41;
        case 0x20046:
            return 0x42;
        case 0x20047:
            return 0x43;
        case 0x20048:
            return 0x44;
        case 0x20049:
            return 0x45;
        case 0x2004a:
            return 0x46;
        case 0x2004b:
            return 0x47;
        case 0x2004c:
            return 0x48;
        case 0x2004d:
            return 0x49;
        case 0x2004e:
            return 0x4a;
        case 0x2004f:
            return 0x4b;
        case 0x20050:
            return 0x4c;
        case 0x20051:
            return 0x4d;
        case 0x20052:
            return 0x4e;
        case 0x20053:
            return 0x4f;
        case 0x20054:
            return 0x50;
        case 0x20055:
            return 0x51;
        case 0x20056:
            return 0x52;
        case 0x20057:
            return 0x53;
        case 0x20058:
            return 0x57;
        case 0x20059:
            return 0x58;
        case 0x2005a:
            return 0x64;
        case 0x2005b:
            return 0x65;
        case 0x2005c:
            return 0x66;
        case 0x2005d:
            return 0x70;
        case 0x2005e:
            return 0x79;
        case 0x2005f:
            return 0x7b;
        case 0x20060:
            return 0x7d;
        case 0x20061:
            return 0x8d;
        case 0x20062:
            return 0x90;
        case 0x20063:
            return 0x91;
        case 0x20064:
            return 0x92;
        case 0x20065:
            return 0x93;
        case 0x20066:
            return 0x94;
        case 0x20067:
            return 0x95;
        case 0x20068:
            return 0x96;
        case 0x20069:
            return 0x97;
        case 0x2006a:
            return 0x9c;
        case 0x2006b:
            return 0x9d;
        case 0x2006c:
            return 0xb3;
        case 0x2006d:
            return 0xb5;
        case 0x2006e:
            return 0xb7;
        case 0x2006f:
            return 0xb8;
        case 0x20070:
            return 0xc7;
        case 0x20071:
            return 0xc5;
        case 0x20072:
            return 0xc8;
        case 0x20073:
            return 0xc9;
        case 0x20074:
            return 0xcb;
        case 0x20075:
            return 0xcd;
        case 0x20076:
            return 0xcf;
        case 0x20077:
            return 0xd0;
        case 0x20078:
            return 0xd1;
        case 0x20079:
            return 0xd2;
        case 0x2007a:
            return 0xd3;
        case 0x2007b:
            return 0xdb;
        case 0x2007c:
            return 0xdc;
        case 0x2007d:
            return 0xdd;
        case 0x2007e:
            return 0xde;
        case 0x2007f:
            return 0xdf;
    }
    return 0;
}

int __stdcall TInputDXDeviceKeyboard::TranslateDXToDoodad(int key)       // 0x25a0
{
    switch (key)
    {
        case 0x1:
            return 0x20005;
        case 0x2:
            return 0x20006;
        case 0x3:
            return 0x20007;
        case 0x4:
            return 0x20008;
        case 0x5:
            return 0x20009;
        case 0x6:
            return 0x2000a;
        case 0x7:
            return 0x2000b;
        case 0x8:
            return 0x2000c;
        case 0x9:
            return 0x2000d;
        case 0xa:
            return 0x2000e;
        case 0xb:
            return 0x2000f;
        case 0xc:
            return 0x20010;
        case 0xd:
            return 0x20011;
        case 0xe:
            return 0x20012;
        case 0xf:
            return 0x20013;
        case 0x10:
            return 0x20014;
        case 0x11:
            return 0x20015;
        case 0x12:
            return 0x20016;
        case 0x13:
            return 0x20017;
        case 0x14:
            return 0x20018;
        case 0x15:
            return 0x20019;
        case 0x16:
            return 0x2001a;
        case 0x17:
            return 0x2001b;
        case 0x18:
            return 0x2001c;
        case 0x19:
            return 0x2001d;
        case 0x1a:
            return 0x2001e;
        case 0x1b:
            return 0x2001f;
        case 0x1c:
            return 0x20020;
        case 0x1d:
            return 0x20021;
        case 0x1e:
            return 0x20022;
        case 0x1f:
            return 0x20023;
        case 0x20:
            return 0x20024;
        case 0x21:
            return 0x20025;
        case 0x22:
            return 0x20026;
        case 0x23:
            return 0x20027;
        case 0x24:
            return 0x20028;
        case 0x25:
            return 0x20029;
        case 0x26:
            return 0x2002a;
        case 0x27:
            return 0x2002b;
        case 0x28:
            return 0x2002c;
        case 0x29:
            return 0x2002d;
        case 0x2a:
            return 0x2002e;
        case 0x2b:
            return 0x2002f;
        case 0x2c:
            return 0x20030;
        case 0x2d:
            return 0x20031;
        case 0x2e:
            return 0x20032;
        case 0x2f:
            return 0x20033;
        case 0x30:
            return 0x20034;
        case 0x31:
            return 0x20035;
        case 0x32:
            return 0x20036;
        case 0x33:
            return 0x20037;
        case 0x34:
            return 0x20038;
        case 0x35:
            return 0x20039;
        case 0x36:
            return 0x2003a;
        case 0x37:
            return 0x2003b;
        case 0x38:
            return 0x2003c;
        case 0x39:
            return 0x2003d;
        case 0x3a:
            return 0x2003e;
        case 0x3b:
            return 0x2003f;
        case 0x3c:
            return 0x20040;
        case 0x3d:
            return 0x20041;
        case 0x3e:
            return 0x20042;
        case 0x3f:
            return 0x20043;
        case 0x40:
            return 0x20044;
        case 0x41:
            return 0x20045;
        case 0x42:
            return 0x20046;
        case 0x43:
            return 0x20047;
        case 0x44:
            return 0x20048;
        case 0x45:
            return 0x20049;
        case 0x46:
            return 0x2004a;
        case 0x47:
            return 0x2004b;
        case 0x48:
            return 0x2004c;
        case 0x49:
            return 0x2004d;
        case 0x4a:
            return 0x2004e;
        case 0x4b:
            return 0x2004f;
        case 0x4c:
            return 0x20050;
        case 0x4d:
            return 0x20051;
        case 0x4e:
            return 0x20052;
        case 0x4f:
            return 0x20053;
        case 0x50:
            return 0x20054;
        case 0x51:
            return 0x20055;
        case 0x52:
            return 0x20056;
        case 0x53:
            return 0x20057;
        case 0x57:
            return 0x20058;
        case 0x58:
            return 0x20059;
        case 0x64:
            return 0x2005a;
        case 0x65:
            return 0x2005b;
        case 0x66:
            return 0x2005c;
        case 0x70:
            return 0x2005d;
        case 0x79:
            return 0x2005e;
        case 0x7b:
            return 0x2005f;
        case 0x7d:
            return 0x20060;
        case 0x8d:
            return 0x20061;
        case 0x90:
            return 0x20062;
        case 0x91:
            return 0x20063;
        case 0x92:
            return 0x20064;
        case 0x93:
            return 0x20065;
        case 0x94:
            return 0x20066;
        case 0x95:
            return 0x20067;
        case 0x96:
            return 0x20068;
        case 0x97:
            return 0x20069;
        case 0x9c:
            return 0x2006a;
        case 0x9d:
            return 0x2006b;
        case 0xb3:
            return 0x2006c;
        case 0xb5:
            return 0x2006d;
        case 0xb7:
            return 0x2006e;
        case 0xb8:
            return 0x2006f;
        case 0xc5:
            return 0x20071;
        case 0xc7:
            return 0x20070;
        case 0xc8:
            return 0x20072;
        case 0xc9:
            return 0x20073;
        case 0xcb:
            return 0x20074;
        case 0xcd:
            return 0x20075;
        case 0xcf:
            return 0x20076;
        case 0xd0:
            return 0x20077;
        case 0xd1:
            return 0x20078;
        case 0xd2:
            return 0x20079;
        case 0xd3:
            return 0x2007a;
        case 0xdb:
            return 0x2007b;
        case 0xdc:
            return 0x2007c;
        case 0xdd:
            return 0x2007d;
        case 0xde:
            return 0x2007e;
        case 0xdf:
            return 0x2007f;
    }
    return 0;
}

const bool TInputDXDeviceKeyboard::BindToDIDevice(HWND hwnd, const DIDEVICEINSTANCEA* instance, IDirectInputDevice8A* device)  // 0x29a0
{
    Initialise();
    Release();
    m_oDeviceInstance = *instance;
    m_poDXInputDevice = device;
    m_oDIDevCaps.dwSize = sizeof(DIDEVCAPS);
    device->GetCapabilities(&m_oDIDevCaps);
    if (m_poDXInputDevice->SetDataFormat(&c_dfDIKeyboard) != DI_OK)
        return false;
    if (hwnd && m_poDXInputDevice->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND) != DI_OK)
        return false;
    m_bUpdated = false;
    m_poDXInputDevice->EnumObjects(EnumObjectCallback, this, DIDFT_BUTTON);
    return true;
}

BOOL __stdcall TInputDXDeviceKeyboard::EnumObjectCallback(const DIDEVICEOBJECTINSTANCEA* object, void* context)   // 0x2a60
{
    TInputDXDeviceKeyboard* keyboard = (TInputDXDeviceKeyboard*)context;
    int doodad = TranslateDXToDoodad(object->dwOfs);
    if (doodad >= 0x20000 && doodad < 0x20080)
        keyboard->m_aKeyNames[doodad - 0x20000] = TSystem::GetLocale()->GetTemporary(TWString(object->tszName));
    return DIENUM_CONTINUE;
}

TOSHI_DEFINE_CLASS(TInputDXDeviceKeyboard, TInputDeviceKeyboard)

void TInputDXDeviceKeyboard::SetUp(int key)                 // 0x2ba0
{
    m_pcCurrentState[key] = 0;
    m_pcPreviousState[key] = (char)0x80;
}

void TInputDXDeviceKeyboard::SetDown(int key)               // 0x2bc0
{
    m_pcCurrentState[key] = (char)0x80;
    m_pcPreviousState[key] = 0;
}

void TInputDXDeviceKeyboard::Clear(int key)                 // 0x2be0
{
    m_pcCurrentState[key] = 0;
}

unsigned short TInputDXDeviceKeyboard::TranslateDoodadToCharacter(int) const   // 0x2bf0
{
    return 0;
}

int TInputDXDeviceKeyboard::GetButtonCount()                // 0x2c00
{
    return 0;
}

void TInputDXDeviceKeyboard::Update(float)                  // 0x2c10
{
    if (!m_acquired)
        return;
    m_bUpdated = true;
}

void TInputDXDeviceKeyboard::Release()                      // 0x2c30
{
    if (m_poDXInputDevice)
    {
        m_poDXInputDevice->Release();
        m_poDXInputDevice = 0;
    }
}

bool TInputDXDeviceKeyboard::Deinitialise()                 // 0x2c50
{
    Release();
    return true;
}

TInputDXDeviceKeyboard::~TInputDXDeviceKeyboard()          // 0x2c60
{
    Deinitialise();
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TInputDXDeviceKeyboard)

const int TInputDXDeviceKeyboard::sm_ciKeyboardBufferSize = 32;

} // namespace Toshi
