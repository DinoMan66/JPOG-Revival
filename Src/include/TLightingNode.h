// Toshi::TLightingNode (TRenderInterface.dll, TLightingNode.cpp, 0x10310-0x108a0).
// While its children render, swaps in its own light direction/colour matrices and ambient colour.
#ifndef TLIGHTINGNODE_H
#define TLIGHTINGNODE_H
#include "TSceneObject.h"
#include "TVector.h"

namespace Toshi {

class TRENDER_API TLightingNode : public TSceneObject                  // 0xc4
{
    TOSHI_DECLARE_CLASS(TLightingNode)
public:
    TLightingNode();
    virtual ~TLightingNode();

    virtual bool Render(TScene& scene);                                 // slot 11

    const TVector4& GetAmbientColour() const { return m_oAmbientColour; }
    const TMatrix44& GetLightColourMatrix() const { return m_oLightColour; }
    const TMatrix44& GetLightDirectionMatrix() const { return m_oLightDirection; }
    void SetAmbientColour(float r, float g, float b) { m_oAmbientColour.Set(r, g, b, 0.0f); }
    void SetAmbientColour(const TVector4& colour) { m_oAmbientColour = colour; }
    void SetLightColourMatrix(const TMatrix44& matrix) { m_oLightColour = matrix; }
    void SetLightDirectionMatrix(const TMatrix44& matrix) { m_oLightDirection = matrix; }

protected:
    TMatrix44 m_oLightDirection;    // +0x34
    TMatrix44 m_oLightColour;       // +0x74
    TVector4  m_oAmbientColour;     // +0xb4

private:
    TLightingNode(const TLightingNode&);                                // not exported
    TLightingNode& operator=(const TLightingNode&);
};

} // namespace Toshi
#endif
