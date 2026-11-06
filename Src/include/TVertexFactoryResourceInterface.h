// Toshi::TVertexFactoryResourceInterface (TRenderInterface.dll, TVertexFactoryResourceInterface.cpp).
// 0xe600-0xe7f0.
#ifndef TVERTEXFACTORYRESOURCEINTERFACE_H
#define TVERTEXFACTORYRESOURCEINTERFACE_H
#include "TResource.h"

namespace Toshi {

// Not exported: plain classes with inline constructors (the 8-stream loop is inlined at each use).
class TVertexStreamFormat
{
public:
    TVertexStreamFormat() : m_uiVertexSize(0), m_uiUnk(0) {}

    unsigned short m_uiVertexSize;  // +0
    unsigned int   m_uiUnk;         // +4
};

class TVertexFactoryFormat
{
public:
    enum { MAX_STREAMS = 8 };
    TVertexFactoryFormat() : m_uiNumStreams(0) {}

    unsigned int        m_uiNumStreams;                 // +0x00
    TVertexStreamFormat m_aStreamFormats[MAX_STREAMS];  // +0x04
};

class TVertexPoolResourceInterface;

class TRENDER_API TVertexFactoryResourceInterface : public TResource     // 0x80
{
    TOSHI_DECLARE_CLASS(TVertexFactoryResourceInterface)
    friend class TVertexPoolResourceInterface;
public:
    TVertexFactoryResourceInterface();
    virtual ~TVertexFactoryResourceInterface();

    virtual bool Create(TVertexFactoryFormat* format, unsigned short maxStaticVertices,
                        unsigned int flags);                                            // slot 10
    virtual TVertexPoolResourceInterface* CreatePool(unsigned short maxVertices, unsigned int flags) = 0; // slot 11

    unsigned int GetFlags();
    unsigned short GetMaxStaticVertices();
    TVertexFactoryFormat* GetVertexFormat();

protected:
    unsigned int         m_uiNumPools;              // +0x30
    TVertexFactoryFormat m_oVertexFormat;           // +0x34
    unsigned short       m_usMaxStaticVertices;     // +0x78
    unsigned int         m_uiFlags;                 // +0x7c

private:
    TVertexFactoryResourceInterface(const TVertexFactoryResourceInterface&);    // not exported
    TVertexFactoryResourceInterface& operator=(const TVertexFactoryResourceInterface&);
};

} // namespace Toshi
#endif
