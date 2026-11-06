// Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyWriter.cpp  (reconstruction)
// Range in PPropertyParser.dll: 0x7fc0-0x985f.
#define PPROPERTYPARSER_EXPORTS
#include "TNewDelete.h"
#include "PPropertyWriter.h"
#include "TSystem.h"
#include <ctype.h>
#include <stdarg.h>

using namespace Toshi;

PPropertyWriter::PPropertyWriter()                                        // 0x7fc0
    : m_pFile(0), m_oDefinitionBlocks(10)
{
    m_iWriteBufferUsed = 0;
    m_bPrintUnsignedAsHex = true;
    m_pWriteBuffer = (char*)tmalloc(0x4000, 0, -1);
}

PPropertyWriter::~PPropertyWriter()                                       // 0x8010
{
    if (m_pFile)
    {
        FlushWriteBuffer();
        TFile::Destroy(m_pFile);
    }
    if (m_pWriteBuffer)
        tfree(m_pWriteBuffer);
}

bool PPropertyWriter::Open(const TWString& fileName)                      // 0x8060
{
    if (m_pFile)
    {
        FlushWriteBuffer();
        TFile::Destroy(m_pFile);
    }
    m_pFile = TFile::Create(TCString(fileName), 10);
    if (m_pFile)
        return true;
    Error(TWString(L"Can't save: ") + fileName.GetString(0));
    return false;
}

bool PPropertyWriter::Open(const TWString& fileName, TFile* file)         // 0x8110
{
    if (m_pFile)
    {
        FlushWriteBuffer();
        TFile::Destroy(m_pFile);
    }
    m_pFile = file;
    if (m_pFile)
        return true;
    Error(TWString(L"Can't save: ") + fileName.GetString(0));
    return false;
}

// Every definition block writes itself, in priority order.
bool PPropertyWriter::Save()                                              // 0x81a0
{
    for (TArray<TManagedPtr<PPropertyBlock> >::Iterator it = m_oDefinitionBlocks.Begin();
         it != m_oDefinitionBlocks.End(); ++it)
    {
        if (!(*it)->Save(this))
            return false;
    }
    return true;
}

bool PPropertyWriter::PutPropertyName(const TPCString& name, const TPCString& subName)   // 0x8220
{
    if (!Put(name))
        return false;
    if (subName.IsEmpty())
        return true;
    if (BufferCPrintf("[") < 0)
        return false;
    if (Put(subName) != true)
        return false;
    if (BufferCPrintf("]") < 0)
        return false;
    return true;
}

bool PPropertyWriter::PutBlockHeader(const TPCString& name)                // 0x82d0
{
    TCString header(name.GetCString().GetString(0));
    return BufferCPrintf("\n[%s]\n", header.GetString(0)) > 0;
}

// One line per property, indented with a tab per level; sub-blocks are written recursively.
bool PPropertyWriter::PutPropertyBlock(int depth, const PProperties* properties)   // 0x8340
{
    for (TQList<PProperties::PProperty>::Iterator it = properties->Begin(); it != properties->End(); ++it)
    {
        const PPropertyValue* value = it->GetValue();
        if (value->IsType(PPropertyValue::TYPE_PROPS))
        {
            if (!Put('\t', depth))
                return false;
            if (!Put(it->GetName()))
                return false;
            if (BufferCPrintf(" {\n") < 0)
                return false;
            if (!PutPropertyBlock(depth + 1, value->GetProperties()))
                return false;
            if (!Put('\t', depth))
                return false;
            if (BufferCPrintf("}\n") < 0)
                return false;
        }
        else if (value->IsType(PPropertyValue::TYPE_UNDEF))
        {
            if (!Put('\t', depth))
                return false;
            if (!Put(it->GetName()))
                return false;
            if (BufferCPrintf(";\n") < 0)
                return false;
        }
        else
        {
            if (!Put('\t', depth))
                return false;
            if (!Put(it->GetName()))
                return false;
            if (BufferCPrintf(" = ") < 0)
                return false;
            if (!Put(*value))
                return false;
            if (BufferCPrintf(";\n") < 0)
                return false;
        }
    }
    return true;
}

bool PPropertyWriter::Put(const PPropertyValue& value)                    // 0x8500
{
    const TClass* type = value.GetType();
    if (type == PPropertyValue::TYPE_INT)
        return Put(value.GetInteger());
    else if (type == PPropertyValue::TYPE_UINT32)
        return Put(value.GetUINT32());
    else if (type == PPropertyValue::TYPE_FLOAT)
        return Put(value.GetFloat());
    else if (type == PPropertyValue::TYPE_BOOL)
        return Put(value.GetBoolean());
    else if (type == PPropertyValue::TYPE_TPCSTRING)
        return Put(value.GetTPCString());
    else if (type == PPropertyValue::TYPE_TPWSTRING)
        return Put(value.GetTPWString());
    else if (type == PPropertyValue::TYPE_TLSTRING)
        return Put(value.GetTLString());
    else if (type == PPropertyValue::TYPE_PROPNAME)
        return Put(value.GetPropertyName());
    else if (type == PPropertyValue::TYPE_ARRAY)
    {
        if (!PutToken(TFileLexer::TOKEN_OPENBRACE))
            return false;
        TArray<PPropertyValue>::Iterator it = value.GetArray()->m_oValues.Begin();
        TArray<PPropertyValue>::Iterator end = value.GetArray()->m_oValues.End();
        if (it != end)
        {
            if (!Put(*it))
                return false;
            ++it;
            while (it != end)
            {
                if (!PutToken(TFileLexer::TOKEN_COMMA))
                    return false;
                if (!Put(*it))
                    return false;
                ++it;
            }
        }
        if (!PutToken(TFileLexer::TOKEN_CLOSEBRACE))
            return false;
        return true;
    }
    return false;
}

bool PPropertyWriter::Put(const PPropertyName& name)                      // 0x8780
{
    if (!Put(name.GetName()))
        return false;
    if (name.GetSubName().IsEmpty())
        return true;
    if (BufferCPrintf("[") < 0)
        return false;
    if (Put(name.GetSubName()) != true)
        return false;
    if (BufferCPrintf("]") < 0)
        return false;
    return true;
}

bool PPropertyWriter::Put(const TPWString& string)                        // 0x8840
{
    return Put(TCString(string.GetWString().GetString(0)));
}

// Temporary strings are written as their text; others as $identifier or $index.
bool PPropertyWriter::Put(const TLString& string)                         // 0x8890
{
    if (string.IsTemporary())
        return Put(TCString(string.GetLocalisedString()));
    if (string.GetIdentifierString().IsEmpty())
    {
        if (!PutToken(TFileLexer::TOKEN_DOLLAR))
            return false;
        if (!Put(string.GetIndex()))
            return false;
        return true;
    }
    if (!PutToken(TFileLexer::TOKEN_DOLLAR))
        return false;
    if (!Put(string.GetIdentifierString()))
        return false;
    return true;
}

bool PPropertyWriter::Put(char c, int count)                              // 0x89d0
{
    while (count-- > 0)
    {
        if (BufferCChar(c) < 0)
            return false;
    }
    return true;
}

// Quoted (with \" and \\ escaped) unless it is a plain identifier.
bool PPropertyWriter::Put(const TCString& string)                         // 0x8a10
{
    bool quotes = string_needs_quotes(string.GetString(0), string.Length());
    if (quotes)
        BufferCChar('"');
    const char* p = string;
    int length = string.Length();
    for (int i = 0; i < length; i++)
    {
        char c = *p++;
        if (c == '"' || c == '\\')
            BufferCChar('\\');
        BufferCChar(c);
    }
    if (quotes)
        BufferCChar('"');
    return true;
}

bool PPropertyWriter::Put(const char* string)                             // 0x8ae0
{
    bool quotes = string_needs_quotes(string, -1);
    if (quotes)
        BufferCChar('"');
    char c;
    while ((c = *string++) != 0)
    {
        if (c == '"' || c == '\\')
            BufferCChar('\\');
        BufferCChar(c);
    }
    if (quotes)
        BufferCChar('"');
    return true;
}

bool PPropertyWriter::Put(unsigned int value)                             // 0x8b90
{
    if (m_bPrintUnsignedAsHex)
        return BufferCPrintf("0x%08Xu", value) >= 0;
    return BufferCPrintf("%du", value) >= 0;
}

bool PPropertyWriter::PutToken(TFileLexer::TokenType token)               // 0x8bf0
{
    switch (token)
    {
    case TFileLexer::TOKEN_SEMICOLON:   if (BufferCChar(';') == -1) return false; return true;
    case TFileLexer::TOKEN_COLON:       if (BufferCChar(':') == -1) return false; return true;
    case TFileLexer::TOKEN_DOT:         if (BufferCChar('.') == -1) return false; return true;
    case TFileLexer::TOKEN_ASTERISK:    if (BufferCChar('*') == -1) return false; return true;
    case TFileLexer::TOKEN_COMMA:       if (BufferCChar(',') == -1) return false; return true;
    case TFileLexer::TOKEN_DOLLAR:      if (BufferCChar('$') == -1) return false; return true;
    case TFileLexer::TOKEN_OPENSQR:     if (BufferCChar('[') == -1) return false; return true;
    case TFileLexer::TOKEN_CLOSESQR:    if (BufferCChar(']') == -1) return false; return true;
    case TFileLexer::TOKEN_OPENBRACE:   if (BufferCChar('{') == -1) return false; return true;
    case TFileLexer::TOKEN_CLOSEBRACE:  if (BufferCChar('}') == -1) return false; return true;
    case TFileLexer::TOKEN_OPENPAREN:   if (BufferCChar('(') == -1) return false; return true;
    case TFileLexer::TOKEN_CLOSEPAREN:  if (BufferCChar(')') == -1) return false; return true;
    case TFileLexer::TOKEN_LESSTHAN:    if (BufferCChar('<') == -1) return false; return true;
    case TFileLexer::TOKEN_GREATERTHAN: if (BufferCChar('>') == -1) return false; return true;
    case TFileLexer::TOKEN_EQUAL:       if (BufferCChar('=') == -1) return false; return true;
    case TFileLexer::TOKEN_EOF:
    case TFileLexer::TOKEN_IDENT:
    case TFileLexer::TOKEN_STRING:
    case TFileLexer::TOKEN_INTEGER:
    case TFileLexer::TOKEN_UINTEGER:
    case TFileLexer::TOKEN_FLOAT:
    case TFileLexer::TOKEN_COMMENT:
        return false;
    }
    return false;
}

bool PPropertyWriter::PutLiteral(const char* literal)                     // 0x8ea0
{
    TCString string(literal);
    return BufferCPrintf("%s", string.GetString(0)) >= 0;
}

// length -1: zero-terminated. Empty strings, strings not starting with a letter, "true",
// "false" and anything with characters other than [A-Za-z0-9_] need quotes.
bool PPropertyWriter::string_needs_quotes(const char* string, int length) const   // 0x8ef0
{
    if ((length == -1 ? *string == 0 : length == 0) || !isalpha(*string))
        return true;
    if (TSystem::StringCompare(string, "true", -1) == 0)
        return true;
    if (TSystem::StringCompare(string, "false", -1) == 0)
        return true;
    if (length == -1)
    {
        while (*string != '\0')
        {
            if (!__isascii(*string) || (!isalnum(*string) && *string != '_'))
                return true;
            string++;
        }
    }
    else
    {
        while (length > 0)
        {
            if (!__isascii(*string) || (!isalnum(*string) && *string != '_'))
                return true;
            string++;
            length--;
        }
    }
    return false;
}

int PPropertyWriter::BufferCChar(const char& c)                           // 0x8ff0
{
    int result = c;
    if (m_iWriteBufferUsed >= 0x4000)
        result = FlushWriteBuffer();
    m_pWriteBuffer[m_iWriteBufferUsed] = c;
    m_iWriteBufferUsed++;
    return result;
}

int __cdecl PPropertyWriter::BufferCPrintf(const char* format, ...)       // 0x9030
{
    FlushWriteBuffer();
    va_list args;
    va_start(args, format);
    return m_pFile->VCPrintf(format, args);
}

int PPropertyWriter::FlushWriteBuffer()                                   // 0x9060
{
    if (m_iWriteBufferUsed > 0)
    {
        int result = m_pFile->Write(m_pWriteBuffer, m_iWriteBufferUsed);
        m_iWriteBufferUsed = 0;
        return result;
    }
    return -1;
}

TOSHI_DEFINE_CLASS(PPropertyWriter, TObject)

void PPropertyWriter::Error(const TWString& message)                      // 0x91c0
{
    message.Print();
}

bool PPropertyWriter::Put(bool value)                                     // 0x91d0
{
    return BufferCPrintf(value ? "true" : "false") >= 0;
}

bool PPropertyWriter::Put(float value)                                    // 0x9210
{
    return BufferCPrintf("%gf", value) >= 0;
}

bool PPropertyWriter::Put(int value)                                      // 0x9240
{
    return BufferCPrintf("%d", value) >= 0;
}

bool PPropertyWriter::Put(const TPCString& string)                        // 0x9270
{
    return Put(string.GetCString());
}

bool PPropertyWriter::PutNewLine()                                        // 0x92a0
{
    return BufferCPrintf("\n") >= 0;
}

bool PPropertyWriter::PutEndLine()                                        // 0x92c0
{
    return BufferCPrintf(";\n") >= 0;
}

bool PPropertyWriter::PutSpace()                                          // 0x92e0
{
    return BufferCPrintf("\t") >= 0;
}

bool PPropertyWriter::PutPropertyBlock(const PProperties* properties)     // 0x9300
{
    return PutPropertyBlock(0, properties);
}

bool PPropertyWriter::IsFileValid()                                       // 0x9320
{
    return m_pFile != 0;
}

void PPropertyWriter::AddDefinitionBlock(TManagedPtr<PPropertyBlock> block)   // 0x9330
{
    bool found = false;
    TArray<TManagedPtr<PPropertyBlock> >::Iterator it = m_oDefinitionBlocks.FindSorted(block, found);
    m_oDefinitionBlocks.InsertAt(it.m_index, 1);
    m_oDefinitionBlocks[it] = block;
}

void PPropertyWriter::Close()                                             // 0x93d0
{
    if (m_pFile)
    {
        FlushWriteBuffer();
        TFile::Destroy(m_pFile);
    }
    m_pFile = 0;
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(PPropertyWriter)
