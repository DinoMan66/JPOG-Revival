// Toshi::TShader / TMaterial / TMesh (TRenderInterface.dll, TShader.cpp, 0xdb90-0xde90).
// All three are abstract resource bases; the platform renderer derives the real ones.
// Inline members live in the inline region (0x6a70-0x6cb8) with the implicit copy constructors.
#ifndef TSHADER_H
#define TSHADER_H
#include "TResource.h"

namespace Toshi {

class TRegMaterial;
class TRenderPacket;

class TRENDER_API TShader : public TResource                            // 0x30
{
    TOSHI_DECLARE_CLASS(TShader)
public:
    TShader();
    virtual ~TShader();

    virtual void Flush();                                               // slot 10
    virtual void Render(TRenderPacket* packet) = 0;                     // slot 11 (name unknown)
};

class TRENDER_API TMaterial : public TResource                          // 0x40
{
    TOSHI_DECLARE_CLASS(TMaterial)
public:
    TMaterial();
    virtual ~TMaterial();

    virtual void PreRender() {}                                         // slot 10
    virtual void PostRender() {}                                        // slot 11

    TRegMaterial* GetRegMaterial() { return m_pRegMaterial; }
    unsigned int GetNumTextures() const { return m_uiNumTextures; }
    void SetNumTextures(unsigned int numTextures) { m_uiNumTextures = numTextures; }
    TShader* GetShader() { return m_pShader; }
    void SetFlags(unsigned int flags) { m_uiFlags = flags; }
    unsigned int GetFlags() { return m_uiFlags; }
    void SetFlag(unsigned int flag, bool set);
    void SetRegMaterial(TRegMaterial* regMaterial);

protected:
    void SetShader(TShader* shader);

    unsigned int  m_uiFlags;            // +0x30
    TShader*      m_pShader;            // +0x34
    TRegMaterial* m_pRegMaterial;       // +0x38
    unsigned int  m_uiNumTextures;      // +0x3c
};

class TRENDER_API TMesh : public TResource                              // 0x38
{
    TOSHI_DECLARE_CLASS(TMesh)
public:
    TMesh();
    virtual ~TMesh();

    virtual bool Render() = 0;                                          // slot 10 (pure, with a body)

    TMaterial* GetMaterial() { return m_pMaterial; }
    TShader* GetShader() { return m_pShader; }

protected:
    void SetShader(TShader* shader);
    void SetMaterial(TMaterial* material);

    TMaterial* m_pMaterial;             // +0x30
    TShader*   m_pShader;               // +0x34
};

} // namespace Toshi
#endif
