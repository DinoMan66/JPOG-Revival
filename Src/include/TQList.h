// Toshi::TQList<T> - intrusive circular doubly linked list (header-only template).
// Reconstructed from PPropertyParser (PProperties keeps its properties in one):
//   node   : T* next (+0), T* prev (+4); a fresh node points at itself, the destructor unlinks
//   list   : just the head node (8 bytes); it does NOT keep a count (PProperties does)
//   ~TQList: while (!IsEmpty()) RemoveHead();  then the head node unlinks itself
//   Iterator is a single T*; operator++ is "if (p) p = p->next".
#ifndef TQLIST_H
#define TQLIST_H

#ifndef TQLIST_FORM
#define TQLIST_FORM 0
#endif

namespace Toshi {

template <class T>
class TQList
{
public:
    class TNode
    {
    public:
        TNode() { m_pNext = (T*)this; m_pPrev = (T*)this; }
        ~TNode() { Remove(); }

        void Remove()
        {
            m_pPrev->m_pNext = m_pNext;
            m_pNext->m_pPrev = m_pPrev;
            m_pNext = (T*)this;
            m_pPrev = (T*)this;
        }
        bool IsLinked() const { return m_pNext != (T*)this; }
        T* Next() const { return m_pNext; }
        T* Prev() const { return m_pPrev; }

        T* m_pNext;     // +0
        T* m_pPrev;     // +4
    };

    class Iterator
    {
    public:
        Iterator() {}
        Iterator(T* p) { m_pPtr = p; }
        T* operator->() const { return m_pPtr; }
        T& operator*() const { return *m_pPtr; }
        operator T*() const { return m_pPtr; }
        Iterator& operator++()
        {
            if (m_pPtr) m_pPtr = m_pPtr->m_pNext;
            return *this;
        }
        // by value: the original keeps both iterators in registers (round 87)
        bool operator!=(Iterator o) const { return m_pPtr != o.m_pPtr; }
        bool operator==(Iterator o) const { return m_pPtr == o.m_pPtr; }
        T* m_pPtr;
    };

    TQList() {}
#if TQLIST_FORM == 1 || TQLIST_FORM == 3
    ~TQList()
    {
        while (m_oHead.m_pNext != (T*)&m_oHead)
            RemoveHead();
    }
#else
    ~TQList()
    {
        while (!IsEmpty())
            RemoveHead();
    }
#endif

#if TQLIST_FORM == 2 || TQLIST_FORM == 3
    int IsEmpty() const { return m_oHead.m_pNext == (T*)&m_oHead; }
#else
    bool IsEmpty() const { return m_oHead.m_pNext == (T*)&m_oHead; }
#endif
    Iterator Begin() const { return Iterator(m_oHead.m_pNext); }
    Iterator End() const { return Iterator((T*)&m_oHead); }
    T* Head() const { return m_oHead.m_pNext; }

    T* RemoveHead()
    {
#if TQLIST_FORM == 1 || TQLIST_FORM == 3
        if (m_oHead.m_pNext == (T*)&m_oHead) return 0;
#else
        if (IsEmpty()) return 0;
#endif
        T* p = m_oHead.m_pNext;
        p->Remove();
        return p;
    }
    void InsertHead(T* p)
    {
        p->m_pNext = m_oHead.m_pNext;
        p->m_pPrev = (T*)&m_oHead;
        p->m_pNext->m_pPrev = p;
        m_oHead.m_pNext = p;
    }
    void InsertTail(T* p)
    {
        p->m_pNext = (T*)&m_oHead;
        p->m_pPrev = m_oHead.m_pPrev;
        p->m_pPrev->m_pNext = p;                    // through the node, like InsertHead (the original keeps the old tail in a register)
        m_oHead.m_pPrev = p;
    }

    TNode m_oHead;

private:
    // not copyable: classes holding a TQList get no copy constructor / assignment in the original
    // (e.g. PProperties, TSkeletonInstance, TKeyframeLibraryManager export none)
    TQList(const TQList&);
    TQList& operator=(const TQList&);
};

} // namespace Toshi
#endif
