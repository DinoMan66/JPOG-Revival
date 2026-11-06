// Z:\Toshi\Source\TInput\TInputInterface.cpp  (reconstruction)
// Also holds the InputEvent constructors and the TInputDeviceChangeEvent constructor
// (they sit at 0x2b30-0x2de0, inside this file's range of the DLL).
#define TINPUT_EXPORTS
#include "TNewDelete.h"
#include "TInputInterface.h"

namespace Toshi {

TOSHI_DEFINE_CLASS_FUNCTIONS(TInputInterface)

// ---------------------------------------------------------------- device list cleanup
// Runs as a MEMBER destructor, after the two emitters are destroyed (the binary does the
// emitters first, then this loop). Unlinks every device and deletes it.
inline bool TInputInterface::DeviceList::IsNodeEmpty() const { return m_node.m_next == &m_node; }
inline bool TInputInterface::DeviceList::IsNodeLinked(TNode<TInputDevice>* n) const { return n != &m_node; }

#define UNLINK_AND_DELETE(dev)                    \
        dev->m_owner = 0;                         \
        dev->m_prev->m_next = dev->m_next;        \
        dev->m_next->m_prev = dev->m_prev;        \
        dev->m_next = dev;                        \
        dev->m_prev = dev;                        \
        m_count = m_count - 1;                    \
        delete dev;

inline TInputInterface::DeviceList::~DeviceList()
{
    while (m_node.m_next != &m_node)
    {
        TInputDevice* dev = m_node.m_next;
        UNLINK_AND_DELETE(dev)
    }
}

// ---------------------------------------------------------------- construction / destruction
TInputInterface::TInputInterface()                      // 0x2800
    : m_emitterA(this), m_emitterB(this)
{
}

TInputInterface::~TInputInterface()                     // 0x2840
{
    // members die in reverse order: m_emitterB, m_emitterA, then m_list (deletes the devices)
}

// ---------------------------------------------------------------- device list
void TInputInterface::RemoveDevice(TInputDevice* device)    // 0x2cf0
{
    if (device != 0)
    {
        device->m_owner = 0;
        device->m_prev->m_next = device->m_next;
        device->m_next->m_prev = device->m_prev;
        device->m_next = device;
        device->m_prev = device;
        m_list.m_count = m_list.m_count - 1;
        device->SetInputInterface(0);
    }
}

void TInputInterface::AddDevice(TInputDevice* device)   // 0x2d30
{
    if (device != 0)
    {
        void* owner = &m_list.m_node;                       // this + 0xc
        TInputDevice* node = (TInputDevice*)&m_list;        // this + 8 (sentinel pseudo-object)
        device->m_owner = owner;
        device->m_next = node;
        TInputDevice* tail = m_list.m_node.m_prev;
        device->m_prev = tail;
        tail->m_next = device;
        m_list.m_node.m_prev = device;
        m_list.m_count = m_list.m_count + 1;
        device->SetInputInterface(this);
    }
}

// ---------------------------------------------------------------- per-device loops
bool TInputInterface::Deinitialise()                    // 0x28d0 (pure virtual WITH a body)
{
    for (TInputDevice* d = m_list.m_node.m_next; d != (TInputDevice*)&m_list; d = (d != 0) ? d->m_next : d)
    {
        d->Unacquire();
        d->Release();
    }
    return true;
}

bool TInputInterface::AcquireAll()                      // 0x2920
{
    bool ok = true;
    for (TInputDevice* d = m_list.m_node.m_next; d != (TInputDevice*)&m_list; d = (d != 0) ? d->m_next : d)
        ok &= d->Acquire();
    return ok;
}

bool TInputInterface::UnacquireAll()                    // 0x2980
{
    bool ok = true;
    for (TInputDevice* d = m_list.m_node.m_next; d != (TInputDevice*)&m_list; d = (d != 0) ? d->m_next : d)
        ok &= d->Unacquire();
    return ok;
}

bool TInputInterface::FlushAll()                        // 0x29e0
{
    bool ok = true;
    for (TInputDevice* d = m_list.m_node.m_next; d != (TInputDevice*)&m_list; d = (d != 0) ? d->m_next : d)
        ok &= d->Flush();
    return ok;
}

int TInputInterface::ProcessEvents(float dt)            // 0x2a40
{
    // The loop test goes through an inline bool helper (DeviceList::IsValid); writing the
    // comparison out directly gives different code (the binary materialises the bool: setne).
    int total = 0;
    TInputDevice* d;
    for (d = m_list.m_node.m_next; m_list.IsValid(d); d = (d != 0) ? d->m_next : d)
        if (d->m_acquired)
            d->Update(dt);
    for (d = m_list.m_node.m_next; m_list.IsValid(d); d = (d != 0) ? d->m_next : d)
        if (d->m_acquired)
            total += d->ProcessEvents(m_emitterA, dt);
    return total;
}

void TInputInterface::StopAllRepeats()                  // 0x2af0
{
    for (TInputDevice* d = m_list.m_node.m_next; d != (TInputDevice*)&m_list; d = (d != 0) ? d->m_next : d)
        d->StopAllRepeats();
}

bool TInputInterface::GetExclusiveMode() const          // 0x2d60
{
    return m_list.m_exclusive;
}

void TInputInterface::SetExclusiveMode(bool exclusive)  // 0x2d70
{
    m_list.m_exclusive = exclusive;
}

bool TInputInterface::Initialise()                      // 0x2d80 (pure virtual WITH a body)
{
    return true;
}

// ---------------------------------------------------------------- InputEvent (0x2b30 - 0x2cc0)
TInputInterface::InputEvent::InputEvent(TInputDevice* src, int doodad, EventType type, float a)   // 0x2b30
{
    m_source = src;
    m_doodad = doodad;
    m_type = type;
    m_mag[0].f = a;
    m_axisCount = 1;
    m_isFloat = true;
}

TInputInterface::InputEvent::InputEvent(TInputDevice* src, int doodad, EventType type, int a)     // 0x2b60
{
    m_source = src;
    m_doodad = doodad;
    m_type = type;
    m_mag[0].i = a;
    m_axisCount = 1;
    m_isFloat = false;
}

TInputInterface::InputEvent::InputEvent(TInputDevice* src, int doodad, EventType type, float a, float b)   // 0x2b90
{
    m_source = src;
    m_doodad = doodad;
    m_type = type;
    m_mag[0].f = a;
    m_mag[1].f = b;
    m_axisCount = 2;
    m_isFloat = true;
}

TInputInterface::InputEvent::InputEvent(TInputDevice* src, int doodad, EventType type, int a, int b)       // 0x2bc0
{
    m_source = src;
    m_doodad = doodad;
    m_type = type;
    m_mag[0].i = a;
    m_mag[1].i = b;
    m_axisCount = 2;
    m_isFloat = false;
}

int TInputInterface::InputEvent::GetMagnitudeInt(int axis) const           // 0x2bf0
{
    if (m_isFloat == false)
        return m_mag[axis].i;
    return (m_mag[axis].f < -0.5f) ? -1 : ((m_mag[axis].f > 0.5f) ? 1 : 0);
}

float TInputInterface::InputEvent::GetMagnitudeFloat(int axis) const       // 0x2ca0
{
    if (m_isFloat != false)
        return m_mag[axis].f;
    return (float)m_mag[axis].i;
}

TInputInterface::InputEvent::InputEvent(TInputDevice* src, int doodad, EventType type)     // 0x2cc0
{
    m_source = src;
    m_doodad = doodad;
    m_type = type;
    m_axisCount = 0;
    m_isFloat = false;
}

TInputDeviceChangeEvent::TInputDeviceChangeEvent(TInputDevice* device, Type type)   // 0x2de0
{
    m_device = device;
    m_type = type;
}

} // namespace Toshi
