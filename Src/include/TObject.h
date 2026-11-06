// Toshi base-object machinery (stubs reconstructed from the imports of TInputInterface.dll;
// the real classes live in TKernelInterface.dll).
//   ??0TClass@Toshi@@QAE@PBDPAV01@P6GPAVTObject@1@XZP6GPAV21@PAX@ZP6GXXZ5I@Z
//   ??1TClass@Toshi@@QAE@XZ                        ?m_sClass@TObject@Toshi@@2VTClass@2@A
//   ??0TObject@Toshi@@IAE@XZ   (protected ctor)    ??1TObject@Toshi@@MAE@XZ  (protected virtual dtor)
//   ?Delete@TObject@Toshi@@UAEXXZ                  (virtual, vtable slot 1)
//   ??0TGenericEmitter@Toshi@@IAE@PAX@Z   ??1TGenericEmitter@Toshi@@IAE@XZ   ?Throw@TGenericEmitter@Toshi@@IAEXPAX@Z
// vtable layout of every Toshi object: [0] GetClass  [1] Delete  [2] destructor  [3..] class-specific
#ifndef TOBJECT_H
#define TOBJECT_H
#include "TMemory.h"

namespace Toshi {

class TObject;

class TClass
{
public:
    typedef TObject* (__stdcall *CreateObjectFn)();
    typedef TObject* (__stdcall *CreateObjectInPlaceFn)(void*);
    typedef void     (__stdcall *StaticFn)();

    TOSHI_API TClass(const char* name, TClass* parent, CreateObjectFn create,
                     CreateObjectInPlaceFn createInPlace, StaticFn initialise,
                     StaticFn deinitialise, unsigned int size);
    TOSHI_API ~TClass();
    TOSHI_API const char* GetName() const;
    TOSHI_API TObject* CreateObject() const;
    TOSHI_API TObject* CreateObjectInPlace(void* memory) const;
    TOSHI_API unsigned short GetVersionMajor() const;
    TOSHI_API unsigned short GetVersionMinor() const;
    TOSHI_API static const TClass* __stdcall Find(const char* name, const TClass* parent);

private:
    unsigned char m_data[0x28];       // PPropertyParser's file-static TClass objects are 0x28 apart
                                      // (exported m_sClass objects are 32-byte aligned -> 0x40 apart)
};

#ifndef TOBJECT_MEMBER_IMPORT   // round 36: class-level import (copy ctor inlines with the imported vtable)
#define TOBJ_M
class TOSHI_API TObject
#else
#define TOBJ_M TOSHI_API
class TObject
#endif
{
public:
    virtual TClass& GetClass() const = 0;            // slot 0
    TOBJ_M virtual void Delete();                 // slot 1
    TOBJ_M bool IsA(const TClass& c) const;
    TOBJ_M static TClass m_sClass;
protected:
    TOBJ_M TObject();
    TOBJ_M virtual ~TObject();                    // slot 2
private:
    // the kernel exports TObject's implicit copy constructor but no operator=, and no TObject-derived
    // class anywhere exports one: assignment is declared private and never defined
    TObject& operator=(const TObject&);
};

class TGenericEmitter
{
protected:
    TOSHI_API TGenericEmitter(void* owner);
    TOSHI_API ~TGenericEmitter();
    TOSHI_API void Throw(void* event);
private:
    unsigned char m_data[0x0C];                      // 0xC bytes (two embedded in TInputInterface: +0x1c, +0x28)
};

// an event with no data (the kernel exports its operator=)
class TOSHI_API TNullEvent
{
};

template <class Owner, class Event>
class TEmitter : public TGenericEmitter
{
public:
    TEmitter(Owner* owner) : TGenericEmitter(owner) {}
    void Throw(Event& e) { TGenericEmitter::Throw(&e); }
};

class TGenericListener
{
protected:
    TOSHI_API TGenericListener();
    TOSHI_API ~TGenericListener();
    TOSHI_API void Connect(TGenericEmitter* emitter, void* owner,
                           bool (__stdcall *callback)(void*, void*, void*), int priority);
    TOSHI_API void Disconnect();
private:
    unsigned char m_data[0x14];
};

template <class Owner, class Caller, class Event>
class TListener : public TGenericListener
{
public:
    typedef bool (__stdcall *Callback)(Owner*, Caller*, Event*);
    TListener() {}
    void Connect(TEmitter<Caller, Event>* emitter, Owner* owner, Callback callback, int priority)
    {
        TGenericListener::Connect(emitter, owner, (bool (__stdcall *)(void*, void*, void*))callback, priority);
    }
    void Disconnect() { TGenericListener::Disconnect(); }
};

} // namespace Toshi

// ---- class boilerplate: every Toshi class has these five functions + a static TClass ----
// (observed for all five TInputInterface.dll classes). CreateObject/CreateObjectInPlace
// return null for these (abstract) classes; both static hooks are empty.
#define TOSHI_DECLARE_CLASS(Name)                                              \
public:                                                                        \
    virtual TClass& GetClass() const;                                          \
    static TClass m_sClass;                                                    \
private:                                                                       \
    static TObject* __stdcall CreateObject();                                  \
    static TObject* __stdcall CreateObjectInPlace(void* p);                    \
    static void __stdcall InitialiseStatic();                                  \
    static void __stdcall DeinitialiseStatic();                                \
public:

#define TOSHI_DEFINE_CLASS_FUNCTIONS(Name)                                     \
    void __stdcall Name::DeinitialiseStatic() {}                               \
    void __stdcall Name::InitialiseStatic() {}                                 \
    TObject* __stdcall Name::CreateObjectInPlace(void*) { return 0; }          \
    TObject* __stdcall Name::CreateObject() { return 0; }                      \
    TClass& Name::GetClass() const { return m_sClass; }

// Concrete classes: CreateObject allocates (tmalloc inlined via operator new) and
// CreateObjectInPlace constructs in the given memory (placement new: null check + ctor).
#define TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(Name)                            \
    void __stdcall Name::DeinitialiseStatic() {}                               \
    void __stdcall Name::InitialiseStatic() {}                                 \
    TObject* __stdcall Name::CreateObjectInPlace(void* p) { return new (p) Name; } \
    TObject* __stdcall Name::CreateObject() { return new Name; }               \
    TClass& Name::GetClass() const { return m_sClass; }

// The class object itself: a static TClass built by a dynamic initializer (+ atexit dtor).
#define TOSHI_DEFINE_CLASS(Name, Parent)                                       \
    TClass Name::m_sClass(#Name, &Parent::m_sClass, Name::CreateObject,         \
                          Name::CreateObjectInPlace, Name::InitialiseStatic,    \
                          Name::DeinitialiseStatic, 0x10000);

#endif
