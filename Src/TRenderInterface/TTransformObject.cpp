// TRenderInterface.dll - TTransformObject.cpp (0x11970-0x12190 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TTransformObject.h"
#include "TScene.h"
#include "TRenderInterface.h"
#include "TRenderContext.h"

#ifndef TTO_SW
#define TTO_SW 0
#endif

namespace Toshi {

TTransformObject::TTransformObject()                                    // 0x11970
{
    SetEulerOrder(EROT_Z, EROT_Y, EROT_X);
    SetMode(MODE_QUAT);
    SetQuat(TQuaternion::IDENTITY);
    SetTranslate(TVector3::VEC_ZERO);
    m_vScale.x = 1.0f;
    m_vScale.y = 1.0f;
    m_vScale.z = 1.0f;
}

TTransformObject::TTransformObject(const TTransformObject& other)        // 0x119e0
    : TSceneObject(), TTransformData(other)
{
}

void TTransformObject::Push(TScene& scene)                              // 0x11ac0
{
    TStack<TMatrix44, 20>& stack = scene.GetRenderMatrixStack();
    TMatrix44& parent = stack.Top();
    TMatrix44& matrix = stack.PushNull();
#if TTO_SW
    switch (m_eMode)
    {
    case MODE_QUAT:
        matrix.PushQuaternion(GetQuat(), parent, GetTranslate());
        matrix.Scale(m_vScale.x, m_vScale.y, m_vScale.z);
        break;
    case MODE_EULER:
        matrix = parent;
        matrix.Translate(GetTranslate());
        for (int i = 0; i < 3; i++)
        {
            switch (m_aEulerOrder[i])
            {
            case EROT_X: matrix.RotateX(GetEuler().x); break;
            case EROT_Y: matrix.RotateY(GetEuler().y); break;
            case EROT_Z: matrix.RotateZ(GetEuler().z); break;
            }
        }
        matrix.Scale(m_vScale.x, m_vScale.y, m_vScale.z);
        break;
    default:
        matrix.Multiply(parent, m_oMatrix);
        break;
    }
#else
    if (m_eMode == MODE_QUAT)
    {
        matrix.PushQuaternion(GetQuat(), parent, GetTranslate());
        matrix.Scale(m_vScale.x, m_vScale.y, m_vScale.z);
    }
    else if (m_eMode == MODE_EULER)
    {
        matrix = parent;
        matrix.Translate(GetTranslate());
        for (int i = 0; i < 3; i++)
        {
            switch (m_aEulerOrder[i])
            {
            case EROT_X: matrix.RotateX(GetEuler().x); break;
            case EROT_Y: matrix.RotateY(GetEuler().y); break;
            case EROT_Z: matrix.RotateZ(GetEuler().z); break;
            }
        }
        matrix.Scale(m_vScale.x, m_vScale.y, m_vScale.z);
    }
    else
        matrix.Multiply(parent, m_oMatrix);
#endif
    GetRenderer()->GetCurrentRenderContext()->SetModelViewMatrix(stack.Top());
}

void TTransformObject::Pop(TScene& scene)                               // 0x11cb0
{
    TStack<TMatrix44, 20>& stack = scene.GetRenderMatrixStack();
    stack.Pop();
    GetRenderer()->GetCurrentRenderContext()->SetModelViewMatrix(stack.Top());
}

void TTransformObject::SetMatrix(const TMatrix44& matrix)               // 0x11cf0
{
    m_eMode = MODE_MATRIX;
    m_oMatrix = matrix;
    matrix.GetTranslate(m_vTranslate);
}

void TTransformObject::SetTranslate(const TVector3& translate)          // 0x11d70
{
    m_vTranslate = translate;
    if (m_eMode == MODE_MATRIX)
        m_oMatrix.SetTranslate(m_vTranslate);
}

void TTransformObject::GetLocalMatrix(TMatrix44& matrix)                // 0x11db0
{
#if TTO_SW
    switch (m_eMode)
    {
    case MODE_QUAT:
        matrix.SetFromQuaternion(GetQuat());
        matrix.SetTranslate(GetTranslate());
        matrix.Scale(m_vScale.x, m_vScale.y, m_vScale.z);
        break;
    case MODE_EULER:
        matrix.Identity();
        matrix.SetTranslate(GetTranslate());
        for (int i = 0; i < 3; i++)
        {
            switch (m_aEulerOrder[i])
            {
            case EROT_X: matrix.RotateX(GetEuler().x); break;
            case EROT_Y: matrix.RotateY(GetEuler().y); break;
            case EROT_Z: matrix.RotateZ(GetEuler().z); break;
            }
        }
        matrix.Scale(m_vScale.x, m_vScale.y, m_vScale.z);
        break;
    default:
        matrix = m_oMatrix;
        break;
    }
#else
    if (m_eMode == MODE_QUAT)
    {
        matrix.SetFromQuaternion(GetQuat());
        matrix.SetTranslate(GetTranslate());
        matrix.Scale(m_vScale.x, m_vScale.y, m_vScale.z);
    }
    else if (m_eMode == MODE_EULER)
    {
        matrix.Identity();
        matrix.SetTranslate(GetTranslate());
        for (int i = 0; i < 3; i++)
        {
            switch (m_aEulerOrder[i])
            {
            case EROT_X: matrix.RotateX(GetEuler().x); break;
            case EROT_Y: matrix.RotateY(GetEuler().y); break;
            case EROT_Z: matrix.RotateZ(GetEuler().z); break;
            }
        }
        matrix.Scale(m_vScale.x, m_vScale.y, m_vScale.z);
    }
    else
        matrix = m_oMatrix;
#endif
}

TOSHI_DEFINE_CLASS(TTransformObject, TSceneObject)                     // 0x11f40

void TTransformObject::SetEulerOrder(EROT first, EROT second, EROT third) // 0x11f90
{
    m_aEulerOrder[0] = (unsigned char)first;
    m_aEulerOrder[1] = (unsigned char)second;
    m_aEulerOrder[2] = (unsigned char)third;
}

void TTransformObject::SetQuat(const TQuaternion& quat)                 // 0x11fc0
{
    m_eMode = MODE_QUAT;
    Quat() = quat;
}

void TTransformObject::SetEuler(const TVector3& euler)                  // 0x11ff0
{
    m_eMode = MODE_EULER;
    Euler() = euler;
}

TTransformData::MODE TTransformObject::SetMode(MODE mode)               // 0x12010
{
    MODE old = m_eMode;
    m_eMode = mode;
    return old;
}

bool TTransformObject::RenderOld(TScene& scene)                         // 0x12020
{
    Push(scene);
    bool result = TSceneObject::Render(scene);
    Pop(scene);
    return result;
}

TTransformObject& TTransformObject::operator=(const TTransformObject& other) // 0x12050
{
    if (&other != this)
        TTransformData::operator=(other);
    return *this;
}

TTransformObject::~TTransformObject()                                   // 0x12080
{
}

TOSHI_DEFINE_CLASS_FUNCTIONS_CONCRETE(TTransformObject)                 // 0x12090-0x12100

} // namespace Toshi
