// Export macro for TRenderInterface.dll (Toshi render core). Every class is exported whole.
#ifndef TRENDERAPI_H
#define TRENDERAPI_H
#ifdef TRENDERINTERFACE_EXPORTS
#define TRENDER_API __declspec(dllexport)
#else
#define TRENDER_API __declspec(dllimport)
#endif
#endif
