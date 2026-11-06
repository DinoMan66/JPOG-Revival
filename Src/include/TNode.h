// Intrusive list node, used as a BASE class of list members (reconstructed).
// Evidence: TInputDevice's constructor stores next, prev, owner (in that order) before it sets the
// vtable, and the list-cleanup loop converts device pointers with "device + 4" - i.e. the node is
// a second base class placed right after the 4-byte vptr of TObject.
// The links point at the whole object (T*), so the "self" value is this-4 for a node at +4.
#ifndef TNODE_H
#define TNODE_H

namespace Toshi {

template <class T>
class TNode
{
public:
    TNode()
    {
        m_next = (T*)((char*)this - 4);
        m_prev = m_next;
        m_owner = 0;
    }
    void* m_owner;     // node +0
    T*    m_next;      // node +4
    T*    m_prev;      // node +8
};

} // namespace Toshi
#endif
