// PPropertyName - "name" or "name[subname]" key of a property (PPropertyParser.dll).
// Source: Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyName.cpp
// Layout (8 bytes): TPCString name (+0), TPCString subname (+4).
#ifndef PPROPERTYNAME_H
#define PPROPERTYNAME_H
#include "PPropertyParserAPI.h"
#include "TPString.h"

class PPROPERTYPARSER_API PPropertyName
{
public:
    PPropertyName();
    PPropertyName(const Toshi::TPCString& name);
    PPropertyName(const Toshi::TPCString& name, const Toshi::TPCString& subName);
    PPropertyName(const PPropertyName& o);
    PPropertyName& operator=(const PPropertyName& o);

    bool operator==(const PPropertyName& o) const;
    bool operator!=(const PPropertyName& o) const;
    bool operator<(const PPropertyName& o) const;
    bool operator>(const PPropertyName& o) const;

    Toshi::TPCString GetString() const;

    // inline; ICC emits these (and the implicit destructor) in PProperties.cpp's range,
    // in reverse declaration order
    const Toshi::TPCString& GetName() const { return m_oName; }
    const Toshi::TPCString& GetSubName() const { return m_oSubName; }
    void SetName(const Toshi::TPCString& name) { m_oName = name; }
    void SetSubName(const Toshi::TPCString& subName) { m_oSubName = subName; }

protected:
    void Split(const Toshi::TPCString& s);

private:
    Toshi::TPCString m_oName;       // +0
    Toshi::TPCString m_oSubName;    // +4
};

PPROPERTYPARSER_API Toshi::TCString operator+(const Toshi::TCString& s, const PPropertyName& name);

#endif
