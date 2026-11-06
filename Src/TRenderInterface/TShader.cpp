// TRenderInterface.dll - TShader.cpp: TShader, TMaterial, TMesh (0xdb90-0xde90 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TShader.h"

namespace Toshi {

TOSHI_DEFINE_CLASS(TShader, TResource)                                 // 0xdb90
TOSHI_DEFINE_CLASS(TMaterial, TResource)
TOSHI_DEFINE_CLASS(TMesh, TResource)

static bool s_bMeshRenderResult = true;                                 // 0x1002c660

// ---- TMesh
void TMesh::SetShader(TShader* shader)                                  // 0xdc80
{
    m_pShader = shader;
}

void TMesh::SetMaterial(TMaterial* material)                            // 0xdc90
{
    m_pMaterial = material;
}

bool TMesh::Render()                                                    // 0xdca0
{
    return s_bMeshRenderResult;
}

TMesh::~TMesh()                                                         // 0xdcb0
{
}

TMesh::TMesh()                                                          // 0xdcc0
{
    m_pMaterial = 0;
    m_pShader = 0;
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TMesh)                                     // 0xdce0-0xdd20

// ---- TMaterial
void TMaterial::SetShader(TShader* shader)                              // 0xdd30
{
    m_pShader = shader;
}

void TMaterial::SetRegMaterial(TRegMaterial* regMaterial)               // 0xdd40
{
    m_pRegMaterial = regMaterial;
}

void TMaterial::SetFlag(unsigned int flag, bool set)                    // 0xdd50
{
    if (set)
        m_uiFlags |= flag;
    else
        m_uiFlags &= ~flag;
}

TMaterial::~TMaterial()                                                 // 0xdd70
{
}

TMaterial::TMaterial()                                                  // 0xdd80
{
    m_uiFlags = 0;
    m_pShader = 0;
    m_uiNumTextures = 0;
    m_pRegMaterial = 0;
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TMaterial)                                 // 0xddb0-0xddf0

// ---- TShader
void TShader::Flush()                                                   // 0xde00
{
}

TShader::~TShader()                                                     // 0xde10
{
}

TShader::TShader()                                                      // 0xde20
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS(TShader)                                   // 0xde40-0xde80

} // namespace Toshi
