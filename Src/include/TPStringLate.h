// Late definitions of the pooled/localised string destructors and TPCString::operator=, for
// PPropertyReader.cpp. In the original that translation unit calls local out-of-line copies
// (0x57b0 ~TLString, 0x57e0 TPCString::operator=, 0x5820 ~TPCString) placed between
// OnLexerParseError and Warning(const TCString&): the bodies were only seen at that point,
// so ICC could not inline them into the functions above.
#ifndef TPSTRINGLATE_H
#define TPSTRINGLATE_H
#include "TPString.h"
#include "TLString.h"
namespace Toshi {

inline TPCString::~TPCString()
{
    if (m_pPooled) Release();
}
inline TPCString& TPCString::operator=(const TPCString& o)
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

inline TLString::~TLString()
{
    if (m_pData) Release();
}

} // namespace Toshi
#endif
