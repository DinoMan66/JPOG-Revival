// Toshi::TFileLexer(UTF8) - import stubs (TFile is in TFile.h, the listeners in TObject.h) (TKernelInterface.dll).
// Signatures come from the kernel's export table. Layout facts used by PPropertyParser:
//   TFileLexerUTF8 is a TObject (deleted through vtable slot 2), 0x8C bytes, output-comments flag at
//   +8, parse-error emitter at +0x80. A Token is 12 bytes: type +0, line +4, value +8 (a TPCString for
//   the string-like types 0x10/0x11/0x15). Token's small members are inline (the kernel exports them
//   because its classes are dllexport); only assign() is a real call.
#ifndef TFILELEXER_H
#define TFILELEXER_H
#include "TObject.h"
#include "TCString.h"
#include "TPString.h"
#include "TFile.h"

namespace Toshi {

class TFileLexer : public TObject
{
public:
    // numeric values are the ones the binaries test; names are descriptive guesses
    enum TokenType
    {
        TOKEN_EOF = 0,
        // names from TFileLexerUTF16::tostring(TokenType) in TKernelInterface.dll (0 and 1 are
        // short strings, read as EOF and SEMI; 5 has no name there but PPropertyWriter writes it as *)
        TOKEN_SEMICOLON = 1, TOKEN_COLON = 2, TOKEN_COMMA = 3, TOKEN_DOT = 4, TOKEN_ASTERISK = 5,
        TOKEN_DOLLAR = 6, TOKEN_OPENSQR = 7, TOKEN_CLOSESQR = 8, TOKEN_OPENBRACE = 9,
        TOKEN_CLOSEBRACE = 10, TOKEN_OPENPAREN = 11, TOKEN_CLOSEPAREN = 12, TOKEN_LESSTHAN = 13,
        TOKEN_GREATERTHAN = 14, TOKEN_EQUAL = 15,
        TOKEN_IDENT = 0x10, TOKEN_STRING = 0x11, TOKEN_INTEGER = 0x12, TOKEN_UINTEGER = 0x13,
        TOKEN_FLOAT = 0x14, TOKEN_COMMENT = 0x15
    };
};

class TFileLexerUTF8 : public TFileLexer
{
public:
    class Token
    {
    public:
        Token() { m_type = TOKEN_EOF; m_line = 0; }
        Token(TokenType type, int line) { m_type = type; m_line = line; }
        Token(const Token& o) { m_type = TOKEN_EOF; assign(o); }
        ~Token()
        {
            if (m_type == TOKEN_IDENT || m_type == TOKEN_STRING || m_type == TOKEN_COMMENT)
#if defined(VAR_H1)
                ((TPCString*)&m_value)->TPCString::~TPCString();
#elif defined(VAR_H2)
                GetStringRef().~TPCString();
#elif defined(VAR_H3)
                DestroyString((TPCString*)&m_value);
#else
                ((TPCString*)&m_value)->~TPCString();
#endif
        }
        TPCString& GetStringRef() { return *(TPCString*)&m_value; }
        static void DestroyString(TPCString* s) { s->~TPCString(); }
        Token& operator=(const Token& o) { assign(o); return *this; }
        TOSHI_API void assign(const Token& o);

        TokenType GetType() const { return m_type; }
        int GetLine() const { return m_line; }
        int GetInteger() const { return m_value.i; }
        unsigned int GetUInteger() const { return m_value.u; }
        float GetFloat() const { return m_value.f; }
        const TPCString& GetString() const { return *(const TPCString*)&m_value; }

        TokenType m_type;           // +0
        int       m_line;           // +4
        union { int i; unsigned int u; float f; void* p; } m_value;    // +8
    };

    class ParseError
    {
    public:
        int         m_iLine;        // +0
        const char* m_szMessage;    // +4
    };

    TOSHI_API TFileLexerUTF8(TFile* file, int lookahead);
    TOSHI_API virtual TClass& GetClass() const;
    TOSHI_API virtual ~TFileLexerUTF8();
    TOSHI_API Token PeekNextToken(int ahead);
    TOSHI_API Token GetNextToken();
    TOSHI_API bool Expect(TokenType type);
    TOSHI_API bool Expect(TokenType type, Token& token);

    unsigned char m_pad04[4];                               // +4
    bool          m_bOutputComments;                        // +8
    unsigned char m_pad09[0x80 - 9];
    TEmitter<TFileLexerUTF8, ParseError> m_oParseErrorEmitter;  // +0x80     sizeof == 0x8C
};

} // namespace Toshi
#endif
