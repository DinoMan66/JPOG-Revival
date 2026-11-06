// Z:\Toshi\Plugins\Source\PPropertyParser\PPropertyBlock.cpp  (reconstruction)
// Range in PPropertyParser.dll: 0x2a60-0x2c6f (??_E, ctor, class object, dtor, class
// statics, then the inline SetName/GetPriority/GetName).
#define PPROPERTYPARSER_EXPORTS
#include "TNewDelete.h"
#include "PPropertyBlock.h"

PPropertyBlock::PPropertyBlock(const TPCString& name, Priority priority)
    : m_oName(name)
{
    m_ePriority = priority;
}

TOSHI_DEFINE_CLASS(PPropertyBlock, TObject)

PPropertyBlock::~PPropertyBlock()
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS(PPropertyBlock)
