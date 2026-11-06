// Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyName.cpp  (reconstruction)
// Range in PPropertyParser.dll: 0x2c70-0x322f. TPCString's assignment and destructor are
// not inlined here; ICC emits local copies (0x3150, 0x3190).
#define PPROPERTYPARSER_EXPORTS
#define TOSHI_STRING_ASSIGN_NOT_INLINED   // MATCHING_NOTES #21
#include "TNewDelete.h"
#include "PPropertyName.h"
#include "TSystem.h"

// "name" -> "name";  "name[sub]" -> "name" + "[" + "sub" + "]"
TCString operator+(const TCString& s, const PPropertyName& name)
{
    TCString result(s);
    result += name.GetName().GetCString().GetString(0);
    if (!name.GetSubName().GetCString().IsEmpty())
        result += TCString("[") + name.GetSubName().GetCString().GetString(0) + "]";
    return result;
}

PPropertyName::PPropertyName(const PPropertyName& o)
    : m_oName(o.m_oName), m_oSubName(o.m_oSubName)
{
}

PPropertyName& PPropertyName::operator=(const PPropertyName& o)
{
    m_oName = o.m_oName;
    m_oSubName = o.m_oSubName;
    return *this;
}

bool PPropertyName::operator==(const PPropertyName& o) const
{
    if (m_oName.Compare(o.m_oName) != 0) return false;
    if (m_oSubName.Compare(o.m_oSubName) != 0) return false;
    return true;
}

bool PPropertyName::operator!=(const PPropertyName& o) const
{
    if (m_oName.Compare(o.m_oName) != 0) return true;
    return m_oSubName.Compare(o.m_oSubName) != 0;
}

bool PPropertyName::operator<(const PPropertyName& o) const
{
    int c = m_oName.Compare(o.m_oName);
    if (c < 0) return true;
    if (c > 0) return false;
    return m_oSubName.Compare(o.m_oSubName) < 0;
}

bool PPropertyName::operator>(const PPropertyName& o) const
{
    int c = m_oName.Compare(o.m_oName);
    if (c > 0) return true;
    if (c < 0) return false;
    return m_oSubName.Compare(o.m_oSubName) > 0;
}

TPCString PPropertyName::GetString() const
{
    TWString str(m_oName.GetCString().GetString(0));
    if (!m_oSubName.GetCString().IsEmpty())
        str += TWString(TCString(L"[") + m_oSubName + L"]");
    return TSystem::GetCStringPool()->Get(TCString(str));
}

void PPropertyName::Split(const TPCString& s)
{
    int open = s.GetCString().Find('[', 0);
    if (open == -1)
    {
        m_oName = s;
        return;
    }
    m_oName = TSystem::GetCStringPool()->Get(s.GetCString().Mid(0, open));
    int close = s.GetCString().Find(']', open);
    if (close != -1)
        m_oSubName = TSystem::GetCStringPool()->Get(s.GetCString().Mid(open + 1, close - open - 1));
}

PPropertyName::PPropertyName(const TPCString& name, const TPCString& subName)
    : m_oName(name), m_oSubName(subName)
{
}

PPropertyName::PPropertyName(const TPCString& name)
{
    Split(name);
}

PPropertyName::PPropertyName()
{
}
