// Toshi math helpers (reconstructed). Min/Max take and return const references: the binary
// compares through the ADDRESS of the operands (lea/cmp [mem]) when they are named objects.
#ifndef TMATH_H
#define TMATH_H
#include "TMemory.h"
namespace Toshi {
class TMath
{
public:
    TOSHI_API static const float PI;
    TOSHI_API static const float MAXFLOAT;
    TOSHI_API static float __stdcall ATan(float);
    TOSHI_API static int __stdcall TruncToInt(float);
    TOSHI_API static int __stdcall FloorToInt(float);
    template <class T> static const T& Max(const T& a, const T& b) { return (a < b) ? b : a; }
    template <class T> static const T& Min(const T& a, const T& b) { return (b < a) ? b : a; }
};
}
#endif
