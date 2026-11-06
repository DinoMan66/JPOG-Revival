// Toshi::TFreeList - fixed-size block allocator (stub; lives in TKernelInterface.dll).
//   ??0TFreeList@Toshi@@QAE@HHHPAD@Z   TFreeList(int blockSize, int, int, char* name)
//   ??1TFreeList@Toshi@@QAE@XZ
//   ?New@TFreeList@Toshi@@QAEPAXI@Z    void* New(unsigned int size)
//   ?Delete@TFreeList@Toshi@@QAEXPAX@Z void  Delete(void* p)
// Classes that use one declare a private static ms_oFreelist plus inline class-level
// operator new/delete (see TOSHI_FREELIST_DECLARE).
#ifndef TFREELIST_H
#define TFREELIST_H
#include <stddef.h>
#include "TMemory.h"

namespace Toshi {

class TFreeList
{
public:
    TOSHI_API TFreeList(int blockSize, int initial, int grow, char* name);
    TOSHI_API ~TFreeList();
    TOSHI_API void* New(unsigned int size);
    TOSHI_API void  Delete(void* p);
private:
    unsigned char m_data[0x20];
};

} // namespace Toshi

// Declared in this order, ICC emits them in reverse (new(file,line), new, placement new,
// delete, GetFreeList is how they appear in the binary).
#define TOSHI_FREELIST_DECLARE()                                                             \
public:                                                                                      \
    static Toshi::TFreeList& GetFreeList() { return ms_oFreelist; }                          \
    static void operator delete(void* p) { ms_oFreelist.Delete(p); }                         \
    static void* operator new(size_t, void* p) { return p; }                                 \
    static void* operator new(size_t size) { return ms_oFreelist.New(size); }               \
    static void* operator new(size_t size, char*, int) { return ms_oFreelist.New(size); }   \
private:                                                                                     \
    static Toshi::TFreeList ms_oFreelist;                                                    \
public:

#endif
