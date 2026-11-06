# TInputInterface.dll - virtual table layout (read from the installed DLL)

Common to every Toshi object:  [0] GetClass() const   [1] TObject::Delete   [2] destructor (vector deleting)
`_purecall` slots are pure virtuals that the DirectInput DLL (TInputDXInterface.dll) overrides - take their
names from that DLL's exports when we get to it.

## TInputDevice (vtable @ 0x7020, 20 slots, sizeof 0x3C)
| slot | function |
|---|---|
| 3,4,5,6 | pure (4)  |
| 7 | Flush() |
| 8,9,10,11 | pure; slot 10 = GetAxisCount(), slot 11 = GetDoodadProperties(int, DoodadProperties&) const |
| 12 | StartRepeat |
| 13 | StopRepeat |
| 14 | StopAllRepeats |
| 15 | ThrowRepeatEvent |
| 16 | IsForceFeedbackDevice |
| 17,18,19 | slot 17 = IsDown (pure in base), 18 = GetAxisInt(int,int) const, 19 = GetAxisFloat(int,int) const |

Controller adds: 20 SetVibration, 21 WasDown, 22 ProcessVirtualButtons.  Keyboard adds 20-24 (pure) and
25 ProcessVirtualButtons.  Mouse adds nothing.

## TInputInterface (vtable @ 0x7300, sizeof 0x34)
3 Initialise (PURE but with a body at 0x2d80), 4 Deinitialise (pure with a body at 0x28d0), 5 AcquireAll,
6 UnacquireAll, 7 FlushAll, 8 SetExclusiveMode(bool), 9 GetExclusiveMode() const, 10 ProcessEvents, 11 StopAllRepeats.

## Layouts
- InputEvent (0x1C): +0 doodad, +4 type, +8 isFloat(bool), +9 axisCount(char), +0xC source, +0x10/+0x14/+0x18 magnitudes (int|float).
- TInputDeviceChangeEvent (8): +0 device, +4 type.
- TInputDevice (0x3C): +4 list owner, +8 next, +0xC prev, +0x14 repeat array {grow=4,count,cap,data} of 12-byte
  RepeatInfo, +0x24 latch array {grow=4,count,cap,data}, +0x34 bool, +0x35 acquired, +0x38 TInputInterface*.
- TInputInterface: device list is circular with its sentinel node at this+8 (head +0x10, tail +0x14, count +0x18).
- Each TClass object (m_sClass) is 0x40 bytes.
