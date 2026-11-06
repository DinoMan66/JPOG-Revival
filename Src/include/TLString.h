// Toshi localised strings (stubs; the real classes live in TKernelInterface.dll).
// Imports seen in TInputDXInterface.dll:
//   ?Delete@TLocalisedString@Toshi@@AAEXXZ                       private: void Delete()
//   ??0TWString@Toshi@@QAE@PBD@Z   ??1TWString@Toshi@@QAE@XZ
//   ?GetTemporary@TLocale@Toshi@@QAE?AVTLString@2@ABVTWString@2@@Z
//   ?GetLocale@TSystem@Toshi@@SGPAVTLocale@2@XZ
// TLString is a 4-byte handle to a reference-counted TLocalisedString (count at +0); all of
// its members are inline (the binary has the reference counting written out at each use).
#ifndef TLSTRING_H
#define TLSTRING_H
#include "TMemory.h"
#include "TCString.h"
#include "TPString.h"
#include "TSystem.h"

namespace Toshi {

class TLString;

class TLocalisedString
{
    friend class TLString;
public:
    TOSHI_API bool IsTemporary() const;

    int       m_iRefCount;                      // +0
    int       m_iIndex;                         // +4 numbered string index
    TPCString m_oIdentifier;                    // +8 named string identifier
    TPWString m_oString;                        // +0xC
private:
    TOSHI_API void Delete();
};

class TLString
{
public:
    TLString() { m_pData = 0; }
    TLString(const TLString& o)
    {
        // the binary re-tests the pointer after storing it (see MATCHING_NOTES #6)
        if (o.m_pData)
        {
            m_pData = o.m_pData;
            if (m_pData) m_pData->m_iRefCount++;
        }
        else
            m_pData = 0;
    }
    ~TLString();
    TLString& operator=(const TLString& o);
    bool IsNull() const { return m_pData == 0; }
    const TWString& GetWString() const { return m_pData ? m_pData->m_oString.GetWString() : ms_wsEmpty; }
    bool IsEmpty() const { return GetWString().IsEmpty(); }
    bool IsTemporary() const { return m_pData ? m_pData->IsTemporary() : false; }
    const TWString& GetLocalisedString() const { return GetWString(); }
    const TCString& GetIdentifierString() const { return m_pData ? m_pData->m_oIdentifier.GetCString() : ms_csEmpty; }
    int GetIndex() const { return m_pData->m_iIndex; }
    bool operator==(const TLString& o) const { return m_pData == o.m_pData; }

    TOSHI_API static const TWString ms_wsEmpty;
    TOSHI_API static const TCString ms_csEmpty;

private:
    void Release()
    {
        if (m_pData)
        {
            if (--m_pData->m_iRefCount == 0)
                m_pData->Delete();
        }
        m_pData = 0;
    }
    TLocalisedString* m_pData;
};
TPWSTRING_INLINE TLString::~TLString()
{
    if (m_pData) Release();
}
TPWSTRING_INLINE TLString& TLString::operator=(const TLString& o)
{
    TLocalisedString* p = o.m_pData;
    if (m_pData != p)
    {
        if (m_pData)
        {
            if (--m_pData->m_iRefCount == 0)
                m_pData->Delete();
        }
        m_pData = o.m_pData;
        if (m_pData) m_pData->m_iRefCount++;
    }
    return *this;
}


class TLocale
{
public:
    TOSHI_API TLString GetTemporary(const TWString& s);
    TOSHI_API TLString Get(const TPCString& name);
    TOSHI_API TLString Get(int index);
    TOSHI_API TLString Get(const char* name);
    TOSHI_API TLString GetOptional(const TPCString& name);
    TOSHI_API TLString GetOptional(const char* name);
    TOSHI_API bool IsValid(const TPCString& name);
    TOSHI_API bool IsValid(int index);
    TOSHI_API bool IsValid(const char* name);
};

} // namespace Toshi
#endif
