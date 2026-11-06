// PPropertyValue / PPropertyValueArray (PPropertyParser.dll)
// Source: Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyValue.cpp
//
// PPropertyValue (8 bytes) = { const TClass* m_pType; 4-byte value }.
// The type is one of the TYPE_xxx class objects (TYPE_PROPS is PProperties::m_sClass,
// TYPE_UNDEF is null, a TObject value uses the object's own class). String and pointer
// values are constructed in place in the value slot (placement new) and destroyed by hand.
//
// PPropertyValueArray (0x14 bytes) = { int m_iRefCount; TArray<PPropertyValue> } - held by
// TManagedPtr, Delete() is "delete this".
#ifndef PPROPERTYVALUE_H
#define PPROPERTYVALUE_H
#include "PPropertyParserAPI.h"
#include "TObject.h"
#include "TPString.h"
#include "TLString.h"
#include "TManagedPtr.h"
#include "TArray.h"
#include "TVector.h"
#include "PPropertyName.h"

class PProperties;
class PPropertyValueArray;

class PPROPERTYPARSER_API PPropertyValue
{
public:
    // Declared WITHOUT the second const: ICC then mangles them exactly like the original export
    // (?TYPE_x@PPropertyValue@@2PBVTClass@Toshi@@B) and loads them from memory at every use
    // instead of constant-folding the address (experiment C, round 15: +8 matches).
#define PPV_TYPEDECL const Toshi::TClass*
#define PPV_TYPEDECL_IN const Toshi::TClass*
    static PPV_TYPEDECL_IN TYPE_INT;
    static PPV_TYPEDECL_IN TYPE_UINT32;
    static PPV_TYPEDECL_IN TYPE_FLOAT;
    static PPV_TYPEDECL_IN TYPE_BOOL;
    static PPV_TYPEDECL_IN TYPE_TPCSTRING;
    static PPV_TYPEDECL_IN TYPE_TPWSTRING;
    static PPV_TYPEDECL_IN TYPE_TLSTRING;
    static PPV_TYPEDECL_IN TYPE_PROPS;
    static PPV_TYPEDECL_IN TYPE_PROPNAME;
    static PPV_TYPEDECL_IN TYPE_ARRAY;
    static PPV_TYPEDECL_IN TYPE_UNDEF;

    PPropertyValue();
    PPropertyValue(int value);
    PPropertyValue(unsigned int value);
    PPropertyValue(float value);
    PPropertyValue(bool value);
    PPropertyValue(const PPropertyValue& o);
    PPropertyValue(const Toshi::TPCString& value);
    PPropertyValue(const Toshi::TPWString& value);
    PPropertyValue(const Toshi::TLString& value);
    PPropertyValue(PProperties* value);
    PPropertyValue(const PPropertyName& value);
    PPropertyValue(PPropertyValueArray* value);
    PPropertyValue(Toshi::TObject* value);
    ~PPropertyValue();

    PPropertyValue& operator=(const PPropertyValue& o);
    PPropertyValue& operator=(const Toshi::TPCString& value);
    PPropertyValue& operator=(const Toshi::TPWString& value);
    PPropertyValue& operator=(const Toshi::TLString& value);
    PPropertyValue& operator=(int value);
    PPropertyValue& operator=(unsigned int value);
    PPropertyValue& operator=(float value);
    PPropertyValue& operator=(PProperties* value);
    PPropertyValue& operator=(const PPropertyName& value);
    PPropertyValue& operator=(PPropertyValueArray* value);

    bool operator==(const PPropertyValue& o) const;
    bool operator==(const Toshi::TPCString& value) const;
    bool operator==(const Toshi::TPWString& value) const;
    bool operator==(const Toshi::TLString& value) const;
    bool operator==(const char* value) const;
    bool operator==(const unsigned short* value) const;
    bool operator==(int value) const;
    bool operator==(unsigned int value) const;
    bool operator==(float value) const;
    bool operator==(bool value) const;
    bool operator==(const PProperties* value) const;
    bool operator==(const PPropertyName& value) const;
    bool operator==(const PPropertyValueArray* value) const;
    bool operator==(const Toshi::TObject* value) const;
    bool operator!=(const PPropertyValue& o) const;

    const Toshi::TClass* GetType() const;
    bool IsType(const Toshi::TClass* type) const;
    bool IsDefined() const;
    bool IsEmpty() const;
    bool CanBeType(const Toshi::TClass* type) const;
    static const char* ToString(const Toshi::TClass* type);

    int          GetInteger() const;
    unsigned int GetUINT32() const;
    float        GetFloat() const;
    bool         GetBoolean() const;
    Toshi::TPCString& GetTPCString();
    Toshi::TPWString& GetTPWString();
    Toshi::TLString&  GetTLString();
    const Toshi::TPCString& GetTPCString() const;
    const Toshi::TPWString& GetTPWString() const;
    const Toshi::TLString&  GetTLString() const;
    PProperties*         GetProperties() const;
    const PPropertyName& GetPropertyName() const;
    PPropertyValueArray* GetArray() const;
    Toshi::TObject*      GetTObject() const;

    void SetInteger(int value);
    void SetUINT32(unsigned int value);
    void SetFloat(float value);
    void SetBoolean(bool value);
    void SetTPCString(const Toshi::TPCString& value);
    void SetTPWString(const Toshi::TPWString& value);
    void SetTLString(const Toshi::TLString& value);
    void SetProperties(PProperties* value);
    void SetPropertyName(const PPropertyName& value);
    void SetArray(PPropertyValueArray* value);
    void SetTObject(Toshi::TObject* value);
    void SetUndef();

protected:
    bool ChangeType(const Toshi::TClass* type);
    void Assign(const PPropertyValue& o);
    Toshi::TManagedPtr<PPropertyValueArray>& GetPropArrayMP();
    Toshi::TManagedPtr<PProperties>& GetPropertiesMP();

private:
    const Toshi::TClass* m_pType;       // +0
    union                               // +4
    {
        int                  i;
        unsigned int         u;
        float                f;
        bool                 b;
        Toshi::TObject*      obj;
        PPropertyName*       name;
        PPropertyValueArray* arr;
        PProperties*         props;
    } m_uValue;
};

class PPROPERTYPARSER_API PPropertyValueArray
{
public:
    // inline; emitted in PProperties.cpp's range (reverse declaration order)
    PPropertyValueArray() : m_iRefCount(0), m_oValues(0, 0) {}
    PPropertyValueArray(int size) : m_iRefCount(0), m_oValues(size, size) {}
    PPropertyValueArray(const PPropertyValueArray& o) : m_iRefCount(0), m_oValues(o.m_oValues) {}
    void Delete() { delete this; }
    ~PPropertyValueArray() {}

    PPropertyValueArray(PPropertyValue* values, int count);
    bool CanBeTypeArray(const Toshi::TClass* type, int count) const;
    bool GetTVector2(Toshi::TVector2& v) const;
    bool GetTVector3(Toshi::TVector3& v) const;
    bool GetTQuaternion(Toshi::TQuaternion& q) const;

    int m_iRefCount;                                // +0
    Toshi::TArray<PPropertyValue> m_oValues;        // +4
};

#endif
