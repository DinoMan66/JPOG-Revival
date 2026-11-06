// Toshi::TManagedPtr<T> - intrusive reference-counting pointer (header-only template).
// T must have an int m_iRefCount and a Delete() that is called when the count drops to 0
// (PProperties: count at +4, Delete is TObject's virtual slot 1;
//  PPropertyValueArray: count at +0, Delete is inline "delete this").
#ifndef TMANAGEDPTR_H
#define TMANAGEDPTR_H

namespace Toshi {

// The reference count of PProperties / PPropertyBlock lives in a small second base
// class (its constructor stores 0 BEFORE the vtable pointer, exactly like the originals - round 18).
class TRefCountBase
{
public:
    TRefCountBase() { m_iRefCount = 0; }
    int m_iRefCount;
};

template <class T>
class TManagedPtr
{
public:
    TManagedPtr() { m_pObject = 0; }
    TManagedPtr(T* p)
    {
        if (p)
        {
            m_pObject = p;
            if (m_pObject) m_pObject->m_iRefCount++;
        }
        else
            m_pObject = 0;
    }
    TManagedPtr(const TManagedPtr& o)
    {
        if (o.m_pObject)
        {
            m_pObject = o.m_pObject;
            if (m_pObject) m_pObject->m_iRefCount++;
        }
        else
            m_pObject = 0;
    }
    ~TManagedPtr()
    {
        if (m_pObject) Release();
    }
    TManagedPtr& operator=(T* p)
    {
        if (m_pObject != p)
        {
            if (m_pObject && --m_pObject->m_iRefCount == 0)
                m_pObject->Delete();
            m_pObject = p;
            if (m_pObject) m_pObject->m_iRefCount++;
        }
        return *this;
    }
    TManagedPtr& operator=(const TManagedPtr& o) { return operator=(o.m_pObject); }
    T* operator->() const { return m_pObject; }
    T& operator*() const { return *m_pObject; }
    operator T*() const { return m_pObject; }
    T* Get() const { return m_pObject; }

private:
    void Release()
    {
        if (m_pObject && --m_pObject->m_iRefCount == 0)
            m_pObject->Delete();
        m_pObject = 0;
    }
    T* m_pObject;
};

} // namespace Toshi
#endif
