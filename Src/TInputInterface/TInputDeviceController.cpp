// Z:\Toshi\Source\TInput\TInputDeviceController.cpp  (reconstruction)
#define TINPUT_EXPORTS
#include "TNewDelete.h"
#include "TInputInterface.h"

namespace Toshi {

TOSHI_DEFINE_CLASS_FUNCTIONS(TInputDeviceController)

TInputDeviceController::TInputDeviceController()        // 0x1ac0
{
    m_buttonsA = (bool*)tmalloc(8, 0, -1);
    m_buttonsB = (bool*)tmalloc(8, 0, -1);
    for (int i = 0; i < 8; i++)
    {
        m_buttonsA[i] = false;
        m_buttonsB[i] = false;
    }
}

TInputDeviceController::~TInputDeviceController()       // 0x2100
{
    tfree(m_buttonsA);
    tfree(m_buttonsB);
}

// SetVibration (0x2180) is inline (empty) in TInputInterface.h.

bool TInputDeviceController::GetDoodadProperties(int doodad, DoodadProperties& props) const   // 0x1b60
{
    if (doodad < 0x10000 || doodad >= 0x1001c)
        return false;
    if (doodad == 0x10019 || doodad == 0x1001a || doodad == 0x1001b)
    {
        props.m_type = 2;
        props.m_flag = true;
    }
    else
    {
        props.m_type = 0;
        props.m_flag = false;
    }
    return true;
}

bool TInputDeviceController::IsDown(int doodad) const        // 0x1bb0
{
    switch (doodad)
    {
    case 0x10011: return m_buttonsA[0];
    case 0x10012: return m_buttonsA[1];
    case 0x10013: return m_buttonsA[2];
    case 0x10014: return m_buttonsA[3];
    case 0x10015: return m_buttonsA[4];
    case 0x10016: return m_buttonsA[5];
    case 0x10017: return m_buttonsA[6];
    case 0x10018: return m_buttonsA[7];
    default:      return false;
    }
}

bool TInputDeviceController::WasDown(int doodad) const       // 0x1c20
{
    switch (doodad)
    {
    case 0x10011: return m_buttonsB[0];
    case 0x10012: return m_buttonsB[1];
    case 0x10013: return m_buttonsB[2];
    case 0x10014: return m_buttonsB[3];
    case 0x10015: return m_buttonsB[4];
    case 0x10016: return m_buttonsB[5];
    case 0x10017: return m_buttonsB[6];
    case 0x10018: return m_buttonsB[7];
    default:      return false;
    }
}

// Sends an event for every button whose state changed, then one stick event (type 3).
int TInputDeviceController::ProcessVirtualButtons(Emitter& emitter, float)   // 0x2000
{
    int count = 0;
    int i = 0;
    int doodad = 0x10011;
    do
    {
        if (m_buttonsA[i] != m_buttonsB[i])
        {
            TInputInterface::InputEvent ev(this, doodad, (TInputInterface::EventType)((m_buttonsA[i] == true) ? 0 : 1));
            emitter.Throw(ev);
            count = count + 1;
        }
        doodad = doodad + 1;
        i = i + 1;
    } while (i < 8);

    TInputInterface::InputEvent stick(this, 0x1001b, (TInputInterface::EventType)3, m_axisX, m_axisY);
    emitter.Throw(stick);
    return count + 1;
}

// Turns the two analogue sticks into 8 digital "virtual buttons" with hysteresis:
// press past 0.75, release back inside 0.5.
void TInputDeviceController::UpdateVirtualButtons()     // 0x1c90
{
    bool* tmp = m_buttonsB;
    m_buttonsB = m_buttonsA;
    m_buttonsA = tmp;

    // The binary keeps these four as real 4-byte floats on the stack (fstp dword); binding the
    // results to const references reproduces that (plain float locals get 8-byte spills).
    // Remaining difference: stack slot order (binary: lx@0 rx@4 ly@8 ry@c, ours: declaration order).
    const float& lx = GetAxisFloat(0x10019, 0);
    const float& ly = GetAxisFloat(0x10019, 1);
    const float& rx = GetAxisFloat(0x1001a, 0);
    const float& ry = GetAxisFloat(0x1001a, 1);

    if (m_buttonsB[3] == false && lx < -0.75f)      m_buttonsA[3] = true;
    else if (m_buttonsB[3] == true && lx > -0.5f)   m_buttonsA[3] = false;
    else                                            m_buttonsA[3] = m_buttonsB[3];

    if (m_buttonsB[1] == false && lx > 0.75f)       m_buttonsA[1] = true;
    else if (m_buttonsB[1] == true && lx < 0.5f)    m_buttonsA[1] = false;
    else                                            m_buttonsA[1] = m_buttonsB[1];

    if (m_buttonsB[0] == false && ly < -0.75f)      m_buttonsA[0] = true;
    else if (m_buttonsB[0] == true && ly > -0.5f)   m_buttonsA[0] = false;
    else                                            m_buttonsA[0] = m_buttonsB[0];

    if (m_buttonsB[2] == false && ly > 0.75f)       m_buttonsA[2] = true;
    else if (m_buttonsB[2] == true && ly < 0.5f)    m_buttonsA[2] = false;
    else                                            m_buttonsA[2] = m_buttonsB[2];

    if (m_buttonsB[7] == false && rx < -0.75f)      m_buttonsA[7] = true;
    else if (m_buttonsB[7] == true && rx > -0.5f)   m_buttonsA[7] = false;
    else                                            m_buttonsA[7] = m_buttonsB[7];

    if (m_buttonsB[5] == false && rx > 0.75f)       m_buttonsA[5] = true;
    else if (m_buttonsB[5] == true && rx < 0.5f)    m_buttonsA[5] = false;
    else                                            m_buttonsA[5] = m_buttonsB[5];

    if (m_buttonsB[4] == false && ry < -0.75f)      m_buttonsA[4] = true;
    else if (m_buttonsB[4] == true && ry > -0.5f)   m_buttonsA[4] = false;
    else                                            m_buttonsA[4] = m_buttonsB[4];

    if (m_buttonsB[6] == false && ry > 0.75f)       m_buttonsA[6] = true;
    else if (m_buttonsB[6] == true && ry < 0.5f)    m_buttonsA[6] = false;
    else                                            m_buttonsA[6] = m_buttonsB[6];

    m_prevAxisX = m_axisX;
    m_prevAxisY = m_axisY;
    if (IsDown(0x10004))      m_axisX = -1.0f;
    else if (IsDown(0x10002)) m_axisX = 1.0f;
    else                      m_axisX = 0.0f;
    if (IsDown(0x10001))      m_axisY = -1.0f;
    else if (IsDown(0x10003)) m_axisY = 1.0f;
    else                      m_axisY = 0.0f;
}

} // namespace Toshi
