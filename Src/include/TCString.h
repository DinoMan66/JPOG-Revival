// Toshi::TCString / Toshi::TWString - import declarations (the classes live in TKernelInterface.dll).
// Every signature below is taken from TKernelInterface.dll's export table (decorated names), so the
// calls compile to the same imports as the original. Wide characters are "unsigned short" (VC6 /
// -Qvc6: wchar_t is not a native type; L"..." literals are const unsigned short*).
// Both classes are 8 bytes: buffer pointer +0, excess/length info +4.
#ifndef TCSTRING_H
#define TCSTRING_H
#include "TMemory.h"

namespace Toshi {

typedef unsigned short TWChar;
class TWString;

class TCString
{
public:
    TOSHI_API TCString();
    TOSHI_API TCString(int reserve);
    TOSHI_API TCString(const char* s);
    TOSHI_API TCString(const TWChar* s);
    TOSHI_API TCString(const TCString& rhs);
    TOSHI_API TCString(const TWString& rhs);
    TOSHI_API ~TCString();

    TOSHI_API TCString& operator=(TCString& rhs);          // non-const ref in the original
    TOSHI_API TCString& operator=(TWString& rhs);
    TOSHI_API TCString& operator=(const char* s);
    TOSHI_API TCString& operator=(const TWChar* s);
    TOSHI_API TCString& operator+=(const TCString& s);
    TOSHI_API TCString& operator+=(const TWString& s);
    TOSHI_API TCString& operator+=(const char* s);
    TOSHI_API TCString operator+(const TCString& s) const;
    TOSHI_API TCString operator+(const TWString& s) const;
    TOSHI_API TCString operator+(const char* s) const;
    TOSHI_API TCString operator+(const TWChar* s) const;
    TOSHI_API bool operator!() const;
    TOSHI_API bool operator==(const TCString& s) const;
    TOSHI_API bool operator==(const char* s) const;
    TOSHI_API bool operator!=(const TCString& s) const;
    TOSHI_API bool operator!=(const char* s) const;
    TOSHI_API bool operator<(const TCString& s) const;
    TOSHI_API bool operator<(const char* s) const;
    TOSHI_API bool operator<=(const TCString& s) const;
    TOSHI_API bool operator<=(const char* s) const;
    TOSHI_API bool operator>(const TCString& s) const;
    TOSHI_API bool operator>(const char* s) const;
    TOSHI_API bool operator>=(const TCString& s) const;
    TOSHI_API bool operator>=(const char* s) const;
    TOSHI_API char& operator[](int i);
    TOSHI_API const char& operator[](int i) const;
    TOSHI_API operator const char*() const;

    TOSHI_API int Compare(const char* s, int len) const;
    TOSHI_API int CompareNoCase(const char* s, int len) const;
    TOSHI_API TCString& Concat(const TCString& s, int len);
    TOSHI_API TCString& Concat(const TWString& s, int len);
    TOSHI_API TCString& Concat(const char* s, int len);
    TOSHI_API void Copy(const TCString& s, int len);
    TOSHI_API void Copy(const TWString& s, int len);
    TOSHI_API void Copy(const char* s, int len);
    TOSHI_API void Copy(const TWChar* s, int len);
    TOSHI_API int ExcessLength() const;
    TOSHI_API int Find(char c, int start) const;
    TOSHI_API int Find(const char* s, int start) const;
    TOSHI_API int FindReverse(char c, int start) const;
    TOSHI_API void ForceSetData(char* data, int len);
    TOSHI_API void UndoForceSetData();
    TOSHI_API TCString& __cdecl Format(const char* fmt, ...);     // QAA = __cdecl member (varargs)
    TOSHI_API TCString& VFormat(const char* fmt, char* args);
    TOSHI_API const char* GetString(int index) const;
    TOSHI_API bool IsAllLowerCase() const;
    TOSHI_API bool IsAllUpperCase() const;
    TOSHI_API bool IsEmpty() const;
    TOSHI_API bool IsIndexValid(int i) const;
    TOSHI_API bool IsUnicode() const;
    TOSHI_API int Length() const;
    TOSHI_API TCString& MakeLower();
    TOSHI_API TCString& MakeUpper();
    TOSHI_API TCString Mid(int start, int len) const;
    TOSHI_API const TCString& Print() const;
    TOSHI_API void Truncate(int len);

private:
    TOSHI_API bool AllocBuffer(int len, bool keep);
    TOSHI_API void FreeBuffer();
    TOSHI_API void Reset();
    TOSHI_API static char* m_aNull;

    char* m_pBuffer;        // +0
    int   m_iExcess;        // +4
};

class TWString
{
public:
    TOSHI_API TWString();
    TOSHI_API TWString(int reserve);
    TOSHI_API TWString(const char* s);
    TOSHI_API TWString(const TWChar* s);
    TOSHI_API TWString(const TWString& rhs);
    TOSHI_API TWString(const TCString& rhs);
    TOSHI_API ~TWString();

    TOSHI_API TWString& operator=(TWString& rhs);
    TOSHI_API TWString& operator=(TCString& rhs);
    TOSHI_API TWString& operator=(const char* s);
    TOSHI_API TWString& operator=(const TWChar* s);
    TOSHI_API TWString& operator+=(const TWString& s);
    TOSHI_API TWString& operator+=(const TWChar* s);
    TOSHI_API TWString operator+(const TWString& s) const;
    TOSHI_API TWString operator+(const TWChar* s) const;
    TOSHI_API bool operator!() const;
    TOSHI_API bool operator==(const TWString& s) const;
    TOSHI_API bool operator==(const TWChar* s) const;
    TOSHI_API bool operator!=(const TWString& s) const;
    TOSHI_API bool operator!=(const TWChar* s) const;
    TOSHI_API bool operator<(const TWString& s) const;
    TOSHI_API bool operator<(const TWChar* s) const;
    TOSHI_API bool operator<=(const TWString& s) const;
    TOSHI_API bool operator<=(const TWChar* s) const;
    TOSHI_API bool operator>(const TWString& s) const;
    TOSHI_API bool operator>(const TWChar* s) const;
    TOSHI_API bool operator>=(const TWString& s) const;
    TOSHI_API bool operator>=(const TWChar* s) const;
    TOSHI_API TWChar& operator[](int i) const;
    TOSHI_API operator const TWChar*() const;

    TOSHI_API int Compare(const TWChar* s, int len) const;
    TOSHI_API int CompareNoCase(const TWChar* s, int len) const;
    TOSHI_API TWString& Concat(const TWString& s, int len);
    TOSHI_API TWString& Concat(const TWChar* s, int len);
    TOSHI_API void Copy(const TWString& s, int len);
    TOSHI_API void Copy(const TCString& s, int len);
    TOSHI_API void Copy(const char* s, int len);
    TOSHI_API void Copy(const TWChar* s, int len);
    TOSHI_API int ExcessLength() const;
    TOSHI_API int Find(TWChar c, int start) const;
    TOSHI_API int Find(const TWChar* s, int start) const;
    TOSHI_API int FindReverse(TWChar c, int start) const;
    TOSHI_API TWString& __cdecl Format(const TWChar* fmt, ...);
    TOSHI_API TWString& VFormat(const TWChar* fmt, char* args);
    TOSHI_API const TWChar* GetString(int index) const;
    TOSHI_API bool IsEmpty() const;
    TOSHI_API bool IsIndexValid(int i) const;
    TOSHI_API bool IsUnicode() const;
    TOSHI_API int Length() const;
    TOSHI_API TWString& MakeLower();
    TOSHI_API TWString& MakeUpper();
    TOSHI_API TWString Mid(int start, int len) const;
    TOSHI_API const TWString& Print() const;
    TOSHI_API void Truncate(int len);

private:
    TOSHI_API bool AllocBuffer(int len, bool keep);
    TOSHI_API void FreeBuffer();
    TOSHI_API void Reset();
    TOSHI_API static TWChar* m_aNull;

    TWChar* m_pBuffer;      // +0
    int     m_iExcess;      // +4
};

// free operators (YG = __stdcall, the -Gz default)
TOSHI_API TCString operator+(const char* a, const TCString& b);
TOSHI_API TWString operator+(const TWChar* a, const TWString& b);
TOSHI_API bool operator==(const char* a, const TCString& b);
TOSHI_API bool operator!=(const char* a, const TCString& b);
TOSHI_API bool operator<(const char* a, const TCString& b);
TOSHI_API bool operator<=(const char* a, const TCString& b);
TOSHI_API bool operator>(const char* a, const TCString& b);
TOSHI_API bool operator>=(const char* a, const TCString& b);
TOSHI_API bool operator==(const TWChar* a, const TWString& b);
TOSHI_API bool operator!=(const TWChar* a, const TWString& b);
TOSHI_API bool operator<(const TWChar* a, const TWString& b);
TOSHI_API bool operator<=(const TWChar* a, const TWString& b);
TOSHI_API bool operator>(const TWChar* a, const TWString& b);
TOSHI_API bool operator>=(const TWChar* a, const TWString& b);

} // namespace Toshi
#endif
