// Toshi input classes - reconstruction of TInputInterface.dll  (BATCH 2a)
// All classes are __declspec(dllexport), as in TApplication.dll.
// Virtual slot layout (see notes\TInputInterface_vtables.md):
//   every object:  [0] GetClass  [1] Delete  [2] destructor
//   TInputDevice:  3 Acquire 4 Unacquire 5 Release 6 Update 7 Flush 8 ProcessEvents 9 GetButtonCount
//                  10 GetAxisCount 11 GetDoodadProperties 12 StartRepeat 13 StopRepeat
//                  14 StopAllRepeats 15 ThrowRepeatEvent 16 IsForceFeedbackDevice 17 IsDown
//                  18 GetAxisInt 19 GetAxisFloat
#ifndef TINPUTINTERFACE_H
#define TINPUTINTERFACE_H
#include "TObject.h"
#include "TNode.h"
#include "TArray.h"

#ifdef TINPUT_EXPORTS
#define TINPUT_API __declspec(dllexport)
#else
#define TINPUT_API __declspec(dllimport)
#endif

namespace Toshi {

class TInputDevice;
class TInputInterface;

// ------------------------------------------------------------------ TInputDeviceChangeEvent
class TINPUT_API TInputDeviceChangeEvent
{
public:
    enum Type { TYPE_0, TYPE_1 };

    TInputDeviceChangeEvent(TInputDevice* device, Type type);

    TInputDeviceChangeEvent& operator=(const TInputDeviceChangeEvent& o)
    {
        m_device = o.m_device;
        m_type = o.m_type;
        return *this;
    }
    Type GetType() const { return m_type; }
    TInputDevice* GetDevice() const { return m_device; }

private:
    TInputDevice* m_device;      // +0
    Type          m_type;        // +4
};

// ------------------------------------------------------------------ TInputInterface
// A second 4-byte base at +4 whose constructor stores 0 BEFORE the vtable is set.
class TInputInterfaceBase
{
public:
    TInputInterfaceBase() { m_unk04 = 0; }
    int m_unk04;
};

class TINPUT_API TInputInterface : public TObject, public TInputInterfaceBase
{
public:
    enum EventType { EVENT_0, EVENT_1, EVENT_2 /* repeat */ };

    class TINPUT_API InputEvent
    {
    public:
        InputEvent(TInputDevice* src, int doodad, EventType type);
        InputEvent(TInputDevice* src, int doodad, EventType type, float a);
        InputEvent(TInputDevice* src, int doodad, EventType type, int a);
        InputEvent(TInputDevice* src, int doodad, EventType type, float a, float b);
        InputEvent(TInputDevice* src, int doodad, EventType type, int a, int b);

        // operator= is compiler-generated (block copy of 7 dwords)
        bool IsMagnitudeFloat() const { return m_isFloat; }
        bool IsMagnitudeInt() const { return m_isFloat == false; }
        signed char GetAxisCount() const { return m_axisCount; }
        TInputDevice* GetSource() const { return m_source; }
        EventType GetEventType() const { return m_type; }
        int GetDoodad() const { return m_doodad; }
        float GetMagnitudeFloat(int axis) const;
        int GetMagnitudeInt(int axis) const;

    private:
        int           m_doodad;      // +0x00
        EventType     m_type;        // +0x04
        bool          m_isFloat;     // +0x08
        signed char   m_axisCount;   // +0x09
        TInputDevice* m_source;      // +0x0c
        union Mag { int i; float f; } m_mag[3];   // +0x10 .. +0x1b   (sizeof == 0x1C)
    };

    // The device list: bool flag at +8 (shares the sentinel's first byte), the sentinel node at +0xC
    // (owner, next = head, prev = tail), count at +0x18.  Its destructor deletes every device.
    struct DeviceList
    {
        DeviceList() : m_exclusive(false), m_count(0) {}
        ~DeviceList();
        bool IsEmpty() const { return m_node.m_next == (TInputDevice*)this; }
        bool NotEmpty() const { return m_node.m_next != (TInputDevice*)this; }
        bool IsValid(TInputDevice* d) const { return d != (TInputDevice*)this; }
        bool IsEnd(TInputDevice* d) const { return d == (TInputDevice*)this; }
        bool IsNodeEmpty() const;                          // defined in TInputInterface.cpp
        bool IsNodeLinked(TNode<TInputDevice>* n) const;
        bool                m_exclusive;   // +0x08
        TNode<TInputDevice> m_node;        // +0x0c
        int                 m_count;       // +0x18
    };

    TOSHI_DECLARE_CLASS(TInputInterface)

    TInputInterface();
    virtual ~TInputInterface();                         // slot 2
    virtual bool Initialise() = 0;                      // 3: PURE but has a body
    virtual bool Deinitialise() = 0;                    // 4: PURE but has a body
    virtual bool AcquireAll();                          // 5
    virtual bool UnacquireAll();                        // 6
    virtual bool FlushAll();                            // 7
    virtual void SetExclusiveMode(bool exclusive);      // 8
    virtual bool GetExclusiveMode() const;              // 9
    virtual int  ProcessEvents(float dt);               // 10
    virtual void StopAllRepeats();                      // 11

    void AddDevice(TInputDevice* device);
    void RemoveDevice(TInputDevice* device);

    // Returns the index-th device that IsA T (instantiated in TInputDXInterface.dll at 0x3c80;
    // the real name is unknown - it is not exported).
    template <class T> T* GetDeviceByIndex(int index)
    {
        for (TInputDevice* d = m_list.m_node.m_next; d != (TInputDevice*)&m_list; d = (d != 0) ? d->m_next : d)
        {
            if (d->IsA(T::m_sClass))
            {
                if (index-- == 0)
                    return (T*)d;
            }
        }
        return 0;
    }

    DeviceList m_list;                                  // +0x08
    TEmitter<TInputInterface, InputEvent> m_emitterA;   // +0x1c
    TEmitter<TInputInterface, TInputDeviceChangeEvent> m_emitterB;   // +0x28  device (un)acquired; sizeof == 0x34
};

// ------------------------------------------------------------------ TInputDevice
class TINPUT_API TInputDevice : public TObject, public TNode<TInputDevice>
{
public:
    struct RepeatInfo { int m_doodad; float m_timer; float m_interval; };   // 12 bytes
    struct DoodadProperties { int m_type; bool m_flag; };
    typedef TEmitter<TInputInterface, TInputInterface::InputEvent> Emitter;

    TOSHI_DECLARE_CLASS(TInputDevice)

    TInputDevice();
    virtual ~TInputDevice();                                              // 2
    virtual bool Acquire() = 0;                                           // 3
    virtual bool Unacquire() = 0;                                         // 4
    virtual void Release() = 0;                                           // 5
    virtual void Update(float dt) = 0;                                    // 6
    // Flush, IsForceFeedbackDevice (and the controller's SetVibration) are INLINE in the
    // header: TInputDXInterface.dll carries its own local copies of them for its vtables.
    virtual bool Flush() { return true; }                                 // 7
    virtual int  ProcessEvents(Emitter& emitter, float dt) = 0;           // 8
    virtual int  GetButtonCount() = 0;                                    // 9
    virtual int  GetAxisCount() = 0;                                      // 10
    virtual bool GetDoodadProperties(int doodad, DoodadProperties& props) const = 0;   // 11
    virtual bool StartRepeat(int doodad, float delay, float interval);    // 12
    virtual void StopRepeat(int doodad);                                  // 13
    virtual void StopAllRepeats();                                        // 14
    virtual void ThrowRepeatEvent(Emitter& emitter, RepeatInfo* info, float time);   // 15
    virtual bool IsForceFeedbackDevice() { return false; }                // 16
    virtual bool IsDown(int doodad) const = 0;                            // 17
    virtual int  GetAxisInt(int a, int b) const = 0;                      // 18
    virtual float GetAxisFloat(int a, int b) const = 0;                   // 19

    void SetInputInterface(TInputInterface* iface);
    TInputInterface* GetInputInterface();
    bool IsAcquired() const;
    bool Latch(int doodad);
    void Unlatch(int doodad);
    bool IsLatched(int doodad);
    void UnlatchAll(int unused);

protected:
    int ProcessRepeats(Emitter& emitter, float dt);

public:
    // layout (TObject vptr +0, TNode<TInputDevice> +4..+0xF)
    int                     m_unk10;            // +0x10
    TArray<RepeatInfo>      m_repeats;          // +0x14
    TArray<int>             m_latched;          // +0x24
    bool                    m_unk34;            // +0x34
    bool                    m_acquired;         // +0x35
    TInputInterface*        m_inputInterface;   // +0x38      sizeof == 0x3C
};

// ------------------------------------------------------------------ TInputDeviceController
class TINPUT_API TInputDeviceController : public TInputDevice
{
public:
    enum VIBRATION_ACTUATOR { VIBRATION_0 };

    TOSHI_DECLARE_CLASS(TInputDeviceController)

    TInputDeviceController();
    virtual ~TInputDeviceController();
    virtual bool GetDoodadProperties(int doodad, DoodadProperties& props) const;   // 11
    virtual bool IsDown(int doodad) const;                                          // 17
    virtual void SetVibration(VIBRATION_ACTUATOR actuator, float amount) {}         // 20
    virtual bool WasDown(int doodad) const;                                         // 21
protected:
    virtual int ProcessVirtualButtons(Emitter& emitter, float dt);                  // 22
    void UpdateVirtualButtons();
public:
    bool*          m_buttonsA;      // +0x3c  current button states (8 bytes, heap)
    bool*          m_buttonsB;      // +0x40  previous button states (8 bytes, heap)
    float          m_axisX;         // +0x44  virtual stick, filled by UpdateVirtualButtons
    float          m_axisY;         // +0x48
    float          m_prevAxisX;     // +0x4c
    float          m_prevAxisY;     // +0x50                          sizeof == 0x54
};

// ------------------------------------------------------------------ TInputDeviceKeyboard
class TINPUT_API TInputDeviceKeyboard : public TInputDevice
{
public:
    TOSHI_DECLARE_CLASS(TInputDeviceKeyboard)

    TInputDeviceKeyboard();
    virtual ~TInputDeviceKeyboard();
    virtual int   GetAxisCount();                                                   // 10
    virtual bool  GetDoodadProperties(int doodad, DoodadProperties& props) const;   // 11
    virtual int   GetAxisInt(int a, int b) const;                                   // 18
    virtual float GetAxisFloat(int a, int b) const;                                 // 19
    virtual bool  IsShiftDown() const = 0;                                          // 20
    virtual bool  IsControlDown() const = 0;                                        // 21
    virtual bool  IsAltDown() const = 0;                                            // 22
    virtual bool  WasDown(int doodad) const = 0;                                    // 23
    virtual unsigned short TranslateDoodadToCharacter(int doodad) const = 0;        // 24
protected:
    virtual int ProcessVirtualButtons(Emitter& emitter, float dt);                  // 25
    void UpdateVirtualButtons();
public:
    void SetVirtStickDoodads(int stick, int a, int b, int c, int d);
    void GetVirtStickDoodads(int stick, int& a, int& b, int& c, int& d);

    float m_axisA;                      // +0x3c  virtual stick values from UpdateVirtualButtons
    float m_axisB;                      // +0x40
    float m_prevAxisA;                  // +0x44
    float m_prevAxisB;                  // +0x48
    struct VirtStick { int m_d[4]; } m_virtSticks[2];   // +0x4c
};

// ------------------------------------------------------------------ TInputDeviceMouse
class TINPUT_API TInputDeviceMouse : public TInputDevice
{
public:
    TOSHI_DECLARE_CLASS(TInputDeviceMouse)

    TInputDeviceMouse();
    virtual ~TInputDeviceMouse();
    virtual bool GetDoodadProperties(int doodad, DoodadProperties& props) const;   // 11
};

} // namespace Toshi
#endif
