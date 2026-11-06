// Z:\Toshi\Source\TInput\TInputDeviceKeyboard.cpp  (reconstruction)
#define TINPUT_EXPORTS
#include "TNewDelete.h"
#include "TInputInterface.h"

namespace Toshi {

TOSHI_DEFINE_CLASS_FUNCTIONS(TInputDeviceKeyboard)

TInputDeviceKeyboard::TInputDeviceKeyboard()            // 0x2210
{
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 4; j++)
            m_virtSticks[i].m_d[j] = 0;
    }
}

TInputDeviceKeyboard::~TInputDeviceKeyboard()           // 0x2570
{
}

void TInputDeviceKeyboard::SetVirtStickDoodads(int stick, int a, int b, int c, int d)   // 0x2240
{
    m_virtSticks[stick].m_d[0] = a;
    m_virtSticks[stick].m_d[1] = b;
    m_virtSticks[stick].m_d[2] = c;
    m_virtSticks[stick].m_d[3] = d;
}

void TInputDeviceKeyboard::GetVirtStickDoodads(int stick, int& a, int& b, int& c, int& d)   // 0x2270
{
    a = m_virtSticks[stick].m_d[0];
    b = m_virtSticks[stick].m_d[1];
    c = m_virtSticks[stick].m_d[2];
    d = m_virtSticks[stick].m_d[3];
}

bool TInputDeviceKeyboard::GetDoodadProperties(int doodad, DoodadProperties& props) const   // 0x2510
{
    if (doodad < 0x20000 || doodad >= 0x20080)
        return false;
    props.m_type = 0;
    props.m_flag = false;
    return true;
}

float TInputDeviceKeyboard::GetAxisFloat(int, int) const     // 0x2540
{
    return 0.0f;
}

int TInputDeviceKeyboard::GetAxisInt(int, int) const         // 0x2550
{
    return 0;
}

int TInputDeviceKeyboard::GetAxisCount()                     // 0x2560
{
    return 0;
}

// Keeps last frame's stick values, then rebuilds them from the four key doodads of each virtual stick.
void TInputDeviceKeyboard::UpdateVirtualButtons()            // 0x22b0
{
    m_prevAxisA = m_axisA;
    m_prevAxisB = m_axisB;
    m_axisB = 0.0f;
    m_axisA = 0.0f;
    for (int i = 0; i < 2; i++)
    {
        if (0 < m_virtSticks[i].m_d[0])
        {
            if (IsDown(m_virtSticks[i].m_d[0]))
                m_axisB = -1.0f;
        }
        if (0 < m_virtSticks[i].m_d[1])
        {
            if (IsDown(m_virtSticks[i].m_d[1]))
                m_axisB = 1.0f;
        }
        if (0 < m_virtSticks[i].m_d[2])
        {
            if (IsDown(m_virtSticks[i].m_d[2]))
                m_axisA = -1.0f;
        }
        if (0 < m_virtSticks[i].m_d[3])
        {
            if (IsDown(m_virtSticks[i].m_d[3]))
                m_axisA = 1.0f;
        }
    }
}

// Virtual button -> two physical keys (writable data at 0x61e0, 12 bytes per entry).
struct VirtualButtonKeys { int m_doodad; int m_keyA; int m_keyB; };
static VirtualButtonKeys s_virtualButtons[3] =
{
    { 0x20002, 0x2002e, 0x2003a },
    { 0x20003, 0x20021, 0x2006b },
    { 0x20004, 0x2003c, 0x2006f },
};

int TInputDeviceKeyboard::ProcessVirtualButtons(Emitter& emitter, float)   // 0x2360
{
    TInputInterface::InputEvent stick(this, 0x20001, (TInputInterface::EventType)3, m_axisA, m_axisB);
    emitter.Throw(stick);
    int count = 1;
    for (int i = 0; i < 3; i++)
    {
        int doodad = s_virtualButtons[i].m_doodad;
        int keyA = s_virtualButtons[i].m_keyA;
        int keyB = s_virtualButtons[i].m_keyB;
        if (!WasDown(keyA) && !WasDown(keyB))
        {
            if (IsDown(keyA) || IsDown(keyB))
            {
                TInputInterface::InputEvent down(this, doodad, (TInputInterface::EventType)0);
                emitter.Throw(down);
                count++;
            }
        }
        if (WasDown(keyA) || WasDown(keyB))
        {
            if (!IsDown(keyA) && !IsDown(keyB))
            {
                TInputInterface::InputEvent up(this, doodad, (TInputInterface::EventType)1);
                emitter.Throw(up);
                count++;
            }
        }
    }
    return count;
}

} // namespace Toshi
