// Toshi::TNodeList<T> - intrusive doubly linked list with an owner pointer per node (header-only).
// Reconstructed from TRenderInterface.dll (TRenderAdapter::Mode lists); names follow OpenToshi.
//   TNode   : m_pList (+0), m_pNext (+4), m_pPrev (+8) - next/prev point at the T objects; the
//             node is a base class of T (after T's vtable pointer)
//   TNodeList: m_oRoot (a TNode, +0) and m_iCount (+0xC). The root seen as a T* is &m_oRoot
//             minus the node's offset in T.
#ifndef TNODELIST_H
#define TNODELIST_H

namespace Toshi {

template <class T>
class TNodeList
{
public:
    class TNode
    {
    public:
        TNode() { m_pNext = (T*)this; m_pPrev = (T*)this; m_pList = 0; }   // original store order
        T* Next() const { return m_pNext; }
        T* Prev() const { return m_pPrev; }
        TNodeList* GetList() const { return m_pList; }
        bool IsLinked() const { return m_pList != 0; }

        TNodeList* m_pList;     // +0
        T*         m_pNext;     // +4
        T*         m_pPrev;     // +8
    };

    // Iterator: the original's loops step with a null check (operator++) and build the End()
    // iterator as a temporary on every comparison.
    class Iterator
    {
    public:
        Iterator(T* ptr) : m_pPtr(ptr) {}
        T* operator->() const { return m_pPtr; }
        T* Get() const { return m_pPtr; }
        // by value: the original keeps both iterators in registers (round 87)
        bool operator==(Iterator other) const { return m_pPtr == other.m_pPtr; }
        bool operator!=(Iterator other) const { return m_pPtr != other.m_pPtr; }
        Iterator& operator++()
        {
            if (m_pPtr)
                m_pPtr = m_pPtr->Next();
            return *this;
        }

        T* m_pPtr;
    };

    TNodeList() { m_iCount = 0; }
#ifndef TNODELIST_NO_DTOR
    ~TNodeList() { DeleteAll(); }
#endif

    T* Remove(T* node)
    {
        node->m_pList = 0;
        node->m_pPrev->m_pNext = node->m_pNext;
        node->m_pNext->m_pPrev = node->m_pPrev;
        node->m_pNext = node;
        node->m_pPrev = node;
        m_iCount--;
        return node;
    }
    void InsertTail(T* node)
    {
        node->m_pList = this;
        node->m_pNext = End();
        node->m_pPrev = m_oRoot.m_pPrev;
        node->m_pPrev->m_pNext = node;      // reuses the value just stored (AddHandler, LoadMaterialLibrary)
        m_oRoot.m_pPrev = node;
        m_iCount++;
    }
    void InsertHead(T* node)
    {
        node->m_pList = this;
        node->m_pNext = m_oRoot.m_pNext;
        node->m_pPrev = End();
        node->m_pNext->m_pPrev = node;      // reuses the value just stored (RegisterTexture)
        m_oRoot.m_pNext = node;
        m_iCount++;
    }
    // Moves every node of 'other' to the end of this list (TDImp::ReorderAllKeySeqs).
    void Append(TNodeList& other)
    {
        T* last = m_oRoot.m_pPrev;
        T* first = other.m_oRoot.m_pNext;
        T* otherLast = other.m_oRoot.m_pPrev;
        if (other.IsEmpty() == true)
            return;
        for (Iterator it = other.Begin(); it != other.End(); ++it)
            it->m_pList = this;
        m_iCount += other.m_iCount;
        last->m_pNext = first;
        first->m_pPrev = last;
        otherLast->m_pNext = End();
        m_oRoot.m_pPrev = otherLast;
        other.m_iCount = 0;
        other.m_oRoot.m_pPrev = other.End();
        other.m_oRoot.m_pNext = other.End();
    }
    T* RemoveHead()
    {
        if (m_oRoot.m_pNext == End())
            return 0;
        return Remove(m_oRoot.m_pNext);
    }
    void DeleteAll()
    {
        while (!IsEmpty())
        {
            T* node = Remove(m_oRoot.m_pNext);
            delete node;
        }
    }

    bool IsEmpty() const { return m_oRoot.m_pNext == &m_oRoot; }
    int Count() const { return m_iCount; }
    T* Head() const { return m_oRoot.m_pNext; }
    T* Tail() const { return m_oRoot.m_pPrev; }
    T* Begin() const { return m_oRoot.m_pNext; }
    T* End() const { return (T*)&m_oRoot; }

    TNode m_oRoot;              // +0x0
    int   m_iCount;             // +0xC

private:
    TNodeList(const TNodeList&);
    TNodeList& operator=(const TNodeList&);
};

} // namespace Toshi
#endif
