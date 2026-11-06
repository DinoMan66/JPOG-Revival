// PPropertyBlock - abstract handler for a named block in a property file (PPropertyParser.dll)
// Source: Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyBlock.cpp
// Layout (0x10): vptr, m_iRefCount (+4, TManagedPtr), TPCString name (+8), Priority (+0xC).
// vtable: GetClass, TObject::Delete, dtor, two pure virtuals (slot 3 is called by
// PPropertyReader with the reader as argument).
#ifndef PPROPERTYBLOCK_H
#define PPROPERTYBLOCK_H
#include "PPropertyParserAPI.h"
#include "TObject.h"
#include "TPString.h"
#include "TManagedPtr.h"

class PPropertyReader;
class PPropertyWriter;

class PPROPERTYPARSER_API PPropertyBlock : public Toshi::TObject, public Toshi::TRefCountBase
{
    TOSHI_DECLARE_CLASS(PPropertyBlock)
public:
    enum Priority { };

    PPropertyBlock(const Toshi::TPCString& name, Priority priority);
    virtual ~PPropertyBlock();
    virtual bool Load(PPropertyReader* reader) = 0;     // slot 3
    virtual bool Save(PPropertyWriter* writer) = 0;     // slot 4

    // inline (reverse order in the binary)
    const Toshi::TPCString& GetName() const { return m_oName; }
    Priority GetPriority() const { return m_ePriority; }
    void SetName(const Toshi::TPCString& name) { m_oName = name; }

private:
    Toshi::TPCString m_oName;       // +0x08
    Priority         m_ePriority;   // +0x0C
};

#endif
