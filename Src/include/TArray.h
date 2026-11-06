// Toshi growable array (reconstructed from the inlined code in TInputDevice).
// Layout (0x10 bytes): +0 grow, +4 count, +8 capacity, +0xC data.
// Iteration idiom seen everywhere:  i = Begin(); i != -1; i = Next(i)
#ifndef TARRAY_H
#define TARRAY_H
#ifndef ITERKIND
#define ITERKIND 0
#endif
#ifndef SETCAP
#define SETCAP 0
#endif
#include "TMemory.h"

namespace Toshi {

// Ordering used by TArray::Find/FindSorted: T::Compare by default; a type without one (e.g.
// TManagedPtr<PPropertyBlock> in PPropertyWriter) gets a non-template overload next to its user.
template <class T>
inline int TArrayCompare(const T& a, const T& b) { return a.Compare(b); }

template <class T>
class TArray
{
public:
    TArray() { m_count = 0; m_grow = 4; m_capacity = 0; m_data = 0; }    // store order as in the binary
    explicit TArray(int grow) { m_count = 0; m_grow = grow; m_capacity = 0; m_data = 0; }
    TArray(int grow, int capacity)
    {
        m_grow = grow;
        m_count = 0;
        m_capacity = capacity;
        if (capacity > 0)
            m_data = new T[capacity];
        else
            m_data = 0;
    }
    TArray(const TArray& o) { m_data = 0; *this = o; }
    ~TArray() { if (m_data) delete[] m_data; }

    // (the binary's copy has no return value)
    void operator=(const TArray& o)
    {
        if (m_data) delete[] m_data;
        m_count = o.m_count;
        m_grow = o.m_grow;
        m_capacity = o.m_count;
        m_data = new T[m_capacity];
        for (int i = 0; i < m_count; i++)
            m_data[i] = o.m_data[i];
    }

    // The original iterates with a small iterator OBJECT kept on the stack (index at +0, array
    // pointer at +4); operator++ writes the index back every step.
    class Iterator
    {
    public:
        Iterator() {}
        Iterator(TArray* array, int index) : m_index(index), m_array(array) {}
        Iterator(TArray& array) : m_index((array.m_count == 0) ? -1 : 0), m_array(&array) {}
#ifdef TARRAY_ITER_COPYCTOR
        Iterator(const Iterator& o) : m_index(o.m_index), m_array(o.m_array) {}
#endif
#ifdef TARRAY_ITER_DTOR
        ~Iterator() {}
#endif
#if defined(TARRAY_OOL_DEREF)
        T& operator*() const;
#elif ITERKIND == 3
        T& operator*() { return (*m_array)[*this]; }
#else
        T& operator*() const { return m_array->m_data[m_index]; }
#endif
        T* operator->() const { return (m_index < 0) ? 0 : &m_array->m_data[m_index]; }
#if defined(TARRAY_OOL_INC)
        Iterator& operator++();
        Iterator& DummyInc()
#elif ITERKIND == 1
        Iterator operator++()                // returns a COPY (old style)
#elif ITERKIND == 2
        void operator++()
#else
        Iterator& operator++()
#endif
        {
            m_index = m_index + 1;
            if (m_array->m_count <= m_index || m_index == 0)
                m_index = -1;
#if ITERKIND != 2
            return *this;
#endif
        }
        Iterator operator++(int) { Iterator old = *this; ++*this; return old; }
#if defined(TARRAY_OOL_NE)
        bool operator!=(const Iterator& o) const;
#else
        bool operator!=(const Iterator& o) const { return m_index != o.m_index; }
#endif
        bool operator!=(int index) const { return m_index != index; }
        bool IsValid() const { return m_index != -1; }
        int     m_index;
        TArray* m_array;
    };
#if defined(TARRAY_OOL_BEGIN)
    Iterator Begin();
    Iterator End();
#elif defined(TARRAY_NAMED_RETURN)
    Iterator Begin() { Iterator it(this, (m_count == 0) ? -1 : 0); return it; }
    Iterator End() { Iterator it(this, -1); return it; }
#else
    Iterator Begin() { return Iterator(this, (m_count == 0) ? -1 : 0); }
    Iterator End() { return Iterator(this, -1); }
#endif
    T& operator[](const Iterator& it) { return m_data[it.m_index]; }
    T& operator[](int i) { return m_data[i]; }

    int Count() const { return m_count; }

    // Binary search in an array kept sorted with T::Compare. Returns the insert position;
    // on a hit it is just past the run of equal elements and found is set.
    Iterator FindSorted(const T& key, bool& found)
    {
        int lo = 0;
        int hi = m_count - 1;
        found = false;
        while (lo <= hi)
        {
            int mid = (lo + hi) / 2;
            int c = TArrayCompare(key, m_data[mid]);
            if (c < 0)
                hi = mid - 1;
            else if (c > 0)
                lo = mid + 1;
            else
            {
                for (mid++; mid <= hi; mid++)
                {
                    if (TArrayCompare(key, m_data[mid]) != 0)
                        break;
                }
                found = true;
                return Iterator(this, mid);
            }
        }
        return Iterator(this, lo);
    }

    // Opens a gap of n (default-constructed) elements at index.
    // first element equal to key (binary search, then back up over equal neighbours), or End()
    Iterator Find(const T& key)
    {
        int lo = 0;
        int hi = m_count - 1;
        while (lo <= hi)
        {
            int mid = (lo + hi) / 2;
            int c = TArrayCompare(key, m_data[mid]);
            if (c < 0)
                hi = mid - 1;
            else if (c > 0)
                lo = mid + 1;
            else
            {
                for (mid--; mid >= lo; mid--)
                {
                    if (TArrayCompare(key, m_data[mid]) != 0)
                        break;
                }
                return Iterator(this, mid + 1);
            }
        }
        return Iterator(this, -1);
    }

    void InsertAt(int index, int n)
    {
        if (m_count + n > m_capacity)
        {
            int capacity = m_capacity + Max(n, m_grow);
            T* data = new T[capacity];
            if (m_data)
            {
                if (index > 0)
                {
                    for (int i = 0; i < index; i++)
                        data[i] = m_data[i];
                }
                if (m_count - index > 0)
                {
                    for (int i = index; i < m_count; i++)
                        data[i + n] = m_data[i];
                }
                delete[] m_data;
            }
            m_capacity = capacity;
            m_data = data;
            m_count += n;
        }
        else
        {
            if (index < m_count)
            {
                for (int i = m_count - 1; i >= index; i--)
                    m_data[i + n] = m_data[i];
            }
            m_count += n;
        }
    }

    void Clear() { if (m_data) tfree(m_data); m_data = 0; m_capacity = 0; m_count = 0; }

    void SetCapacity(int n)
    {
#if SETCAP == 6
        // TRenderInterface's copy (TResourceNameMap): new[]/delete[] instead of tmalloc/tfree
        if (n == 0)
        {
            if (m_data) delete[] m_data;
            m_data = 0; m_capacity = 0; m_count = 0;
        }
        else
        {
            T* p = new T[n];
            int c = Min(m_count, n);
            for (int i = 0; i < c; i++)
                p[i] = m_data[i];
            m_capacity = n;
            if (m_data) delete[] m_data;
            m_data = p;
        }
#else
        if (n == 0)
        {
            if (m_data) tfree(m_data);
            m_data = 0; m_capacity = 0; m_count = 0;
        }
        else
        {
#if SETCAP == 1
            T* p = (T*)tmalloc(n * (int)sizeof(T), 0, -1);
            for (int i = 0; i < Min(m_count, n); i++)
                p[i] = m_data[i];
#elif SETCAP == 2
            T* p = (T*)tmalloc(n * (int)sizeof(T), 0, -1);
            int c = Min(m_count, n);
            int i;
            for (i = 0; i < c; i++)
                p[i] = m_data[i];
#elif SETCAP == 3
            T* p = (T*)tmalloc(sizeof(T) * n, 0, -1);
            int c = Min(m_count, n);
            for (int i = 0; i < c; i++)
                p[i] = m_data[i];
#elif SETCAP == 4
            T* p = (T*)tmalloc(n * (int)sizeof(T), 0, -1);
            int c = Min(n, m_count);
            for (int i = 0; i < c; i++)
                p[i] = m_data[i];
#elif SETCAP == 5
            T* p = (T*)tmalloc(n * (int)sizeof(T), 0, -1);
            int c = Min(m_count, n);
            for (int i = 0; i < c; i++)
                p[i] = (*this)[i];
#else
            T* p = (T*)tmalloc(n * (int)sizeof(T), 0, -1);
            int c = Min(m_count, n);
            for (int i = 0; i < c; i++)
                p[i] = m_data[i];
#endif
            m_capacity = n;
            if (m_data) tfree(m_data);
            m_data = p;
        }
#endif
    }

    // Appends one (uninitialised) slot and returns it.
#if SETCAP == 6
    T& AddEmpty()
    {
        if (m_count + 1 > m_capacity)
            SetCapacity(Max(m_capacity + m_grow, m_count + 1));
        return m_data[m_count++];
    }
#else
    T& AddEmpty()
    {
        int n = m_count + 1;
        if (m_capacity < n)
        {
            int g = m_capacity + m_grow;
            SetCapacity(Max(g, n));
            n = m_count + 1;
        }
        m_count = n;
        return m_data[n - 1];
    }
#endif

    // Removes element i by moving the last element into its place.
    void Pop() { if (m_count > 0) m_count--; }
    T& Last() { return m_data[m_count - 1]; }

    void RemoveFast(int i)
    {
        int n = m_count;
        if (n > 1) { m_data[i] = m_data[n - 1]; n = m_count; }
        if (n > 0) m_count = n - 1;
    }

    int m_grow;        // +0
    int m_count;       // +4
    int m_capacity;    // +8
    T*  m_data;        // +0xC

private:
    static const int& Min(const int& a, const int& b) { return (b < a) ? b : a; }
    static const int& Max(const int& a, const int& b) { return (a < b) ? b : a; }
};

// Non-inline (out-of-class) definitions, experiment: see MATCHING_NOTES #16/#21
#if defined(TARRAY_OOL_BEGIN)
template <class T> typename TArray<T>::Iterator TArray<T>::Begin() { return Iterator(this, (m_count == 0) ? -1 : 0); }
template <class T> typename TArray<T>::Iterator TArray<T>::End() { return Iterator(this, -1); }
#endif
#if defined(TARRAY_OOL_INC)
template <class T> typename TArray<T>::Iterator& TArray<T>::Iterator::operator++()
{
    m_index = m_index + 1;
    if (m_array->m_count <= m_index || m_index == 0)
        m_index = -1;
    return *this;
}
#endif
#if defined(TARRAY_OOL_NE)
template <class T> bool TArray<T>::Iterator::operator!=(const Iterator& o) const { return m_index != o.m_index; }
#endif
#if defined(TARRAY_OOL_DEREF)
template <class T> T& TArray<T>::Iterator::operator*() const { return m_array->m_data[m_index]; }
#endif

} // namespace Toshi
#endif
