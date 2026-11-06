// Toshi::TStack<T, MAX> - fixed-size stack (header-only template). Layout from TScene's render
// matrix stack (TStack<TMatrix44, 20> at TScene+0x50): the index of the top first, then the items.
#ifndef TSTACK_H
#define TSTACK_H

namespace Toshi {

template <class T, int MAX>
class TStack
{
public:
    T& Top() { return m_aItems[m_iTop]; }
    void Reset() { m_iTop = 0; }
    T& PushNull() { return m_aItems[++m_iTop]; }
    void Push(const T& item)
    {
        int top = ++m_iTop;
        if (top < MAX)
            m_aItems[top] = item;
    }
    void Pop()
    {
        int top = m_iTop - 1;
        if (top < 0)
            m_iTop = 0;
        else
            m_iTop = top;
    }

    int m_iTop;             // +0x00
    T   m_aItems[MAX];      // +0x04
};

} // namespace Toshi
#endif
