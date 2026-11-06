// PPropertyWriter - writes property files (PPropertyParser.dll)
// Source: Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyWriter.cpp  (code 0x7fc0-0x985f)
// Layout (0x24): vptr; TFile* +4; TArray of definition blocks +8 (grow 10, sorted by priority);
// write buffer +0x18 (0x4000 bytes); bytes used +0x1C; bool print-unsigned-as-hex +0x20 (1).
// vtable: GetClass, TObject::Delete, dtor.
#ifndef PPROPERTYWRITER_H
#define PPROPERTYWRITER_H
#include "PPropertyParserAPI.h"
#include "TObject.h"
#include "TFileLexer.h"
#include "TArray.h"
#include "TManagedPtr.h"
#include "TLString.h"
#include "PPropertyBlock.h"
#include "PPropertyValue.h"
#include "PProperties.h"

namespace Toshi {
// definition blocks are kept sorted by priority (FindSorted at 0x9480)
inline int TArrayCompare(const TManagedPtr<PPropertyBlock>& a, const TManagedPtr<PPropertyBlock>& b)
{
    return a->GetPriority() - b->GetPriority();
}
}

class PPROPERTYPARSER_API PPropertyWriter : public Toshi::TObject
{
    TOSHI_DECLARE_CLASS(PPropertyWriter)
public:
    PPropertyWriter();
    virtual ~PPropertyWriter();

    bool Open(const Toshi::TWString& fileName);
    bool Open(const Toshi::TWString& fileName, Toshi::TFile* file);
    void Close();
    bool IsFileValid();
    bool Save();
    void AddDefinitionBlock(Toshi::TManagedPtr<PPropertyBlock> block);

    bool PutBlockHeader(const Toshi::TPCString& name);
    bool PutPropertyBlock(const PProperties* properties);
    bool PutPropertyBlock(int depth, const PProperties* properties);
    bool PutPropertyName(const Toshi::TPCString& name, const Toshi::TPCString& subName);
    bool Put(const PPropertyValue& value);
    bool Put(const PPropertyName& name);
    bool Put(const Toshi::TPCString& string);
    bool Put(const Toshi::TPWString& string);
    bool Put(const Toshi::TLString& string);
    bool Put(const Toshi::TCString& string);
    bool Put(const char* string);
    bool Put(char c, int count);
    bool Put(unsigned int value);
    bool Put(int value);
    bool Put(float value);
    bool Put(bool value);
    bool PutToken(Toshi::TFileLexer::TokenType token);
    bool PutLiteral(const char* literal);
    bool PutNewLine();
    bool PutEndLine();
    bool PutSpace();
    void Error(const Toshi::TWString& message);

    void PrintUnsignedAsHex(bool hex) { m_bPrintUnsignedAsHex = hex; }

protected:
    bool string_needs_quotes(const char* string, int length) const;
    int BufferCChar(const char& c);
    int __cdecl BufferCPrintf(const char* format, ...);
    int FlushWriteBuffer();

private:
    Toshi::TFile*                                   m_pFile;                // +0x04
    Toshi::TArray<Toshi::TManagedPtr<PPropertyBlock> > m_oDefinitionBlocks; // +0x08
    char*                                           m_pWriteBuffer;         // +0x18
    int                                             m_iWriteBufferUsed;     // +0x1C
    bool                                            m_bPrintUnsignedAsHex;  // +0x20
};

#endif
