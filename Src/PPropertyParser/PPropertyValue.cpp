// Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyValue.cpp  (reconstruction)
// Range in PPropertyParser.dll: 0x5f30-~0x7f3f. Also holds PPropertyValueArray's
// out-of-line members, the TYPE_xxx class objects and local copies of the TLString /
// TPWString / TPCString assignment and destructor.
#define PPROPERTYPARSER_EXPORTS
#define TOSHI_STRING_ASSIGN_NOT_INLINED   // MATCHING_NOTES #21
#include "TNewDelete.h"
#include "PPropertyValue.h"
#include "PProperties.h"

// ---------------------------------------------------------------- type objects
// Plain TClass objects that only carry a name (no parent, no factory functions).
static TClass s_oTypeInt("INT", 0, 0, 0, 0, 0, 0x10000);
static TClass s_oTypeUINT32("UINT32", 0, 0, 0, 0, 0, 0x10000);
static TClass s_oTypeFloat("FLOAT", 0, 0, 0, 0, 0, 0x10000);
static TClass s_oTypeBool("BOOL", 0, 0, 0, 0, 0, 0x10000);
static TClass s_oTypeTPCString("TPCString", 0, 0, 0, 0, 0, 0x10000);
static TClass s_oTypeTPWString("TPWString", 0, 0, 0, 0, 0, 0x10000);
static TClass s_oTypeTLString("TLString", 0, 0, 0, 0, 0, 0x10000);
static TClass s_oTypePropName("PPropertyName", 0, 0, 0, 0, 0, 0x10000);
static TClass s_oTypeArray("PPropertyValueArray", 0, 0, 0, 0, 0, 0x10000);

PPV_TYPEDECL PPropertyValue::TYPE_INT       = &s_oTypeInt;
PPV_TYPEDECL PPropertyValue::TYPE_UINT32    = &s_oTypeUINT32;
PPV_TYPEDECL PPropertyValue::TYPE_FLOAT     = &s_oTypeFloat;
PPV_TYPEDECL PPropertyValue::TYPE_BOOL      = &s_oTypeBool;
PPV_TYPEDECL PPropertyValue::TYPE_TPCSTRING = &s_oTypeTPCString;
PPV_TYPEDECL PPropertyValue::TYPE_TPWSTRING = &s_oTypeTPWString;
PPV_TYPEDECL PPropertyValue::TYPE_TLSTRING  = &s_oTypeTLString;
PPV_TYPEDECL PPropertyValue::TYPE_PROPS     = &PProperties::m_sClass;
PPV_TYPEDECL PPropertyValue::TYPE_PROPNAME  = &s_oTypePropName;
PPV_TYPEDECL PPropertyValue::TYPE_ARRAY     = &s_oTypeArray;
PPV_TYPEDECL PPropertyValue::TYPE_UNDEF     = 0;

// ---------------------------------------------------------------- construction

PPropertyValue::PPropertyValue(const TPCString& value)
{
    m_pType = TYPE_TPCSTRING;
    new (&m_uValue) TPCString;
    GetTPCString() = value;
}

PPropertyValue::PPropertyValue(const TPWString& value)
{
    m_pType = TYPE_TPWSTRING;
    new (&m_uValue) TPWString;
    GetTPWString() = value;
}

PPropertyValue::PPropertyValue(const TLString& value)
{
    m_pType = TYPE_TLSTRING;
    new (&m_uValue) TLString;
    GetTLString() = value;
}

PPropertyValue::PPropertyValue(PProperties* value)
{
    m_pType = TYPE_PROPS;
    new (&m_uValue) TManagedPtr<PProperties>;
    GetPropertiesMP() = value;
}

PPropertyValue::PPropertyValue(const PPropertyName& value)
{
    m_pType = TYPE_PROPNAME;
    m_uValue.name = new PPropertyName(value);
}

PPropertyValue::PPropertyValue(PPropertyValueArray* value)
{
    m_pType = TYPE_ARRAY;
    new (&m_uValue) TManagedPtr<PPropertyValueArray>;
    GetPropArrayMP() = value;
}

PPropertyValue::PPropertyValue(TObject* value)
{
    m_pType = value ? &value->GetClass() : TYPE_UNDEF;
    m_uValue.obj = value;
}

PPropertyValue::~PPropertyValue()
{
    if (m_pType == TYPE_TPCSTRING)
        GetTPCString().~TPCString();
    else if (m_pType == TYPE_TPWSTRING)
        GetTPWString().~TPWString();
    else if (m_pType == TYPE_TLSTRING)
        GetTLString().~TLString();
    else if (m_pType == TYPE_PROPS)
        GetPropertiesMP().~TManagedPtr<PProperties>();
    else if (m_pType == TYPE_ARRAY)
        GetPropArrayMP().~TManagedPtr<PPropertyValueArray>();
    else if (m_pType == TYPE_PROPNAME)
        delete m_uValue.name;
}

// ---------------------------------------------------------------- comparison

bool PPropertyValue::operator==(const TPCString& value) const
{
    return m_pType == TYPE_TPCSTRING && GetTPCString() == value;
}

bool PPropertyValue::operator==(const TPWString& value) const
{
    return m_pType == TYPE_TPWSTRING && GetTPWString() == value;
}

bool PPropertyValue::operator==(const TLString& value) const
{
    return m_pType == TYPE_TLSTRING && GetTLString() == value;
}

bool PPropertyValue::operator==(const char* value) const
{
    return m_pType == TYPE_TPCSTRING && GetTPCString().GetCString().Compare(value, -1) == 0;
}

bool PPropertyValue::operator==(const unsigned short* value) const
{
    return m_pType == TYPE_TPWSTRING && GetTPWString().GetWString().Compare(value, -1) == 0;
}

bool PPropertyValue::operator==(const PPropertyValue& o) const
{
    if (m_pType != o.m_pType) return false;
    if (m_pType == TYPE_UNDEF) return true;
    else if (m_pType == TYPE_TPCSTRING) return GetTPCString() == o.GetTPCString();
    else if (m_pType == TYPE_TPWSTRING) return GetTPWString() == o.GetTPWString();
    else if (m_pType == TYPE_TLSTRING) return GetTLString() == o.GetTLString();
    else if (m_pType == TYPE_INT) return m_uValue.i == o.m_uValue.i;
    else if (m_pType == TYPE_UINT32) return m_uValue.u == o.m_uValue.u;
    else if (m_pType == TYPE_FLOAT) return m_uValue.f == o.m_uValue.f;
    else if (m_pType == TYPE_BOOL) return m_uValue.b == o.m_uValue.b;
    else if (m_pType == TYPE_PROPS) return GetProperties() == o.GetProperties();
    else if (m_pType == TYPE_PROPNAME) return *m_uValue.name == *o.m_uValue.name;
    else if (m_pType == TYPE_ARRAY) return GetArray() == o.GetArray();
    return m_uValue.obj == o.m_uValue.obj;
}

bool PPropertyValue::operator==(const TObject* value) const
{
    return &value->GetClass() == m_pType && value == m_uValue.obj;
}

bool PPropertyValue::IsEmpty() const
{
    if (m_pType == TYPE_TPCSTRING) return GetTPCString().IsEmpty();
    else if (m_pType == TYPE_TPWSTRING) return GetTPWString().IsEmpty();
    else if (m_pType == TYPE_TLSTRING) return GetTLString().IsEmpty();
    else if (m_pType == TYPE_INT) return false;
    else if (m_pType == TYPE_UINT32) return false;
    else if (m_pType == TYPE_FLOAT) return false;
    else if (m_pType == TYPE_BOOL) return false;
    else if (m_pType == TYPE_PROPS) return GetProperties() == 0;
    else if (m_pType == TYPE_PROPNAME) return m_uValue.name == 0 || m_uValue.name->GetName().IsEmpty();
    else if (m_pType == TYPE_ARRAY) return GetArray() == 0 || GetArray()->m_oValues.m_count == 0;
    else if (m_pType == TYPE_UNDEF) return true;
    return m_uValue.obj == 0;
}

// Integers convert to FLOAT only below 1e8; a float converts to INT only when it is whole.
bool PPropertyValue::CanBeType(const TClass* type) const
{
    if (m_pType == TYPE_INT)
        return type == TYPE_INT || type == TYPE_UINT32 || (type == TYPE_FLOAT && m_uValue.i < 100000000.0f);
    else if (m_pType == TYPE_UINT32)
        return type == TYPE_INT || type == TYPE_UINT32 || (type == TYPE_FLOAT && m_uValue.i < 100000000.0f);
    else if (m_pType == TYPE_FLOAT)
        return type == TYPE_FLOAT || (type == TYPE_INT && (float)(int)m_uValue.f == m_uValue.f);
    else if (m_pType == TYPE_BOOL)
        return type == TYPE_BOOL || type == TYPE_INT || type == TYPE_FLOAT;
    else if (m_pType == TYPE_TPCSTRING)
        return type == TYPE_TPCSTRING;
    return type == m_pType;
}

bool PPropertyValue::GetBoolean() const
{
    if (m_pType == TYPE_INT) return m_uValue.i != 0;
    if (m_pType == TYPE_UINT32) return m_uValue.u != 0;
    if (m_pType == TYPE_FLOAT) return m_uValue.f != 0.0f;
    if (m_pType == TYPE_BOOL) return m_uValue.b;
    return false;
}

// ---------------------------------------------------------------- string access
// A value of another type yields a (function-local) empty string.

TPCString& PPropertyValue::GetTPCString()
{
    static TPCString s_oEmpty;
    if (m_pType == TYPE_TPCSTRING) return *(TPCString*)&m_uValue;
    return s_oEmpty;
}

TPWString& PPropertyValue::GetTPWString()
{
    static TPWString s_oEmpty;
    if (m_pType == TYPE_TPWSTRING) return *(TPWString*)&m_uValue;
    return s_oEmpty;
}

TLString& PPropertyValue::GetTLString()
{
    static TLString s_oEmpty;
    if (m_pType == TYPE_TLSTRING) return *(TLString*)&m_uValue;
    return s_oEmpty;
}

const TPCString& PPropertyValue::GetTPCString() const
{
    static TPCString s_oEmpty;
    if (m_pType == TYPE_TPCSTRING) return *(const TPCString*)&m_uValue;
    return s_oEmpty;
}

const TPWString& PPropertyValue::GetTPWString() const
{
    static TPWString s_oEmpty;
    if (m_pType == TYPE_TPWSTRING) return *(const TPWString*)&m_uValue;
    return s_oEmpty;
}

const TLString& PPropertyValue::GetTLString() const
{
    static TLString s_oEmpty;
    if (m_pType == TYPE_TLSTRING) return *(const TLString*)&m_uValue;
    return s_oEmpty;
}

// ---------------------------------------------------------------- changing the type

void PPropertyValue::SetPropertyName(const PPropertyName& value)
{
    ChangeType(TYPE_PROPNAME);
    m_uValue.name = new PPropertyName(value);
}

void PPropertyValue::SetTObject(TObject* value)
{
    if (value)
        ChangeType(&value->GetClass());
    else
        ChangeType(TYPE_UNDEF);
    m_uValue.obj = value;
}

// Destroys the current value and default-constructs one of the new type.
bool PPropertyValue::ChangeType(const TClass* type)
{
    if (m_pType == type) return false;

    if (m_pType == TYPE_TPCSTRING)
        GetTPCString().~TPCString();
    else if (m_pType == TYPE_TPWSTRING)
        GetTPWString().~TPWString();
    else if (m_pType == TYPE_TLSTRING)
        GetTLString().~TLString();
    else if (m_pType == TYPE_PROPS)
        GetPropertiesMP().~TManagedPtr<PProperties>();
    else if (m_pType == TYPE_ARRAY)
        GetPropArrayMP().~TManagedPtr<PPropertyValueArray>();
    else if (m_pType == TYPE_PROPNAME)
        delete m_uValue.name;

    if (type == TYPE_TPCSTRING)
        new (&m_uValue) TPCString;
    else if (type == TYPE_TPWSTRING)
        new (&m_uValue) TPWString;
    else if (type == TYPE_TLSTRING)
        new (&m_uValue) TLString;
    else if (type == TYPE_PROPS)
        new (&m_uValue) TManagedPtr<PProperties>;
    else if (type == TYPE_ARRAY)
        new (&m_uValue) TManagedPtr<PPropertyValueArray>;
    else if (type == TYPE_PROPNAME)
        m_uValue.name = 0;

    m_pType = type;
    return true;
}

void PPropertyValue::Assign(const PPropertyValue& o)
{
    ChangeType(o.m_pType);
    if (m_pType == TYPE_TPCSTRING)
        GetTPCString() = o.GetTPCString();
    else if (m_pType == TYPE_TPWSTRING)
        GetTPWString() = o.GetTPWString();
    else if (m_pType == TYPE_TLSTRING)
        GetTLString() = o.GetTLString();
    else if (m_pType == TYPE_INT)
        m_uValue.i = o.m_uValue.i;
    else if (m_pType == TYPE_UINT32)
        m_uValue.u = o.m_uValue.u;
    else if (m_pType == TYPE_FLOAT)
        m_uValue.f = o.m_uValue.f;
    else if (m_pType == TYPE_BOOL)
        m_uValue.b = o.m_uValue.b;
    else if (m_pType == TYPE_PROPS)
        GetPropertiesMP() = o.GetProperties();
    else if (m_pType == TYPE_PROPNAME)
        m_uValue.name = new PPropertyName(*o.m_uValue.name);
    else if (m_pType == TYPE_ARRAY)
        GetPropArrayMP() = o.m_uValue.arr;
    else if (m_pType != TYPE_UNDEF)
        m_uValue.obj = o.m_uValue.obj;
}

// ---------------------------------------------------------------- PPropertyValueArray

PPropertyValueArray::PPropertyValueArray(PPropertyValue* values, int count)
    : m_iRefCount(0), m_oValues(1, count)
{
    for (int i = 0; i < count; i++)
        m_oValues.m_data[i] = values[i];
}

bool PPropertyValueArray::GetTVector2(TVector2& v) const
{
    if (!CanBeTypeArray(PPropertyValue::TYPE_FLOAT, 2)) return false;
    v.x = m_oValues.m_data[0].GetFloat();
    v.y = m_oValues.m_data[1].GetFloat();
    return true;
}

bool PPropertyValueArray::GetTVector3(TVector3& v) const
{
    if (!CanBeTypeArray(PPropertyValue::TYPE_FLOAT, 3)) return false;
    v.x = m_oValues.m_data[0].GetFloat();
    v.y = m_oValues.m_data[1].GetFloat();
    v.z = m_oValues.m_data[2].GetFloat();
    return true;
}

bool PPropertyValueArray::GetTQuaternion(TQuaternion& q) const
{
    if (!CanBeTypeArray(PPropertyValue::TYPE_FLOAT, 4)) return false;
    q.x = m_oValues.m_data[0].GetFloat();
    q.y = m_oValues.m_data[1].GetFloat();
    q.z = m_oValues.m_data[2].GetFloat();
    q.w = m_oValues.m_data[3].GetFloat();
    return true;
}

// count == -1: any size
bool PPropertyValueArray::CanBeTypeArray(const TClass* type, int count) const
{
    int size = m_oValues.m_count;
    if (count != -1 && size != count) return false;
    for (int i = 0; i < size; i++)
    {
        if (!m_oValues.m_data[i].CanBeType(type)) return false;
    }
    return true;
}

// ---------------------------------------------------------------- PPropertyValue (cont.)

TManagedPtr<PPropertyValueArray>& PPropertyValue::GetPropArrayMP()
{
    return *(TManagedPtr<PPropertyValueArray>*)&m_uValue;
}

TManagedPtr<PProperties>& PPropertyValue::GetPropertiesMP()
{
    return *(TManagedPtr<PProperties>*)&m_uValue;
}

const char* PPropertyValue::ToString(const TClass* type)
{
    if (type == TYPE_UNDEF) return "TNULL";
    return type->GetName();
}

void PPropertyValue::SetArray(PPropertyValueArray* value)
{
    ChangeType(TYPE_ARRAY);
    GetPropArrayMP() = value;
}

void PPropertyValue::SetProperties(PProperties* value)
{
    ChangeType(TYPE_PROPS);
    GetPropertiesMP() = value;
}

void PPropertyValue::SetTLString(const TLString& value)
{
    ChangeType(TYPE_TLSTRING);
    GetTLString() = value;
}

void PPropertyValue::SetTPWString(const TPWString& value)
{
    ChangeType(TYPE_TPWSTRING);
    GetTPWString() = value;
}

void PPropertyValue::SetTPCString(const TPCString& value)
{
    ChangeType(TYPE_TPCSTRING);
    GetTPCString() = value;
}

void PPropertyValue::SetBoolean(bool value)
{
    ChangeType(TYPE_BOOL);
    m_uValue.b = value;
}

void PPropertyValue::SetFloat(float value)
{
    ChangeType(TYPE_FLOAT);
    m_uValue.f = value;
}

void PPropertyValue::SetUINT32(unsigned int value)
{
    ChangeType(TYPE_UINT32);
    m_uValue.u = value;
}

void PPropertyValue::SetInteger(int value)
{
    ChangeType(TYPE_INT);
    m_uValue.i = value;
}

void PPropertyValue::SetUndef()
{
    ChangeType(TYPE_UNDEF);
}

TObject* PPropertyValue::GetTObject() const
{
    if (m_pType == TYPE_UNDEF) return 0;
    return m_uValue.obj;
}

PPropertyValueArray* PPropertyValue::GetArray() const
{
    return m_uValue.arr;
}

const PPropertyName& PPropertyValue::GetPropertyName() const
{
    return *m_uValue.name;
}

PProperties* PPropertyValue::GetProperties() const
{
    if (m_pType == TYPE_PROPS) return m_uValue.props;
    return 0;
}

float PPropertyValue::GetFloat() const
{
    if (m_pType == TYPE_INT) return (float)m_uValue.i;
    if (m_pType == TYPE_UINT32) return (float)m_uValue.u;
    if (m_pType == TYPE_FLOAT) return m_uValue.f;
    return 0.0f;
}

unsigned int PPropertyValue::GetUINT32() const
{
    if (m_pType == TYPE_INT) return m_uValue.i;
    if (m_pType == TYPE_UINT32) return m_uValue.u;
    if (m_pType == TYPE_FLOAT) return (unsigned int)m_uValue.f;
    return 0;
}

int PPropertyValue::GetInteger() const
{
    if (m_pType == TYPE_INT) return m_uValue.i;
    if (m_pType == TYPE_UINT32) return m_uValue.u;
    if (m_pType == TYPE_FLOAT) return (int)m_uValue.f;
    return 0;
}

const TClass* PPropertyValue::GetType() const
{
    return m_pType;
}

bool PPropertyValue::IsType(const TClass* type) const
{
    return m_pType == type;
}

bool PPropertyValue::IsDefined() const
{
    return m_pType != TYPE_UNDEF;
}

bool PPropertyValue::operator!=(const PPropertyValue& o) const
{
    return !(*this == o);
}

bool PPropertyValue::operator==(const PPropertyValueArray* value) const
{
    return m_pType == TYPE_ARRAY && GetArray() == value;
}

bool PPropertyValue::operator==(const PPropertyName& value) const
{
    return m_pType == TYPE_PROPNAME && *m_uValue.name == value;
}

bool PPropertyValue::operator==(const PProperties* value) const
{
    return m_pType == TYPE_PROPS && GetProperties() == value;
}

bool PPropertyValue::operator==(bool value) const
{
    return m_pType == TYPE_BOOL && m_uValue.b == value;
}

bool PPropertyValue::operator==(float value) const
{
    return m_pType == TYPE_FLOAT && m_uValue.f == value;
}

bool PPropertyValue::operator==(unsigned int value) const
{
    return m_pType == TYPE_UINT32 && m_uValue.u == value;
}

bool PPropertyValue::operator==(int value) const
{
    return m_pType == TYPE_INT && m_uValue.i == value;
}

PPropertyValue& PPropertyValue::operator=(PPropertyValueArray* value)
{
    SetArray(value);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(const PPropertyValue& o)
{
    Assign(o);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(const PPropertyName& value)
{
    SetPropertyName(value);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(PProperties* value)
{
    SetProperties(value);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(float value)
{
    SetFloat(value);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(unsigned int value)
{
    SetUINT32(value);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(int value)
{
    SetInteger(value);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(const TLString& value)
{
    SetTLString(value);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(const TPWString& value)
{
    SetTPWString(value);
    return *this;
}

PPropertyValue& PPropertyValue::operator=(const TPCString& value)
{
    SetTPCString(value);
    return *this;
}

PPropertyValue::PPropertyValue(const PPropertyValue& o)
{
    m_pType = TYPE_UNDEF;
    Assign(o);
}

PPropertyValue::PPropertyValue(bool value)
{
    m_pType = TYPE_BOOL;
    m_uValue.b = value;
}

PPropertyValue::PPropertyValue(float value)
{
    m_pType = TYPE_FLOAT;
    m_uValue.f = value;
}

PPropertyValue::PPropertyValue(unsigned int value)
{
    m_pType = TYPE_UINT32;
    m_uValue.u = value;
}

PPropertyValue::PPropertyValue(int value)
{
    m_pType = TYPE_INT;
    m_uValue.i = value;
}

PPropertyValue::PPropertyValue()
{
    m_pType = TYPE_UNDEF;
}
