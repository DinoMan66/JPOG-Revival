// Z:\Toshi\Plugins\Source\PPropertyParser\PProperties.cpp  (reconstruction)
// Range in PPropertyParser.dll: 0x1000-0x2a5f. Besides the PProperties members it holds
// (emitted by the compiler) the out-of-line copies of every inline member of PProperty,
// PProperties, PPropertyValueArray and PPropertyName, the vector deleting destructors,
// TArray<PPropertyValue>::operator= and the global operator new/delete.
#define PPROPERTYPARSER_EXPORTS
#define TOSHI_NEWDELETE_OUTOFLINE
#include "TNewDelete.h"
#include "PProperties.h"
#include "TSystem.h"

// Non-exported, unreferenced (0x2990 new / 0x2980 delete) - this is the only object file
// that has them; the others inline tmalloc/tfree.
void* __cdecl operator new(size_t size)
{
    return tmalloc((int)size, 0, -1);
}

void __cdecl operator delete(void* p)
{
    tfree(p);
}

// One slot of the temporary array Sort() builds (12 bytes).
struct PPropertySortEntry
{
    PPropertySortEntry() {}
    PPropertySortEntry(const PPropertyName& name, PProperties::PProperty* property)
        : m_oName(name), m_pProperty(property) {}

    int Compare(const PPropertySortEntry& o) const
    {
        int c = m_oName.GetName().Compare(o.m_oName.GetName());
        if (c != 0) return c;
        if (m_oName.GetSubName().IsEmpty()) return -1;
        if (o.m_oName.GetSubName().IsEmpty()) return 1;
        return m_oName.GetSubName().Compare(o.m_oName.GetSubName());
    }

    PPropertyName           m_oName;
    PProperties::PProperty* m_pProperty;
};

// ---------------------------------------------------------------- PProperties

PProperties::PProperties()
    : m_pParent(0)
{
    m_iPropertyCount = 0;
}

PProperties::PProperties(PProperties* parent)
    : m_pParent(parent)
{
    m_iPropertyCount = 0;
}

PProperties::~PProperties()
{
    TQList<PProperty>::Iterator it = m_oPropertySet.Begin();
    while (it != m_oPropertySet.End())
    {
        PProperty* p = it;
        ++it;
        delete p;
    }
    m_iPropertyCount = 0;
}

void PProperties::PutProperty(const TPCString& name, const TPCString& subName, const PPropertyValue& value, const TPCString& comment)
{
    PutProperty(PPropertyName(name, subName), value, comment);
}

void PProperties::PutProperty(const PPropertyName& name, const PPropertyValue& value, const TPCString& comment)
{
    m_oPropertySet.InsertTail(new PProperty(name, value, comment));
    m_iPropertyCount++;
}

void PProperties::PutProperty(const PPropertyName& name, const PPropertyValue& value)
{
    m_oPropertySet.InsertTail(new PProperty(name, value));
    m_iPropertyCount++;
}

void PProperties::PutPropertyUnique(const TPCString& name, const TPCString& subName, const PPropertyValue& value, const TPCString& comment)
{
    PutPropertyUnique(PPropertyName(name, subName), value, comment);
}

// The original does not look for an existing property here: it inserts, exactly like PutProperty
// (0x1330 is a full copy of PutProperty's body, which is why the TPCString overload calls it, not PutProperty).
void PProperties::PutPropertyUnique(const PPropertyName& name, const PPropertyValue& value, const TPCString& comment)
{
    m_oPropertySet.InsertTail(new PProperty(name, value, comment));
    m_iPropertyCount++;
}

void PProperties::PutPropertyUnique(const PPropertyName& name, const PPropertyValue& value)
{
    for (TQList<PProperty>::Iterator it = m_oPropertySet.Begin(); it != m_oPropertySet.End(); ++it)
    {
        if (it->GetName() == name)
        {
            it->SetValue(value);
            return;
        }
    }
    PutProperty(name, value);
}

const PPropertyValue* PProperties::GetProperty(const PPropertyName& name) const
{
    TQList<PProperty>::Iterator end = m_oPropertySet.End();
    for (TQList<PProperty>::Iterator it = m_oPropertySet.Begin(); it != end; ++it)
    {
        if (it->GetName() == name)
            return it->GetValue();
    }
    return 0;
}

const PPropertyValue* PProperties::GetProperty(const TPCString& name, int index) const
{
    TCString str;
    str.Format("%i", index);
    return GetProperty(PPropertyName(name, TSystem::GetCStringPool()->Get(str)));
}

const PPropertyValue* PProperties::GetRequiredProperty(const TPCString& name, const TClass* type) const
{
    const PPropertyValue* value = GetProperty(PPropertyName(name));
    if (!value) return 0;
    if (value->GetType() != type) return 0;
    return value;
}

const PPropertyValue* PProperties::GetRequiredProperty(const TPCString& name, const TPCString& subName, const TClass* type) const
{
    const PPropertyValue* value = GetProperty(PPropertyName(name, subName));
    if (!value) return 0;
    if (value->GetType() != type) return 0;
    return value;
}

const PPropertyValue* PProperties::GetRequiredProperty(const TPCString& name, int index, const TClass* type) const
{
    const PPropertyValue* value = GetProperty(name, index);
    if (!value) return 0;
    if (value->GetType() != type) return 0;
    return value;
}

const PPropertyValue* PProperties::GetOptionalProperty(const TPCString& name, const TClass* type) const
{
    const PPropertyValue* value = GetProperty(PPropertyName(name));
    if (value && value->GetType() == type) return value;
    return 0;
}

const PPropertyValue* PProperties::GetOptionalProperty(const TPCString& name, const TPCString& subName, const TClass* type) const
{
    const PPropertyValue* value = GetProperty(PPropertyName(name, subName));
    if (value && value->GetType() == type) return value;
    return 0;
}

const PPropertyValue* PProperties::GetOptionalProperty(const TPCString& name, int index, const TClass* type) const
{
    const PPropertyValue* value = GetProperty(name, index);
    if (value && value->GetType() == type) return value;
    return 0;
}

const PProperties::PProperty* PProperties::FindProperty(const TPCString& name, TQList<PProperty>::Iterator& it) const
{
    while (it != m_oPropertySet.End())
    {
        if (it->GetName().GetName() == name)
        {
            PProperty* p = it;
            ++it;
            return p;
        }
        ++it;
    }
    return 0;
}

void PProperties::Sort()
{
    TArray<PPropertySortEntry> sorted(m_iPropertyCount, 0);
    while (!m_oPropertySet.IsEmpty())
    {
        PProperty* p = m_oPropertySet.RemoveHead();
        bool found = false;
        PPropertySortEntry entry(p->GetName(), p);
        TArray<PPropertySortEntry>::Iterator it = sorted.FindSorted(entry, found);
        sorted.InsertAt(it.m_index, 1);
        sorted[it] = entry;
    }
    for (TArray<PPropertySortEntry>::Iterator i = sorted.Begin(); i != sorted.End(); ++i)
        m_oPropertySet.InsertTail(i->m_pProperty);
}

// ---------------------------------------------------------------- PProperty

PProperties::PProperty::PProperty(const PPropertyName& name, const PPropertyValue& value)
    : m_oName(name), m_oValue(value)
{
}

PProperties::PProperty::PProperty(const PPropertyName& name, const PPropertyValue& value, const TPCString& comment)
    : m_oName(name), m_oValue(value), m_oComment(comment)
{
}

TOSHI_DEFINE_CLASS(PProperties, TObject)
TFreeList PProperties::ms_oFreelist(sizeof(PProperties), 0, 8, "PProperties");
TFreeList PProperties::PProperty::ms_oFreelist(sizeof(PProperties::PProperty), 0, 8, "PProperties::PProperty");

// ---------------------------------------------------------------- convenience overloads

const PPropertyValue* PProperties::GetProperty(const TPCString& name, const TPCString& subName) const
{
    return GetProperty(PPropertyName(name, subName));
}

const PPropertyValue* PProperties::GetProperty(const TPCString& name) const
{
    return GetProperty(PPropertyName(name));
}

void PProperties::PutPropertyUnique(const TPCString& name, const TPCString& subName, const PPropertyValue& value)
{
    PutPropertyUnique(PPropertyName(name, subName), value);
}

void PProperties::PutPropertyUnique(const TPCString& name, const PPropertyValue& value)
{
    PutPropertyUnique(PPropertyName(name), value);
}

void PProperties::PutPropertyUnique(const TPCString& name, const PPropertyValue& value, const TPCString& comment)
{
    PutPropertyUnique(PPropertyName(name), value, comment);
}

void PProperties::PutProperty(const TPCString& name, const TPCString& subName, const PPropertyValue& value)
{
    PutProperty(PPropertyName(name, subName), value);
}

void PProperties::PutProperty(const TPCString& name, const PPropertyValue& value)
{
    PutProperty(PPropertyName(name), value);
}

void PProperties::PutProperty(const TPCString& name, const PPropertyValue& value, const TPCString& comment)
{
    PutProperty(PPropertyName(name), value, comment);
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(PProperties)
