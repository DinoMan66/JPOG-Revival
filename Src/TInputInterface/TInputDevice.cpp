// Z:\Toshi\Source\TInput\TInputDevice.cpp  (reconstruction)
// Kept as its OWN translation unit on purpose: ThrowRepeatEvent must CALL the InputEvent
// constructor (defined in TInputInterface.cpp), not inline it.
#define TINPUT_EXPORTS
#include "TNewDelete.h"
#include "TInputInterface.h"

namespace Toshi {

// Loop-form experiments (Var*_TInputDevice.cpp builds set LOOPFORM; default 0 = form in use).
#ifndef LOOPFORM
#define LOOPFORM 0
#endif
#if LOOPFORM == 1
#define FOR_EACH(T, arr, it) for (TArray<T>::Iterator it = arr.Begin(); it.IsValid(); ++it)
#define AT(arr, it) (*it)
#elif LOOPFORM == 2
#define FOR_EACH(T, arr, it) for (TArray<T>::Iterator it = arr.Begin(); it != -1; ++it)
#define AT(arr, it) (*it)
#elif LOOPFORM == 3
#define FOR_EACH(T, arr, it) for (TArray<T>::Iterator it = arr.Begin(); it.IsValid(); ++it)
#define AT(arr, it) arr[it]
#elif LOOPFORM == 4
#define FOR_EACH(T, arr, it) for (TArray<T>::Iterator it(arr); it.IsValid(); ++it)
#define AT(arr, it) (*it)
#else
#define FOR_EACH(T, arr, it) for (TArray<T>::Iterator it = arr.Begin(); it != arr.End(); ++it)
#define AT(arr, it) (*it)
#endif

TOSHI_DEFINE_CLASS_FUNCTIONS(TInputDevice)

// ---------------------------------------------------------------- construction / destruction
// The TNode base constructor (next, prev, owner) and TObject() run before the vtable is set; the
// two TArray members are constructed after it; then the body below.
TInputDevice::TInputDevice()                            // 0x1080
{
    m_unk34 = false;
    m_acquired = false;
    m_inputInterface = 0;
}

TInputDevice::~TInputDevice()                           // 0x10d0
{
    // members die in reverse order: m_latched (frees +0x30), m_repeats (frees +0x20), then ~TObject
}

// ---------------------------------------------------------------- repeats
bool TInputDevice::StartRepeat(int doodad, float delay, float interval)       // 0x1100
{
    FOR_EACH(RepeatInfo, m_repeats, it)
    {
        if (doodad == AT(m_repeats, it).m_doodad)
        {
            AT(m_repeats, it).m_interval = interval;
            return true;
        }
    }
    RepeatInfo& r = m_repeats.AddEmpty();
    r.m_doodad = doodad;
    r.m_timer = delay;
    r.m_interval = interval;
    return true;
}

void TInputDevice::StopRepeat(int doodad)                                     // 0x11f0
{
    FOR_EACH(RepeatInfo, m_repeats, it)
    {
        if (doodad == AT(m_repeats, it).m_doodad)
        {
            m_repeats.RemoveFast(it.m_index);
            return;
        }
    }
}

// ---------------------------------------------------------------- latching
bool TInputDevice::Latch(int doodad)                                          // 0x12a0
{
    FOR_EACH(int, m_latched, it)
    {
        if (doodad == AT(m_latched, it))
            return true;
    }
    m_latched.AddEmpty() = doodad;
    return true;
}

void TInputDevice::Unlatch(int doodad)                                        // 0x1450
{
    FOR_EACH(int, m_latched, it)
    {
        if (AT(m_latched, it) == doodad)
        {
            m_latched.RemoveFast(it.m_index);
            return;
        }
    }
}

bool TInputDevice::IsLatched(int doodad)                                      // 0x14e0
{
    FOR_EACH(int, m_latched, it)
    {
        if (doodad == AT(m_latched, it))
            return true;
    }
    return false;
}

int TInputDevice::ProcessRepeats(Emitter& emitter, float dt)                  // 0x1560
{
    int thrown = 0;
    if (0 < m_repeats.m_count)
    {
        int i = 0;
        do
        {
            RepeatInfo* r = &m_repeats.m_data[i];
            if (!IsDown(r->m_doodad))
            {
                m_repeats.RemoveFast(i);
            }
            else
            {
                float t = r->m_timer - dt;
                if (t <= 0.0f)
                {
                    r->m_timer = r->m_interval;
                    ThrowRepeatEvent(emitter, r, dt);
                    thrown = thrown + 1;
                }
                else
                {
                    r->m_timer = t;
                }
                i = i + 1;
            }
        } while (i < m_repeats.m_count);
    }
    return thrown;
}

void TInputDevice::ThrowRepeatEvent(Emitter& emitter, RepeatInfo* info, float)   // 0x16d0
{
    TInputInterface::InputEvent ev(this, info->m_doodad, (TInputInterface::EventType)2);
    emitter.Throw(ev);
}

void TInputDevice::UnlatchAll(int)                      // 0x1700
{
    m_latched.Clear();
}

void TInputDevice::StopAllRepeats()                     // 0x1720
{
    m_repeats.Clear();
}

void TInputDevice::SetInputInterface(TInputInterface* iface)       // 0x1740
{
    m_inputInterface = iface;
}

// IsForceFeedbackDevice (0x1900) and Flush (0x1910) are inline in TInputInterface.h.

bool TInputDevice::IsAcquired() const                   // 0x1920
{
    return m_acquired;
}

TInputInterface* TInputDevice::GetInputInterface()      // 0x1930
{
    return m_inputInterface;
}

} // namespace Toshi
