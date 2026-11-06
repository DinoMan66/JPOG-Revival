// Intrusive reference-counted pointer used for TApplication::m_pKernel (reconstructed).
// The pointed-to object keeps its count at +4 and has a virtual Delete() in vtable slot 1.
// The shape below follows the inlined code seen in TApplication's dtor, copy ctor, both
// assignment operators and Create(). The exact original source is unknown - tune this
// header first when a smart-pointer function mismatches.
#ifndef TREF_H
#define TREF_H

namespace Toshi {

template <class T>
class TRef
{
public:
    TRef() : m_p(0) {}

    TRef(const TRef& o) : m_p(0)
    {
        if (o.m_p) {
            m_p = o.m_p;
            if (m_p) m_p->m_refCount++;     // original re-checks here (extra 'je' after the store)
        }
    }

    ~TRef()
    {
        if (m_p) {
            Release();
            m_p = 0;
        }
    }

    TRef& operator=(T* p)
    {
        if (m_p != p) {
            Release();
            m_p = p;
            if (p) p->m_refCount++;
        }
        return *this;
    }

    TRef& operator=(const TRef& o)
    {
        if (m_p != o.m_p) {
            Release();
            m_p = o.m_p;
            if (m_p) m_p->m_refCount++;
        }
        return *this;
    }

    operator T*() const { return m_p; }
    T* operator->() const { return m_p; }

private:
    void Release()
    {
        if (m_p) {
            if (--m_p->m_refCount == 0)
                m_p->Delete();
        }
    }

    T* m_p;
};

} // namespace Toshi
#endif
