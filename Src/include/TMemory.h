// Toshi memory + import helpers (reconstructed).
// TKernelInterface.dll exports these; every other Toshi DLL imports them.
//   ?tmalloc@@YGPAXHPADH@Z   void* __stdcall tmalloc(int size, char* file, int line)
//   ?tfree@@YGXPAX@Z         void  __stdcall tfree(void* p)
// The "YG" in the decorated names means __stdcall, which is the default here because
// every Toshi DLL is compiled with -Gz.
#ifndef TMEMORY_H
#define TMEMORY_H

#ifndef TOSHI_API
#define TOSHI_API __declspec(dllimport)     // calls go through the import table: call [__imp_...]
#endif

TOSHI_API void* tmalloc(int size, char* file, int line);
TOSHI_API void  tfree(void* p);

#endif
