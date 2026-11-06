// Export macro for PPropertyParser.dll (Z:\Toshi\Plugins\Source\PPropertyParser\).
// The DLL exports every class whole: the export table lists implicit members too.
#ifndef PPROPERTYPARSERAPI_H
#define PPROPERTYPARSERAPI_H
#ifdef PPROPERTYPARSER_EXPORTS
#define PPROPERTYPARSER_API __declspec(dllexport)
#else
#define PPROPERTYPARSER_API __declspec(dllimport)
#endif

// the plugin classes live in the global namespace but use Toshi types unqualified
namespace Toshi {}
using namespace Toshi;
#endif
