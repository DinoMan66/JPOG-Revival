// Inline replacement operator new/delete, as used by the Toshi DLLs that do NOT define them
// out-of-line (TInputInterface.dll has no operator new/delete function of its own: the compiler
// inlines tmalloc/tfree straight into the vector deleting destructors).
// (TApplication.dll is different: it defines them as real functions - see TApplication.cpp.)
#ifndef TNEWDELETE_H
#define TNEWDELETE_H
#include <stddef.h>
#include "TMemory.h"

// A file that defines them out of line itself (PProperties.cpp, like TApplication.cpp)
// defines TOSHI_NEWDELETE_OUTOFLINE first and provides the bodies.
#ifdef TOSHI_NEWDELETE_OUTOFLINE
void* __cdecl operator new(size_t size);
void  __cdecl operator delete(void* p);
#else
inline void* __cdecl operator new(size_t size) { return tmalloc((int)size, 0, -1); }
inline void  __cdecl operator delete(void* p)  { tfree(p); }
#endif

// placement new (as in <new.h>), used by CreateObjectInPlace
#ifndef __PLACEMENT_NEW_INLINE
#define __PLACEMENT_NEW_INLINE
inline void* __cdecl operator new(size_t, void* p) { return p; }
#endif

#endif
