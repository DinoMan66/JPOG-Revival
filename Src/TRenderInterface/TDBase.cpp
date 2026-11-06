// TRenderInterface.dll - TDBase.cpp (0x133c0-0x13f90 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TDBase.h"
#include "TSystem.h"
#include <math.h>

namespace Toshi {

char TDBase::s_BlockNames[BLOCK_NUMOF][5] = {
    "SMSH", "BONE", "KEYF", "SKEY", "ASEQ", "SSET", "MATL", "IAUX", "SKEL", "MSET", "SMPX",
    "IAPX", "ASE1", "SKE1", "KEY1", "QKEY", "ASE2", "SKE2", "PKEY", "PQKE", "MSE2", "PRMS"
};
float TDBase::s_fDeltaTransEpsilon = 0.01f;
float TDBase::s_fDeltaTransDeviation = 0.05f;

void TDBase::TKeyFrame::Get(float& time, TQuaternion& rotation, TVector4& translation) // 0x133c0
{
    time = m_fTime;
    rotation = m_oRotation;
    translation = m_oTranslation;
}

void TDBase::TKeyFrame::Set(const float& time, const TQuaternion& rotation, const TVector4& translation) // 0x13410
{
    m_fTime = time;
    m_oRotation = rotation;
    m_oTranslation = translation;
}

void TDBase::TKeyFrame::GetInterpolate(TKeyFrame& out, TKeyFrame& a, TKeyFrame& b, float time) // 0x13460
{
    float delta = b.m_fTime - a.m_fTime;
    if (delta == 0.0f)
    {
        out = a;
    }
    else
    {
        float ratio = (time - a.m_fTime) / delta;
        a.m_oRotation.MakePrincple();
        b.m_oRotation.MakePrincple();
        out.m_oRotation.Slerp(a.m_oRotation, b.m_oRotation, ratio);
        out.m_oTranslation.Lerp(a.m_oTranslation, b.m_oTranslation, ratio);
        out.m_fTime = time;
    }
    out.m_oRotation.MakePrincple();
}

void TDBase::TKeySequence::GenerateKey(TKeyFrame& key, float time)     // 0x13560
{
    TKeyFrame* prev = 0;
    TKeyFrame* next = 0;
    for (TNodeList<TKeyFrame>::Iterator it = m_oKeys.Begin(); it != m_oKeys.End(); ++it)
    {
        if (it->m_fTime <= time)
            prev = it.Get();
        if (it->m_fTime >= time)
        {
            next = it.Get();
            break;
        }
    }

    if (next)
    {
        if (prev)
            TKeyFrame::GetInterpolate(key, *prev, *next, time);
        else
        {
            key = *next;
            key.m_oRotation.MakePrincple();
        }
    }
    else if (prev)
    {
        key = *prev;
        key.m_oRotation.MakePrincple();
    }
    else
        key.Set(time, TQuaternion::IDENTITY, TVector4::VEC_ZERO);
}

void TDBase::TKeySequence::GenerateCloseKey(TKeyFrame& key, float time) // 0x136a0
{
    TKeyFrame* prev = 0;
    TKeyFrame* next = 0;
    for (TNodeList<TKeyFrame>::Iterator it = m_oKeys.Begin(); it != m_oKeys.End(); ++it)
    {
        if (it->m_fTime <= time)
            prev = it.Get();
        if (it->m_fTime >= time)
        {
            next = it.Get();
            break;
        }
    }

    if (next)
    {
        if (prev)
        {
            if (fabs(prev->m_fTime - time) <= m_fFrameTime)
                key = *prev;
            else if (fabs(next->m_fTime - time) < m_fFrameTime)
                key = *next;
            else
                TKeyFrame::GetInterpolate(key, *prev, *next, time);
        }
        else
        {
            key = *next;
            key.m_oRotation.MakePrincple();
        }
    }
    else if (prev)
    {
        key = *prev;
        key.m_oRotation.MakePrincple();
    }
    else
        key.Set(time, TQuaternion::IDENTITY, TVector4::VEC_ZERO);
}

void TDBase::TKeySequence::GenerateExportCloseKey(TKeyFrame& key, float time) // 0x13890
{
    TKeyFrame* prev = 0;
    TKeyFrame* next = 0;
    for (TNodeList<TKeyFrame>::Iterator it = m_oKeys.Begin(); it != m_oKeys.End(); ++it)
    {
        if (it->m_fTime <= time)
            prev = it.Get();
        if (it->m_fTime >= time)
        {
            next = it.Get();
            break;
        }
    }

    if (next)
    {
        if (prev)
        {
            if (fabs(prev->m_fTime - time) <= fabs(next->m_fTime - time))
                key = *prev;
            else
                key = *next;
        }
        else
        {
            key = *next;
            key.m_oRotation.MakePrincple();
        }
    }
    else if (prev)
    {
        key = *prev;
        key.m_oRotation.MakePrincple();
    }
    else
        key.Set(time, TQuaternion::IDENTITY, TVector4::VEC_ZERO);
}

void TDBase::TKeySequence::DeleteData()                                 // 0x13a50
{
    while (!m_oKeys.IsEmpty())
    {
        TKeyFrame* key = m_oKeys.RemoveHead();
        delete key;
    }
}

void TDBase::TSequence::DeleteData()                                    // 0x13ab0
{
    while (!m_oKeySeqs.IsEmpty())
    {
        TKeySequence* keySeq = m_oKeySeqs.RemoveHead();
        delete keySeq;
    }
}

void TDBase::TSequence::SetFlag(unsigned short flags)                   // 0x13b70
{
    m_usFlags = flags;
}

TNodeList<TDBase::TKeySequence>& TDBase::TSequence::GetKeySeqs()        // 0x13b80
{
    return m_oKeySeqs;
}

void TDBase::TSequence::SetSeqName(const char* name)                    // 0x13b90
{
    TSystem::StringCopy(m_szName, name, 15);
}

void TDBase::TSequence::SetNumKeySeqs(unsigned int count)               // 0x13bb0
{
    m_uiNumKeySeqs = count;
}

const char* TDBase::TSequence::GetSeqName()                            // 0x13bc0
{
    return m_szName;
}

void TDBase::TSequence::GetNumKeySeqs(unsigned int& count)              // 0x13bd0
{
    count = m_uiNumKeySeqs;
}

TDBase::TSequence::~TSequence()                                         // 0x13be0
{
    DeleteData();
}

TDBase::TSequence::TSequence()                                          // 0x13c90
{
}

TDBase::TKeySequence& TDBase::TKeySequence::operator=(TKeySequence& other) // 0x13cb0
{
    TSystem::StringCopy(m_szBoneName, other.m_szBoneName, 15);
    return *this;
}

TNodeList<TDBase::TKeyFrame>& TDBase::TKeySequence::GetKeys()           // 0x13cd0
{
    return m_oKeys;
}

void TDBase::TKeySequence::SetAssociatedBoneName(const char* name)      // 0x13ce0
{
    TSystem::StringCopy(m_szBoneName, name, 15);
}

void TDBase::TKeySequence::SetNumKeys(unsigned int count)               // 0x13d00
{
    m_uiNumKeys = count;
}

const char* TDBase::TKeySequence::GetAssociatedBoneName()               // 0x13d10
{
    return m_szBoneName;
}

void TDBase::TKeySequence::GetNumKeys(unsigned int& count)              // 0x13d20
{
    count = m_uiNumKeys;
}

TDBase::TKeySequence::TKeySequence()                                    // 0x13d90
    : m_bLocked(false)
{
    m_bTranslateAnimating = false;
    m_fFrameTime = 1.0f / 60.0f;
}

bool TDBase::TKeyFrame::IsInExclusiveTimeRange(float start, float end)  // 0x13dc0
{
    if (m_fTime <= start) return false;
    if (m_fTime >= end) return false;
    return true;
}

bool TDBase::TKeyFrame::IsInTimeRange(float start, float end)           // 0x13df0
{
    if (m_fTime < start) return false;
    if (m_fTime > end) return false;
    return true;
}

TDBase::TKeyFrame& TDBase::TKeyFrame::operator=(const TKeyFrame& other) // 0x13e20
{
    m_fTime = other.m_fTime;
    m_oRotation = other.m_oRotation;
    m_oTranslation = other.m_oTranslation;
    return *this;
}

TDBase::TKeyFrame::~TKeyFrame()                                         // 0x13e60
{
}

TDBase::TKeyFrame::TKeyFrame()                                          // 0x13e70
{
}

TDBase::TDBase()                                                        // 0x13e80
{
}

} // namespace Toshi
