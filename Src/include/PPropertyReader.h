// PPropertyReader - reads property files through the kernel's UTF-8 lexer (PPropertyParser.dll)
// Source: Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyReader.cpp  (code 0x32b0-0x5f2f)
// Layout (0x58): vptr; TCString file name +4; TFile* +0xC; TFileLexerUTF8* +0x10;
// sorted TArray of definition blocks +0x14 (grow 10, 8-byte entries {TPCString name, block});
// bool load-comments +0x24 (0); bool assert-on-error +0x25 (1); TArray +0x28 (grow 10);
// parse-error emitter +0x38; lexer listener +0x44.
// vtable: GetClass, TObject::Delete, dtor, Open(name, TFile*), Open(name).
#ifndef PPROPERTYREADER_H
#define PPROPERTYREADER_H
#include "PPropertyParserAPI.h"
#include "TObject.h"
#include "TFileLexer.h"
#include "TArray.h"
#include "TManagedPtr.h"
#include "PPropertyBlock.h"
#include "PPropertyValue.h"
#include "PProperties.h"

// Thrown through PPropertyReader::GetParseErrorEmitter(). Everything is compiler-generated
// (the exported default ctor only runs Token's ctor; copy/assign/dtor are member-wise).
class PPROPERTYPARSER_API PPropertyReaderParseError
{
public:
    Toshi::TFileLexerUTF8::Token m_oToken;      // +0x00
    const char*                  m_szFileName;  // +0x0C
    const char*                  m_szMessage;   // +0x10
};

class PPROPERTYPARSER_API PPropertyReader : public Toshi::TObject
{
    TOSHI_DECLARE_CLASS(PPropertyReader)
public:
    typedef Toshi::TFileLexerUTF8::Token Token;

    // one entry of the sorted definition-block table
    struct BlockEntry
    {
        BlockEntry() {}
        BlockEntry(const Toshi::TPCString& name) { m_oName = name; }
        BlockEntry(const Toshi::TPCString& name, const Toshi::TManagedPtr<PPropertyBlock>& block)
            : m_oName(name), m_pBlock(block) {}
        int Compare(const BlockEntry& o) const { return m_oName.Compare(o.m_oName); }

        Toshi::TPCString                     m_oName;    // +0
        Toshi::TManagedPtr<PPropertyBlock>   m_pBlock;   // +4
    };

    PPropertyReader();
    virtual ~PPropertyReader();
    virtual bool Open(const Toshi::TCString& fileName, Toshi::TFile* file);    // slot 3
    virtual bool Open(const Toshi::TCString& fileName);                        // slot 4
    void Close();
    void AddDefinitionBlock(Toshi::TManagedPtr<PPropertyBlock> block);

    bool Load();
    bool Load(const Toshi::TCString& fileName, PProperties& props);
    PProperties* LoadPropertyBlock();
    bool LoadPropertyBlock(PProperties& props);
    bool LoadProperty(PProperties* props);
    bool LoadPropertyName(Toshi::TPCString& name, Toshi::TPCString& subName);
    bool LoadBlockHeader(Toshi::TPCString& name);
    PPropertyValue Token2Value(const Token& token);
    bool GetValue(PPropertyValue& value);

    void Error(const Toshi::TCString& message);
    void Error(const Token& token, const Toshi::TCString& message);
    void Warning(const Token& token, const Toshi::TCString& message);
    void Warning(const Toshi::TCString& message);

    Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError>* GetParseErrorEmitter() { return &m_oParseErrorEmitter; }
    Toshi::TFileLexerUTF8* GetLexer() { return m_pLexer; }
    void SetAssertOnError(bool assert) { m_bAssertOnError = assert; }
    void SetLoadComments(bool load) { m_bLoadComments = load; }

private:
    bool SkipBlock();
    static bool __stdcall OnLexerParseError(PPropertyReader* self, Toshi::TFileLexerUTF8* lexer,
                                            Toshi::TFileLexerUTF8::ParseError* error);

    Toshi::TCString                 m_oFileName;        // +0x04
    Toshi::TFile*                   m_pFile;            // +0x0C
    Toshi::TFileLexerUTF8*          m_pLexer;           // +0x10
    Toshi::TArray<BlockEntry>       m_oDefinitionBlocks;// +0x14
    bool                            m_bLoadComments;    // +0x24
    bool                            m_bAssertOnError;   // +0x25
    Toshi::TArray<int>              m_oBlockLines;       // +0x28 stack: line of each open "{"
    Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError> m_oParseErrorEmitter;   // +0x38
    Toshi::TListener<PPropertyReader, Toshi::TFileLexerUTF8, Toshi::TFileLexerUTF8::ParseError> m_oLexerListener; // +0x44
};

#endif
