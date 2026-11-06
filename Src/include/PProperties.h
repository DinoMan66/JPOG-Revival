// PProperties - a ref-counted list of named property values (PPropertyParser.dll)
// Source: Z:\Toshi\Plugins\Source\PPropertyParser\PProperties.cpp
//
// Layout (0x18): vptr, m_iRefCount (+4), m_pParent (+8), TQList<PProperty> (+0xC, 8 bytes),
// m_iPropertyCount (+0x14). Both PProperties and PProperty come from TFreeLists.
// PProperty (0x1C): list node (+0), PPropertyName (+8), PPropertyValue (+0x10),
// TPCString comment (+0x18).
#ifndef PPROPERTIES_H
#define PPROPERTIES_H
#include "PPropertyParserAPI.h"
#include "TObject.h"
#include "TQList.h"
#include "TManagedPtr.h"
#include "TFreeList.h"
#include "PPropertyName.h"
#include "PPropertyValue.h"

class PPROPERTYPARSER_API PProperties : public Toshi::TObject, public Toshi::TRefCountBase
{
    TOSHI_DECLARE_CLASS(PProperties)
public:
    class PPROPERTYPARSER_API PProperty : public Toshi::TQList<PProperty>::TNode
    {
        TOSHI_FREELIST_DECLARE()
    public:
        PProperty(const PPropertyName& name, const PPropertyValue& value);
        PProperty(const PPropertyName& name, const PPropertyValue& value, const Toshi::TPCString& comment);

        // inline; ICC emits them in reverse order
        const PPropertyName& GetName() const { return m_oName; }
        const PPropertyValue* GetValue() const { return &m_oValue; }
        void SetValue(const PPropertyValue& value) { m_oValue = value; }
        Toshi::TPCString GetComment() const { return m_oComment; }
        void SetComment(const Toshi::TPCString& comment) { m_oComment = comment; }

    private:
        PPropertyName    m_oName;       // +0x08
        PPropertyValue   m_oValue;      // +0x10
        Toshi::TPCString m_oComment;    // +0x18
    };

    PProperties();
    PProperties(PProperties* parent);
    virtual ~PProperties();

    void PutProperty(const Toshi::TPCString& name, const Toshi::TPCString& subName, const PPropertyValue& value, const Toshi::TPCString& comment);
    void PutProperty(const PPropertyName& name, const PPropertyValue& value, const Toshi::TPCString& comment);
    void PutProperty(const PPropertyName& name, const PPropertyValue& value);
    void PutPropertyUnique(const Toshi::TPCString& name, const Toshi::TPCString& subName, const PPropertyValue& value, const Toshi::TPCString& comment);
    void PutPropertyUnique(const PPropertyName& name, const PPropertyValue& value, const Toshi::TPCString& comment);
    void PutPropertyUnique(const PPropertyName& name, const PPropertyValue& value);
    const PPropertyValue* GetProperty(const PPropertyName& name) const;
    const PPropertyValue* GetProperty(const Toshi::TPCString& name, int index) const;
    const PPropertyValue* GetRequiredProperty(const Toshi::TPCString& name, const Toshi::TClass* type) const;
    const PPropertyValue* GetRequiredProperty(const Toshi::TPCString& name, const Toshi::TPCString& subName, const Toshi::TClass* type) const;
    const PPropertyValue* GetRequiredProperty(const Toshi::TPCString& name, int index, const Toshi::TClass* type) const;
    const PPropertyValue* GetOptionalProperty(const Toshi::TPCString& name, const Toshi::TClass* type) const;
    const PPropertyValue* GetOptionalProperty(const Toshi::TPCString& name, const Toshi::TPCString& subName, const Toshi::TClass* type) const;
    const PPropertyValue* GetOptionalProperty(const Toshi::TPCString& name, int index, const Toshi::TClass* type) const;
    const PProperty* FindProperty(const Toshi::TPCString& name, Toshi::TQList<PProperty>::Iterator& it) const;
    void Sort();
    const PPropertyValue* GetProperty(const Toshi::TPCString& name, const Toshi::TPCString& subName) const;
    const PPropertyValue* GetProperty(const Toshi::TPCString& name) const;
    void PutPropertyUnique(const Toshi::TPCString& name, const Toshi::TPCString& subName, const PPropertyValue& value);
    void PutPropertyUnique(const Toshi::TPCString& name, const PPropertyValue& value);
    void PutPropertyUnique(const Toshi::TPCString& name, const PPropertyValue& value, const Toshi::TPCString& comment);
    void PutProperty(const Toshi::TPCString& name, const Toshi::TPCString& subName, const PPropertyValue& value);
    void PutProperty(const Toshi::TPCString& name, const PPropertyValue& value);
    void PutProperty(const Toshi::TPCString& name, const PPropertyValue& value, const Toshi::TPCString& comment);

    // inline (reverse order in the binary)
    TOSHI_FREELIST_DECLARE()
    Toshi::TQList<PProperty>::Iterator Begin() const { return m_oPropertySet.Begin(); }
    Toshi::TQList<PProperty>::Iterator End() const { return m_oPropertySet.End(); }
    PProperties* GetParentProperties() const { return m_pParent; }
    int GetPropertyCount() const { return m_iPropertyCount; }
    bool IsEmpty() const { return m_oPropertySet.IsEmpty(); }
    const Toshi::TQList<PProperty>& GetPropertySet() const { return m_oPropertySet; }
    Toshi::TQList<PProperty>& GetPropertySet() { return m_oPropertySet; }

private:
    PProperties*              m_pParent;            // +0x08
    Toshi::TQList<PProperty>  m_oPropertySet;       // +0x0C
    int                       m_iPropertyCount;     // +0x14
};

#endif
