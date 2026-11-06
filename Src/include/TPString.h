// Toshi pooled strings (stubs; the real classes live in TKernelInterface.dll).
// Imports seen in PPropertyParser.dll:
//   ?Delete@TPooledCString@Toshi@@AAEXXZ            private: void Delete()
//   ?Delete@TPooledWString@Toshi@@AAEXXZ
//   ?ms_sEmpty@TPCString@Toshi@@2VTCString@2@B      public: static const TCString ms_sEmpty
//   ?ms_sEmpty@TPWString@Toshi@@2VTWString@2@B
//   ?Compare@TPCString@Toshi@@QBEHABV12@@Z          int Compare(const TPCString&) const
//   ?Get@TCStringPool@Toshi@@QAE?AVTPCString@2@ABVTCString@2@@Z
//   ??HToshi@@YG?AVTCString@0@ABV10@ABVTPCString@0@@Z   TCString operator+(const TCString&, const TPCString&)
// A TPCString/TPWString is a 4-byte handle to a reference-counted pooled string
// (count at +0, the string object at +4). Everything except Compare is inline: the binary
// has the reference counting written out at each use, and ICC emits local copies of the
// (not inlined) assignment and destructor in each object file.
#ifndef TPSTRING_H
#define TPSTRING_H
#include "TMemory.h"
#include "TCString.h"

namespace Toshi {

class TPCString;
class TPWString;

class TPooledCString
{
    friend class TPCString;
public:
    int      m_iRefCount;                       // +0
    TCString m_oString;                         // +4
private:
    TOSHI_API void Delete();
};

class TPooledWString
{
    friend class TPWString;
public:
    int      m_iRefCount;                       // +0
    TWString m_oString;                         // +4
private:
    TOSHI_API void Delete();
};

class TPCString
{
public:
    TPCString() { m_pPooled = 0; }
    TPCString(const TPCString& o)
    {
        // clear first, then copy: matches PPropertyName's copy ctors (experiment D, round 16)
        m_pPooled = 0;
        if (o.m_pPooled)
        {
            m_pPooled = o.m_pPooled;
            if (m_pPooled) m_pPooled->m_iRefCount++;
        }
    }
    ~TPCString();                                   // non-inline: see MATCHING_NOTES #21
    TPCString& operator=(const TPCString& o);
    bool operator==(const TPCString& o) const { return m_pPooled == o.m_pPooled; }
    bool operator!=(const TPCString& o) const { return m_pPooled != o.m_pPooled; }
    const TCString& GetCString() const { return m_pPooled ? m_pPooled->m_oString : ms_sEmpty; }
    // the kernel also exports operator* and operator const TCString* (??D / ??B)
    const TCString& operator*() const
    {
        if (!m_pPooled) return ms_sEmpty;
        return m_pPooled->m_oString;
    }
    operator const TCString*() const
    {
        if (!m_pPooled) return &ms_sEmpty;
        return &m_pPooled->m_oString;
    }
    bool IsEmpty() const { return GetCString().IsEmpty(); }

    TOSHI_API int Compare(const TPCString& o) const;
    TOSHI_API static const TCString ms_sEmpty;

private:
    void Release()
    {
        if (m_pPooled)
        {
            if (--m_pPooled->m_iRefCount == 0)
                m_pPooled->Delete();
        }
        m_pPooled = 0;
    }
    TPooledCString* m_pPooled;
};
// These are defined here WITHOUT `inline` on purpose: the original object code calls a local
// out-of-line copy of them in each translation unit, which ICC only produces for non-inline
// functions (MATCHING_NOTES #21). Each .cpp is compiled and compared on its own; a linkable
// build will need these turned into per-file (static) copies - still to do.
// TPCString's destructor is never inlined. The assignments (and the TPWString/TLString
// destructors) were inlined in some of the original translation units and not in others; a
// .cpp that needs the out-of-line form defines TOSHI_STRING_ASSIGN_NOT_INLINED before its
// includes (PPropertyName, PPropertyReader and PPropertyValue do).
#ifdef TOSHI_STRING_ASSIGN_NOT_INLINED
#define TPCSTRING_ASSIGN_INLINE
#define TPWSTRING_INLINE
#else
#define TPCSTRING_ASSIGN_INLINE inline
#define TPWSTRING_INLINE inline
#endif
TPCString::~TPCString()
{
    if (m_pPooled) Release();
}
TPCSTRING_ASSIGN_INLINE TPCString& TPCString::operator=(const TPCString& o)
{
    TPooledCString* p = o.m_pPooled;
    if (m_pPooled != p)
    {
        if (m_pPooled)
        {
            if (--m_pPooled->m_iRefCount == 0)
                m_pPooled->Delete();
        }
        m_pPooled = o.m_pPooled;
        if (m_pPooled) m_pPooled->m_iRefCount++;
    }
    return *this;
}

class TPWString
{
public:
    TPWString() { m_pPooled = 0; }
    TPWString(const TPWString& o)
    {
        // clear first, then copy: matches PPropertyName's copy ctors (experiment D, round 16)
        m_pPooled = 0;
        if (o.m_pPooled)
        {
            m_pPooled = o.m_pPooled;
            if (m_pPooled) m_pPooled->m_iRefCount++;
        }
    }
    ~TPWString();
    TPWString& operator=(const TPWString& o);
    bool operator==(const TPWString& o) const { return m_pPooled == o.m_pPooled; }
    const TWString& GetWString() const { return m_pPooled ? m_pPooled->m_oString : ms_sEmpty; }
    bool IsEmpty() const { return GetWString().IsEmpty(); }

    TOSHI_API static const TWString ms_sEmpty;

private:
    void Release()
    {
        if (m_pPooled)
        {
            if (--m_pPooled->m_iRefCount == 0)
                m_pPooled->Delete();
        }
        m_pPooled = 0;
    }
    TPooledWString* m_pPooled;
};
TPWSTRING_INLINE TPWString::~TPWString()
{
    if (m_pPooled) Release();
}
TPWSTRING_INLINE TPWString& TPWString::operator=(const TPWString& o)
{
    TPooledWString* p = o.m_pPooled;
    if (m_pPooled != p)
    {
        if (m_pPooled)
        {
            if (--m_pPooled->m_iRefCount == 0)
                m_pPooled->Delete();
        }
        m_pPooled = o.m_pPooled;
        if (m_pPooled) m_pPooled->m_iRefCount++;
    }
    return *this;
}

class TCStringPool
{
public:
    TOSHI_API TPCString Get(const TCString& s);
    TOSHI_API TPCString Get(int i);
    TOSHI_API TPCString Get(const char* s);
    TOSHI_API TPCString GetOptional(const TCString& s);
    TOSHI_API TPCString GetOptional(int i);
    TOSHI_API TPCString GetOptional(const char* s);
    TOSHI_API TPCString GetVolatile(const TCString& s);
    TOSHI_API TPCString GetVolatile(int i);
    TOSHI_API TPCString GetVolatile(const char* s);
    TOSHI_API bool IsPresent(const TCString& s);
    TOSHI_API bool IsPresent(int i);
    TOSHI_API bool IsPresent(const char* s);
    TOSHI_API bool WriteFile(const char* name);
    TOSHI_API void Dump() const;
};

TOSHI_API TCString operator+(const TCString& a, const TPCString& b);

} // namespace Toshi
#endif
