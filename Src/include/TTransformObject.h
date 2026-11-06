// Toshi::TTransformData / TTransformObject (TRenderInterface.dll, TTransformObject.cpp,
// 0x11970-0x12190). TTransformObject derives from TSceneObject and (second base, +0x34)
// TTransformData: a rotation as quaternion, euler angles or a full matrix, plus translate/scale.
// The small accessors are inline (emitted with TCameraObject.cpp, 0xf4c0-0xf5a0).
#ifndef TTRANSFORMOBJECT_H
#define TTRANSFORMOBJECT_H
#include "TSceneObject.h"
#include "TVector.h"

namespace Toshi {

class TScene;

class TTransformData                                                    // 0x60
{
public:
    enum MODE { MODE_QUAT, MODE_EULER, MODE_MATRIX };
    enum EROT { EROT_X, EROT_Y, EROT_Z };

    // the rotation storage is shared by the three modes (quaternion / euler / matrix)
    TQuaternion& Quat() { return *(TQuaternion*)&m_oMatrix; }
    TVector3& Euler() { return *(TVector3*)&m_oMatrix; }

    TMatrix44     m_oMatrix;            // +0x00 (+0x34 in TTransformObject)
    MODE          m_eMode;              // +0x40
    TVector3      m_vTranslate;         // +0x44
    TVector3      m_vScale;             // +0x50
    unsigned char m_aEulerOrder[4];     // +0x5c (EROT values; copied as one dword)
};

class TRENDER_API TTransformObject : public TSceneObject, public TTransformData // 0x94
{
    TOSHI_DECLARE_CLASS(TTransformObject)
public:
    TTransformObject();
    TTransformObject(const TTransformObject& other);
    virtual ~TTransformObject();
    TTransformObject& operator=(const TTransformObject& other);

    virtual void GetLocalMatrix(TMatrix44& matrix);                     // slot 13
    virtual bool RenderOld(TScene& scene);                              // slot 14

    void Push(TScene& scene);
    void Pop(TScene& scene);
    void SetMatrix(const TMatrix44& matrix);
    void SetTranslate(const TVector3& translate);
    void SetEulerOrder(EROT first, EROT second, EROT third);
    void SetQuat(const TQuaternion& quat);
    void SetEuler(const TVector3& euler);
    MODE SetMode(MODE mode);

    const TVector3& GetScale() { return m_vScale; }
    const TVector3& GetTranslate()
    {
        if (m_eMode == MODE_MATRIX)
            return *(const TVector3*)&m_oMatrix.m_f41;
        return m_vTranslate;
    }
    const TMatrix44& GetMatrix() { return m_oMatrix; }
    const TQuaternion& GetQuat() { return Quat(); }
    const TVector3& GetEuler() { return Euler(); }
    MODE GetMode() { return m_eMode; }
    void SetScale(float x, float y, float z) { m_vScale.x = x; m_vScale.y = y; m_vScale.z = z; }
    void SetScale(float scale) { m_vScale.x = scale; m_vScale.y = scale; m_vScale.z = scale; }
    void SetScale(const TVector3& scale) { m_vScale = scale; }
};

} // namespace Toshi
#endif
