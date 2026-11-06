// Z:\Toshi\Source\TInput\TInputDeviceMouse.cpp  (reconstruction)
#define TINPUT_EXPORTS
#include "TNewDelete.h"
#include "TInputInterface.h"

namespace Toshi {

TOSHI_DEFINE_CLASS_FUNCTIONS(TInputDeviceMouse)

TInputDeviceMouse::TInputDeviceMouse()                  // 0x2710
{
}

TInputDeviceMouse::~TInputDeviceMouse()                 // 0x2700
{
}

bool TInputDeviceMouse::GetDoodadProperties(int doodad, DoodadProperties& props) const   // 0x2650
{
    if (doodad < 0x30000 || doodad >= 0x3000d)
        return false;
    if (doodad == 0x3000b)
    {
        props.m_type = 2;
        props.m_flag = false;
    }
    else if (doodad == 0x3000c)
    {
        props.m_type = 1;
        props.m_flag = false;
    }
    else
    {
        props.m_type = 0;
        props.m_flag = false;
    }
    return true;
}

} // namespace Toshi
