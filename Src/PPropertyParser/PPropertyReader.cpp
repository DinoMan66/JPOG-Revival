// Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyReader.cpp  (reconstruction)
// Range in PPropertyParser.dll: 0x32b0-0x5f2f.
// BATCH 1: construction, Open/Close, AddDefinitionBlock, Load(), errors/warnings, small members.
// BATCH 2: Load(name, props), LoadPropertyBlock x2, LoadProperty, LoadPropertyName,
//          LoadBlockHeader, Token2Value, GetValue, SkipBlock.
#define PPROPERTYPARSER_EXPORTS
#define TOSHI_STRING_ASSIGN_NOT_INLINED   // MATCHING_NOTES #21
#include "TNewDelete.h"
#include "PPropertyReader.h"
#include "TSystem.h"

PPropertyReader::PPropertyReader()
    : m_oDefinitionBlocks(10), m_oBlockLines(10), m_oParseErrorEmitter(this)
{
    m_pFile = 0;
    m_pLexer = 0;
    m_bLoadComments = false;
    m_bAssertOnError = true;
}

TOSHI_DEFINE_CLASS(PPropertyReader, TObject)

PPropertyReader::~PPropertyReader()
{
    if (m_pFile)
        TFile::Destroy(m_pFile);
    if (m_pLexer)
        delete m_pLexer;
}

bool PPropertyReader::Open(const TCString& fileName)                       // 0x3380
{
    m_oFileName = fileName;
    if (m_pFile)
    {
        TFile::Destroy(m_pFile);
        m_pFile = 0;
    }
    if (m_pLexer)
    {
        delete m_pLexer;
        m_pLexer = 0;
    }
    m_pFile = TFile::Create(fileName, 1);
    if (!m_pFile)
    {
        Warning(TCString().Format("Can't open file", fileName.GetString(0)));
        return false;
    }
    m_pLexer = new TFileLexerUTF8(m_pFile, 2);
    m_pLexer->m_bOutputComments = m_bLoadComments;
    m_oLexerListener.Connect(&m_pLexer->m_oParseErrorEmitter, this, OnLexerParseError, 0);
    return true;
}

bool PPropertyReader::Open(const TCString& fileName, TFile* file)           // 0x3480
{
    m_oFileName = fileName;
    if (m_pFile)
    {
        TFile::Destroy(m_pFile);
        m_pFile = 0;
    }
    if (m_pLexer)
    {
        delete m_pLexer;
        m_pLexer = 0;
    }
    m_pFile = file;
    m_pLexer = new TFileLexerUTF8(m_pFile, 2);
    m_pLexer->m_bOutputComments = m_bLoadComments;
    m_oLexerListener.Connect(&m_pLexer->m_oParseErrorEmitter, this, OnLexerParseError, 0);
    return true;
}

void PPropertyReader::Close()                                               // 0x3530
{
    if (m_pFile)
    {
        TFile::Destroy(m_pFile);
        m_pFile = 0;
    }
    if (m_pLexer)
    {
        delete m_pLexer;
        m_pLexer = 0;
    }
}

void PPropertyReader::AddDefinitionBlock(TManagedPtr<PPropertyBlock> block) // 0x3570
{
    bool found = false;
    BlockEntry entry(block->GetName(), block);
    TArray<BlockEntry>::Iterator it = m_oDefinitionBlocks.FindSorted(entry, found);
    m_oDefinitionBlocks.InsertAt(it.m_index, 1);
    m_oDefinitionBlocks[it] = entry;
}

bool PPropertyReader::Load()                                                // 0x3680
{
    Token token;
    token = m_pLexer->PeekNextToken(0);
    while (token.GetType() != TFileLexer::TOKEN_EOF)
    {
        TPCString name;
        if (!LoadBlockHeader(name))
            return false;
        TArray<BlockEntry>::Iterator it = m_oDefinitionBlocks.Find(BlockEntry(name));
        if (it.m_index == -1)
        {
            Warning(token, TCString().Format("Unknown block labelled '%s' found -- skipping",
                                             name.GetCString().GetString(0)));
            SkipBlock();
        }
        else
        {
            TManagedPtr<PPropertyBlock> definition = it->m_pBlock;
            definition->Load(this);
        }
        token = m_pLexer->PeekNextToken(0);
    }
    return true;
}

bool PPropertyReader::Load(const TCString& fileName, PProperties& properties)  // 0x38c0
{
    if (!Open(fileName))
        return false;
    Token token = m_pLexer->PeekNextToken(0);
    if (token.GetType() != TFileLexer::TOKEN_EOF)
    {
        m_oBlockLines.Clear();
        if (LoadProperty(&properties))
        {
            if (m_pLexer->PeekNextToken(0).GetType() == TFileLexer::TOKEN_EOF)
                return true;
            Error(TCString("Unexpected data at end of file!"));
        }
    }
    Close();
    return false;
}

PProperties* PPropertyReader::LoadPropertyBlock()                         // 0x39d0
{
    bool eof = m_pLexer->PeekNextToken(0).GetType() == TFileLexer::TOKEN_EOF;
    if (eof)
        return 0;
    PProperties* properties = new PProperties();
    m_oBlockLines.Clear();
    if (!LoadProperty(properties))
        return 0;
    return properties;
}

bool PPropertyReader::LoadPropertyBlock(PProperties& properties)          // 0x3a70
{
    bool eof = m_pLexer->PeekNextToken(0).GetType() == TFileLexer::TOKEN_EOF;
    if (eof)
        return false;
    m_oBlockLines.Clear();
    if (!LoadProperty(&properties))
        return false;
    return true;
}

void PPropertyReader::Error(const TCString& message)                       // 0x4c80
{
    PPropertyReaderParseError error;
    error.m_oToken = Token();
    error.m_szFileName = m_oFileName.GetString(0);
    error.m_szMessage = message.GetString(0);
    m_oParseErrorEmitter.Throw(error);
}

void PPropertyReader::Error(const Token& token, const TCString& message)   // 0x4d20
{
    PPropertyReaderParseError error;
    error.m_oToken = token;
    error.m_szMessage = message.GetString(0);
    m_oParseErrorEmitter.Throw(error);
}

void PPropertyReader::Warning(const Token& token, const TCString& message) // 0x4d90
{
    PPropertyReaderParseError error;
    error.m_oToken = token;
    error.m_szMessage = message.GetString(0);
    m_oParseErrorEmitter.Throw(error);
}

bool __stdcall PPropertyReader::OnLexerParseError(PPropertyReader* self, TFileLexerUTF8*,
                                                  TFileLexerUTF8::ParseError* error)   // 0x55c0
{
    PPropertyReaderParseError e;
    e.m_oToken = Token(TFileLexer::TOKEN_EOF, error->m_iLine);
    e.m_szMessage = error->m_szMessage;
    self->m_oParseErrorEmitter.Throw(e);
    return true;
}

// Reads "name[sub] <op> ..." statements into properties until the closing "}" (or end of
// file at the outer level). Operators: ";" (no value), "=" value ";", "." (descend into a
// sub-block, creating it if needed) and "{" (nested block, read recursively).
bool PPropertyReader::LoadProperty(PProperties* properties)              // 0x3b00
{
    Token semicolon;
    PProperties* current = properties;
    for (;;)
    {
        Token token = m_pLexer->PeekNextToken(0);
        if (token.GetType() == TFileLexer::TOKEN_CLOSEBRACE)
        {
            m_oBlockLines.Pop();
            m_pLexer->GetNextToken();
            return true;
        }
        if (token.GetType() == TFileLexer::TOKEN_EOF && m_oBlockLines.Count() != 0)
        {
            Error(token, TCString().Format("Unexpected end of file in middle of property block (started at line %d)",
                                           m_oBlockLines.Last()));
            m_oBlockLines.Pop();
            return false;
        }
        if (token.GetType() != TFileLexer::TOKEN_IDENT && token.GetType() != TFileLexer::TOKEN_STRING &&
            token.GetType() != TFileLexer::TOKEN_COMMENT)
            return true;

        TPCString comment;
        TPCString name;
        TPCString subName;
        while (token.GetType() == TFileLexer::TOKEN_COMMENT && token.GetType() != TFileLexer::TOKEN_EOF)
        {
            comment = token.GetString();
            m_pLexer->GetNextToken();
            token = m_pLexer->PeekNextToken(0);
        }
        if (!LoadPropertyName(name, subName))
            return false;

        PPropertyName propertyName(name, subName);
        Token op = m_pLexer->GetNextToken();
        if (op.GetType() == TFileLexer::TOKEN_OPENBRACE)
        {
            m_oBlockLines.AddEmpty() = op.GetLine();
            PProperties* block = new PProperties(current);
            current->PutProperty(propertyName, PPropertyValue(block), comment);
            if (!LoadProperty(block))
                return false;
        }
        else if (op.GetType() == TFileLexer::TOKEN_DOT)
        {
            const PPropertyValue* value = current->GetProperty(propertyName);
            if (value)
                current = value->GetProperties();
            else
            {
                PProperties* block = new PProperties(current);
                current->PutProperty(propertyName, PPropertyValue(block), comment);
                current = block;
            }
        }
        else if (op.GetType() == TFileLexer::TOKEN_SEMICOLON)
        {
            current->PutProperty(propertyName, PPropertyValue(), comment);
        }
        else if (op.GetType() == TFileLexer::TOKEN_EQUAL)
        {
            PPropertyValue value;
            if (!GetValue(value))
                return false;
            current->PutProperty(propertyName, value, comment);
            if (!m_pLexer->Expect(TFileLexer::TOKEN_SEMICOLON, semicolon))
            {
                Error(semicolon, TCString().Format("Expected semicolon at end of property assignment"));
                return false;
            }
            current = properties;
        }
        else
        {
            Error(op, TCString().Format("Unexpected operator after property '%s'",
                                        propertyName.GetString().GetCString().GetString(0)));
            return false;
        }
    }
}

PPropertyValue PPropertyReader::Token2Value(const Token& token)            // 0x4af0
{
    static TPCString s_true = TSystem::GetCStringPool()->Get(TCString(L"true"));
    static TPCString s_false = TSystem::GetCStringPool()->Get(TCString(L"false"));
    if (token.GetType() == TFileLexer::TOKEN_INTEGER)
        return PPropertyValue(token.GetInteger());
    else if (token.GetType() == TFileLexer::TOKEN_UINTEGER)
        return PPropertyValue(token.GetUInteger());
    else if (token.GetType() == TFileLexer::TOKEN_FLOAT)
        return PPropertyValue(token.GetFloat());
    else if (token.GetType() == TFileLexer::TOKEN_STRING)
        return PPropertyValue(token.GetString());
    else if (token.GetType() == TFileLexer::TOKEN_IDENT)
    {
        if (token.GetString() == s_true)
            return PPropertyValue(true);
        if (token.GetString() == s_false)
            return PPropertyValue(false);
        return PPropertyValue(token.GetString());
    }
    return PPropertyValue();
}

// name  or  name[subname]  where subname is an identifier, a string or a string-pool index.
bool PPropertyReader::LoadPropertyName(TPCString& name, TPCString& subName) // 0x44f0
{
    Token closeToken;
    Token token = m_pLexer->GetNextToken();
    Token next = m_pLexer->PeekNextToken(0);
    if (token.GetType() != TFileLexer::TOKEN_IDENT && token.GetType() != TFileLexer::TOKEN_STRING)
    {
        Error(token, TCString("Expecting property name to be identifer"));
        return false;
    }
    name = token.GetString();
    if (next.GetType() == TFileLexer::TOKEN_OPENSQR)
    {
        m_pLexer->GetNextToken();
        Token sub = m_pLexer->GetNextToken();
        if (m_pLexer->Expect(TFileLexer::TOKEN_CLOSESQR, closeToken) != true)
        {
            Error(closeToken, TCString("Expected close bracket after property sub-name"));
            return false;
        }
        if (sub.GetType() == TFileLexer::TOKEN_IDENT || sub.GetType() == TFileLexer::TOKEN_STRING)
            subName = sub.GetString();
        else if (sub.GetType() == TFileLexer::TOKEN_INTEGER)
            subName = TSystem::GetCStringPool()->Get(sub.GetInteger());
        else
        {
            Error(sub, TCString("Expecting property subname to be identifier or integer"));
            return false;
        }
    }
    return true;
}

// "[ name ]" - comments in front of it are skipped when comments are being loaded.
bool PPropertyReader::LoadBlockHeader(TPCString& name)                     // 0x4860
{
    Token nameToken;
    Token token;
    while (m_bLoadComments && m_pLexer->PeekNextToken(0).GetType() == TFileLexer::TOKEN_COMMENT)
        m_pLexer->GetNextToken();
    if (!m_pLexer->Expect(TFileLexer::TOKEN_OPENSQR, token))
    {
        Error(token, TCString(L"Block header not found (expected '[')"));
        return false;
    }
    if (!m_pLexer->Expect(TFileLexer::TOKEN_IDENT, nameToken))
    {
        Error(nameToken, TCString(L"Block header not found (expected identifier)"));
        return false;
    }
    if (!m_pLexer->Expect(TFileLexer::TOKEN_CLOSESQR, token))
    {
        Error(token, TCString(L"Block header not found (expected ']')"));
        return false;
    }
    name = nameToken.GetString();
    return true;
}

// A value: a literal, "$name"/"$index" (localised string), "name[sub]" (property reference)
// or "{ v, v, ... }" (array).
bool PPropertyReader::GetValue(PPropertyValue& value)                     // 0x4e00
{
    Token token = m_pLexer->PeekNextToken(0);
    if (token.GetType() == TFileLexer::TOKEN_OPENBRACE)
    {
        PPropertyValueArray* array = new PPropertyValueArray(3);
        m_pLexer->GetNextToken();
        for (;;)
        {
            bool close = m_pLexer->PeekNextToken(0).GetType() == TFileLexer::TOKEN_CLOSEBRACE;
            if (close)
            {
                m_pLexer->GetNextToken();
                break;
            }
            if (!GetValue(array->m_oValues.AddEmpty()))
            {
                delete array;
                return false;
            }
            Token separator = m_pLexer->GetNextToken();
            if (separator.GetType() == TFileLexer::TOKEN_COMMA)
                continue;
            if (separator.GetType() == TFileLexer::TOKEN_CLOSEBRACE)
                break;
            Error(separator, TCString("Expected a comma or close brace inside a list"));
            delete array;
            return false;
        }
        value = array;
    }
    else if (token.GetType() == TFileLexer::TOKEN_DOLLAR)
    {
        m_pLexer->GetNextToken();
        token = m_pLexer->GetNextToken();
        if (token.GetType() == TFileLexer::TOKEN_IDENT)
            value = PPropertyValue(TSystem::GetLocale()->Get(token.GetString()));
        else if (token.GetType() == TFileLexer::TOKEN_INTEGER)
            value = PPropertyValue(TSystem::GetLocale()->Get(token.GetInteger()));
        else
        {
            Error(token, TCString("Expected identifier after '$' for localised string name"));
            return false;
        }
    }
    else
    {
        bool reference = m_pLexer->PeekNextToken(1).GetType() == TFileLexer::TOKEN_OPENSQR;
        if (reference)
        {
            TPCString name;
            TPCString subName;
            LoadPropertyName(name, subName);
            value = PPropertyValue(PPropertyName(name, subName));
        }
        else
        {
            m_pLexer->GetNextToken();
            value = Token2Value(token);
            if (!value.IsDefined())
            {
                Error(token, TCString("Expected a value"));
                return false;
            }
        }
    }
    return true;
}

// Skips everything up to the next "[" block header (or end of file). Each statement is
// consumed up to its terminating ';', counting { } nesting.
bool PPropertyReader::SkipBlock()                                          // 0x5440
{
    Token token;
    token = m_pLexer->PeekNextToken(0);
    int depth = 0;
    while (token.GetType() != TFileLexer::TOKEN_OPENSQR && token.GetType() != TFileLexer::TOKEN_EOF)
    {
        token = m_pLexer->GetNextToken();
        while ((token.GetType() != TFileLexer::TOKEN_SEMICOLON || depth > 0) && token.GetType() != TFileLexer::TOKEN_EOF)
        {
            if (token.GetType() == TFileLexer::TOKEN_OPENBRACE)
                depth++;
            if (token.GetType() == TFileLexer::TOKEN_CLOSEBRACE)
                depth--;
            token = m_pLexer->GetNextToken();
        }
        token = m_pLexer->PeekNextToken(0);
    }
    return true;
}

void PPropertyReader::Warning(const TCString&)                             // 0x5850
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(PPropertyReader)
