// Toshi::TNodeTree<T> - intrusive tree (header-only template), reconstructed from TRenderInterface.dll
// (the resource tree). TNode: m_pTree (+0), m_pNext (+4), m_pPrev (+8), m_pParent (+0xC),
// m_pChild (+0x10); siblings form a ring. TNodeTree: m_oRoot (+0), m_iCount (+0x14).
// The template code reads the links through T's own (out-of-line) Next()/Child()/Parent()
// functions where the original does (TResource exports them); names follow OpenToshi.
#ifndef TNODETREE_H
#define TNODETREE_H

namespace Toshi {

template <class T>
class TNodeTree
{
public:
    class TNode
    {
    public:
        TNode()
        {
            // store order of the original: links first, the owner pointer last (round 36)
            m_pParent = 0;
            m_pChild = 0;
            m_pNext = (T*)this;
            m_pPrev = (T*)this;
            m_pTree = 0;
        }

#ifdef TRES_CHILD_NODE
        T* Child() const { return m_pChild; }
#endif
        TNodeTree* m_pTree;     // +0x00
        T*         m_pNext;     // +0x04
        T*         m_pPrev;     // +0x08
        T*         m_pParent;   // +0x0C
        T*         m_pChild;    // +0x10
    };

    TNodeTree() { m_iCount = 0; }
    ~TNodeTree() { DeleteAll(); }

    void DeleteRecurse(T* node);
    void DeleteAll()
    {
        T* node = m_oRoot.m_pChild;
        while (node != 0)
        {
            Remove(*node, false);
            DeleteRecurse(node);
            node = m_oRoot.m_pChild;
        }
    }

    // inserts node as the last child of parent
    void Insert(T* parent, T* node)
    {
        Remove(*node, false);
        T* first = parent->m_pChild;
#ifdef TNT_INSERT_SWAP
        if (first != 0)
        {
            node->m_pTree = this;
            node->m_pNext = first;
            node->m_pPrev = first->m_pPrev;
#ifdef TNT_INSERT_A
            first->m_pPrev->m_pNext = node;
#else
            node->m_pPrev->m_pNext = node;      // round 38: the original reuses the value just stored
#endif
            first->m_pPrev = node;
            node->m_pParent = first->m_pParent;
            m_iCount++;
        }
        else
        {
            parent->m_pChild = node;
            m_iCount++;
        }
#else
        if (first == 0)
        {
            parent->m_pChild = node;
            m_iCount++;
        }
        else
        {
            node->m_pTree = this;
            node->m_pNext = first;
            node->m_pPrev = first->m_pPrev;
#ifdef TNT_INSERT_A
            first->m_pPrev->m_pNext = node;
#else
            node->m_pPrev->m_pNext = node;      // round 38: the original reuses the value just stored
#endif
            first->m_pPrev = node;
            node->m_pParent = first->m_pParent;
            m_iCount++;
        }
#endif
        node->m_pParent = parent;
        node->m_pTree = this;
    }
    void InsertAtRoot(T* node) { Insert(GetRoot(), node); }

    T* Remove(T& node, bool moveChildren);

    typedef bool (__stdcall *t_RecurseSimpleCallback)(T* node, void* userData);
    bool RecurseSimple(t_RecurseSimpleCallback callback, T* start, bool siblings, void* userData);
    typedef bool (__stdcall *t_RecurseCallback)(T* node, void* userData);
    void Recurse(t_RecurseCallback enter, t_RecurseCallback leave, t_RecurseCallback callback,
                 T* start, bool siblings, void* userData);

    T* GetRoot() { return (T*)&m_oRoot; }
    int Count() const { return m_iCount; }

    TNode m_oRoot;              // +0x00
    int   m_iCount;             // +0x14

private:
    TNodeTree(const TNodeTree&);
    TNodeTree& operator=(const TNodeTree&);
};

// Not inline in the original: one local copy per file (0xc990 in TRenderInterface.cpp's range).
template <class T>
T* TNodeTree<T>::Remove(T& node, bool moveChildren)
{
    TNodeTree* tree = node.m_pTree;
    if (tree != 0)
    {
        if (tree != this)
            return &node;
        m_iCount--;
    }
    if (moveChildren == true)
    {
        while (node.Child() != 0)
        {
            T* parent = node.Parent();
            T* child = node.Child();
            Remove(*child, false);
            if (parent != 0)
                Insert(parent, child);
            else
                InsertAtRoot(child);
        }
    }
    if (node.m_pParent != 0)
    {
        if (node.m_pParent->Child() == &node)
        {
            if (node.Next() == &node)
                node.m_pParent->m_pChild = 0;
            else
                node.m_pParent->m_pChild = node.Next();
        }
        node.m_pParent = 0;
    }
    node.m_pPrev->m_pNext = node.m_pNext;
    node.m_pNext->m_pPrev = node.m_pPrev;
    node.m_pNext = &node;
    node.m_pPrev = &node;
    node.m_pTree = 0;
    return &node;
}

// Not inline (0xc910 in TRenderInterface.cpp's range): deletes node, its ring of siblings and their children.
template <class T>
void TNodeTree<T>::DeleteRecurse(T* node)
{
    while (node != 0)
    {
#ifdef TNT_DR_NODE
        TNode* tn = node;
#define TNT_DR_CHILD tn->m_pChild
#else
#define TNT_DR_CHILD node->m_pChild
#endif
        T* next = node->m_pNext;
        if (next == node)
            next = 0;
        if (TNT_DR_CHILD != 0)
            DeleteRecurse(TNT_DR_CHILD);
#undef TNT_DR_CHILD
        Remove(*node, false);
        delete node;
        node = next;
    }
}

// Not inline either (0xc790): visits start (and, if siblings, the rest of its ring), depth first.
template <class T>
bool TNodeTree<T>::RecurseSimple(t_RecurseSimpleCallback callback, T* start, bool siblings, void* userData)
{
    T* node = start;
    while (node != 0)
    {
        T* next = node->Next();
        if (next == start || next == node || !siblings)
            next = 0;
        if (!callback(node, userData))
            return false;
        if (node->Child() != 0)
        {
            if (!RecurseSimple(callback, node->Child(), true, userData))
                return false;
        }
        node = next;
    }
    return true;
}

// Not inline (0xc840): like RecurseSimple, plus enter/leave callbacks around each child ring.
// The enter/leave callbacks get the ring's start node, not the parent (as in the original).
template <class T>
void TNodeTree<T>::Recurse(t_RecurseCallback enter, t_RecurseCallback leave, t_RecurseCallback callback,
                           T* start, bool siblings, void* userData)
{
    T* node = start;
    while (node != 0)
    {
        T* next = node->Next();
        if (next == start || next == node || !siblings)
            next = 0;
        if (callback)
            callback(node, userData);
        if (node->Child() != 0)
        {
            if (enter)
                enter(start, userData);
            Recurse(enter, leave, callback, node->Child(), true, userData);
            if (leave)
                leave(start, userData);
        }
        node = next;
    }
}

} // namespace Toshi
#endif
