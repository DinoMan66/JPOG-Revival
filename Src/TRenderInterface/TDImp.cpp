// TRenderInterface.dll - TDImp.cpp (0x13f90-0x18690 in the original)
// Built WITHOUT TRENDERINTERFACE_EXPORTS: the original reaches TRenderInterface::s_Interface and
// TMaterialLibraryManager::GetMaterial through __imp_ pointers. TDImp/TDBase use TDIMP_API.
// Not written yet: ComputeBoneBox, BoneSpaceTransform, BuildImpParentChildHierarchy, BoneRemap,
// ComposeHierarchy, ReorderAllKeySeqs, CollapseSequences, the Import* readers, MarkMeshSet,
// TBone::ComputeManualAndHook / ReorderKeySeqs and IsManualRestricted.
#include "TNewDelete.h"
#include "TDImp.h"
#include "TFile.h"
#include "TCString.h"
#include "TRenderInterface.h"
#include "TMath.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

namespace Toshi {

// An IAPX vertex: the IAUX one plus a fourth dword (only the base part is cleared).
struct SkinVertexP : public TDBase::SkinVertex                          // 0x34
{
    float m_fExtra;
};

// The bone names collected by MarkMeshSet (a local list, freed through a compiler-made deleting dtor).
struct TBoneNameNode : public TNodeList<TBoneNameNode>::TNode           // 0x14
{
    TCString m_oName;
};

TDImp::TDImp()                                                          // 0x13f90
    : m_pSkeleton(0)
{
    m_puiManualRemapTable = 0;
    m_puiManualRemapTableInverse = 0;
}

bool TDImp::Create(const char* fileName)                                // 0x14010
{
    TCString name;
    name.Copy(fileName, -1);
    m_pFile = TFile::Create(name, 1);
    if (!m_pFile)
        return false;

    unsigned int numMeshSetsA;
    unsigned int numMeshSetsB;
    ImportMeshSet(numMeshSetsB, numMeshSetsA, this);
    for (unsigned int i = 0; i < numMeshSetsB + numMeshSetsA; i++)
    {
        char header[5];
        m_pFile->Read(header, 5);
        m_pFile->Read(&m_uiLODCount, 4);
        for (unsigned int lod = 0; lod < m_uiLODCount; lod++)
        {
            TDImpMaterial* material = new TDImpMaterial(this);
            TSkinMesh* skinMesh = new TSkinMesh(this);
            m_aMaterials[lod].InsertTail(material);
            m_aSkinMeshes[lod].InsertTail(skinMesh);
            skinMesh->SetMaterial(material);
            skinMesh->FileReadSubMeshes(this);
        }
    }

    m_pSkeleton = new TSkeleton(this);
    m_pSkeleton->FileReadBones(this);
    ImportSequenceSet(m_oSequenceSet, this);
    MarkMeshSet();
    TFile::Destroy(m_pFile);
    m_pFile = 0;
    BoneRemap();
    return true;
}

TDImp::TDImp(const char* fileName)                                      // 0x14260
    : m_pSkeleton(0)
{
    Create(fileName);
}

TDImp::~TDImp()                                                         // 0x142f0
{
    tfree(m_puiManualRemapTableInverse);
    tfree(m_puiManualRemapTable);
    if (m_pSkeleton)
        delete m_pSkeleton;
}

void TDImp::BoneSpaceTransform(TBone* bone, TVector3& point, TAABox& box, bool& first) // 0x14a70
{
    TVector3 translation;
    TQuaternion rotation;
    bone->GetPoseC(rotation, translation);
    TVector3 transformed;
    TQuaternion::RotateVector(transformed, rotation, point);
    transformed.Add(translation);
    if (!first)
    {
        box.m_oMin = transformed;
        box.m_oMax = TVector3::VEC_ZERO;
    }
    else
    {
        TAABox pointBox(transformed, TVector3::VEC_ZERO);
        box.BoxUnion(&pointBox, 1);
    }
}

void TDImp::BuildImpParentChildHierarchy()                             // 0x14b40
{
    TNodeList<TBone>& bones = m_pSkeleton->m_oBones;
    for (TNodeList<TBone>::Iterator it = bones.Begin(); it != bones.End();)
    {
        TBone* bone = it.Get();
        ++it;
        bone->SetHasChildren(false);
        for (TNodeList<TBone>::Iterator child = bones.Begin(); child != bones.End();)
        {
            TBone* other = child.Get();
            ++child;
            if (_stricmp(bone->m_szName, other->m_szParentName) == 0)
            {
                bone->SetHasChildren(true);
                other->SetParent(bone);
                goto next;
            }
        }
        bone->SetParent(0);
    next:;
    }
}

void TDImp::ComposeHierarchy()                                          // 0x156f0
{
    TNodeList<TBone>& bones = m_pSkeleton->m_oBones;
    for (TNodeList<TBone>::Iterator it = bones.Begin(); it != bones.End();)
    {
        TBone* bone = it.Get();
        ++it;
        bone->ComposeWithParent();
    }
}

void TDImp::ReorderAllKeySeqs()                                         // 0x15730
{
    for (TNodeList<TSequence>::Iterator seq = m_oSequenceSet.Begin(); seq != m_oSequenceSet.End(); ++seq)
    {
        TNodeList<TKeySequence>& keySeqs = seq->GetKeySeqs();
        TNodeList<TKeySequence> ordered;
        TNodeList<TBone>& bones = m_pSkeleton->m_oBones;
        for (TNodeList<TBone>::Iterator it = bones.Begin(); it != bones.End();)
        {
            TBone* bone = it.Get();
            ++it;
            bool found = false;
            for (TNodeList<TKeySequence>::Iterator k = keySeqs.Begin(); k != keySeqs.End();)
            {
                TKeySequence* keySeq = k.Get();
                ++k;
                if (_stricmp(keySeq->GetAssociatedBoneName(), bone->m_szName) == 0)
                {
                    keySeq->SetNumKeys(keySeq->GetKeys().Count());
                    keySeqs.Remove(keySeq);
                    ordered.InsertTail(keySeq);
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                TKeySequence* keySeq = new TKeySequence;
                TKeyFrame* key = new TKeyFrame;
                keySeq->SetAssociatedBoneName(bone->m_szName);
                keySeq->SetLocked(false);
                keySeq->SetBoneDepth(0);
                keySeq->TranslateAnimating(false);
                TVector4 translation;
                TQuaternion rotation;
                bone->GetPose(rotation, translation);
                float time = 0.0f;
                key->Set(time, rotation, translation);
                keySeq->GetKeys().InsertTail(key);
                keySeq->SetNumKeys(keySeq->GetKeys().Count());
                ordered.InsertTail(keySeq);
            }
        }
        keySeqs.DeleteAll();
        keySeqs.Append(ordered);
    }
}

void TDImp::CollapseSequences(TBone* bone, TVector4& translation, TQuaternion& rotation) // 0x15a50
{
    TVector4 boneTranslation = translation;
    TQuaternion boneRotation = rotation;
    bone->SetPose(boneRotation, boneTranslation);
    for (TNodeList<TSequence>::Iterator seq = m_oSequenceSet.Begin(); seq != m_oSequenceSet.End(); ++seq)
    {
        TNodeList<TKeySequence>& keySeqs = seq->GetKeySeqs();
        for (TNodeList<TKeySequence>::Iterator keySeq = keySeqs.Begin(); keySeq != keySeqs.End(); ++keySeq)
        {
            if (_stricmp(keySeq->GetAssociatedBoneName(), bone->m_szName) == 0)
            {
                TNodeList<TKeyFrame>& keys = keySeq->GetKeys();
                for (TNodeList<TKeyFrame>::Iterator key = keys.Begin(); key != keys.End(); ++key)
                {
                    TKeyFrame* frame = key.Get();
                    ++key;
                    keys.Remove(frame);
                    delete frame;
                }
                keySeq->SetNumKeys(0);
                break;
            }
        }
    }
}

bool TDImp::ImportSequenceSet(TNodeList<TSequence>& set, TDImp* imp)   // 0x15bb0
{
    bool result = false;
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_SSET], -1) == 0)
        {
            result = true;
            unsigned int count;
            imp->m_pFile->Read(&count, 4);
            for (unsigned int i = 0; i < count; i++)
            {
                TSequence* sequence = new TSequence;
                if (!ImportSequence(*sequence, imp))
                {
                    result = false;
                    delete sequence;
                    break;
                }
                set.InsertTail(sequence);
            }
        }
    }
    return result;
}

bool TDImp::ImportSequence(TSequence& sequence, TDImp* imp)              // 0x15ca0
{
    bool result = false;
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_ASEQ], -1) == 0)
        {
            TNodeList<TKeySequence>& keySeqs = sequence.GetKeySeqs();
            result = true;
            char name[16];
            unsigned int count;
            imp->m_pFile->Read(name, 15);
            imp->m_pFile->Read(&count, 4);
            sequence.SetNumKeySeqs(count);
            sequence.SetSeqName(name);
            for (unsigned int i = 0; i < count; i++)
            {
                TKeySequence* keySeq = new TKeySequence;
                if (!ImportKeySequence(*keySeq, imp))
                {
                    result = false;
                    delete keySeq;
                    break;
                }
                keySeqs.InsertTail(keySeq);
            }
        }
        else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_ASE1], -1) == 0)
        {
            TNodeList<TKeySequence>& keySeqs = sequence.GetKeySeqs();
            result = true;
            char name[16];
            unsigned int count;
            float length;
            imp->m_pFile->Read(name, 15);
            imp->m_pFile->Read(&count, 4);
            imp->m_pFile->Read(&length, 4);
            sequence.SetNumKeySeqs(count);
            sequence.SetSeqName(name);
            SetSequenceLength(length);
            for (unsigned int i = 0; i < count; i++)
            {
                TKeySequence* keySeq = new TKeySequence;
                if (!ImportKeySequence(*keySeq, imp))
                {
                    result = false;
                    delete keySeq;
                    break;
                }
                keySeqs.InsertTail(keySeq);
            }
        }
        else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_ASE2], -1) == 0)
        {
            TNodeList<TKeySequence>& keySeqs = sequence.GetKeySeqs();
            result = true;
            char name[16];
            unsigned short flags;
            unsigned int count;
            float length;
            imp->m_pFile->Read(name, 15);
            imp->m_pFile->Read(&flags, 2);
            imp->m_pFile->Read(&count, 4);
            imp->m_pFile->Read(&length, 4);
            sequence.SetFlag(flags);
            sequence.SetNumKeySeqs(count);
            sequence.SetSeqName(name);
            SetSequenceLength(length);
            for (unsigned int i = 0; i < count; i++)
            {
                TKeySequence* keySeq = new TKeySequence;
                if (!ImportKeySequence(*keySeq, imp))
                {
                    result = false;
                    delete keySeq;
                    break;
                }
                keySeqs.InsertTail(keySeq);
            }
        }
    }
    return result;
}

bool TDImp::ImportKeySequence(TKeySequence& keySeq, TDImp* imp)         // 0x16000
{
    bool result = false;
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_SKE2], -1) == 0)
        {
            TNodeList<TKeyFrame>& keys = keySeq.GetKeys();
            result = true;
            char name[16];
            unsigned int count;
            char locked;
            tag_TShortVector3 shortTranslation;
            imp->m_pFile->Read(name, 15);
            imp->m_pFile->Read(&count, 4);
            imp->m_pFile->Read(&locked, 1);
            imp->m_pFile->Read(&shortTranslation, 6);
            keySeq.SetAssociatedBoneName(name);
            keySeq.SetNumKeys(count);
            keySeq.SetLocked(locked);
            imp->m_pFile->Read(header, 5);
            if (TSystem::StringCompare(header, s_BlockNames[BLOCK_PKEY], -1) == 0)
            {
                for (unsigned int i = 0; i < count; i++)
                {
                    TKeyFrame* key = new TKeyFrame;
                    unsigned short packedTime;
                    short packedRotation[4];
                    short packedTranslation[3];
                    imp->m_pFile->Read(&packedTime, 2);
                    imp->m_pFile->Read(packedRotation, 8);
                    imp->m_pFile->Read(packedTranslation, 6);
                    float time = packedTime * GetSequenceLength() * (1.0f / 32768.0f);
                    TQuaternion rotation(packedRotation[0] * (1.0f / 16384.0f), packedRotation[1] * (1.0f / 16384.0f),
                                         packedRotation[2] * (1.0f / 16384.0f), packedRotation[3] * (1.0f / 16384.0f));
                    TVector4 translation(packedTranslation[0] * (1.0f / 256.0f), packedTranslation[1] * (1.0f / 256.0f),
                                         packedTranslation[2] * (1.0f / 256.0f), 1.0f);
                    key->Set(time, rotation, translation);
                    keys.InsertTail(key);
                }
            }
            else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_PQKE], -1) == 0)
            {
                for (unsigned int i = 0; i < count; i++)
                {
                    TKeyFrame* key = new TKeyFrame;
                    unsigned short packedTime;
                    short packedRotation[4];
                    imp->m_pFile->Read(&packedTime, 2);
                    imp->m_pFile->Read(packedRotation, 8);
                    float time = packedTime * GetSequenceLength() * (1.0f / 32768.0f);
                    TQuaternion rotation(packedRotation[0] * (1.0f / 16384.0f), packedRotation[1] * (1.0f / 16384.0f),
                                         packedRotation[2] * (1.0f / 16384.0f), packedRotation[3] * (1.0f / 16384.0f));
                    TVector4 translation(shortTranslation.x * (1.0f / 256.0f), shortTranslation.y * (1.0f / 256.0f),
                                         shortTranslation.z * (1.0f / 256.0f), 1.0f);
                    key->Set(time, rotation, translation);
                    keys.InsertTail(key);
                }
            }
        }
        else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_SKE1], -1) == 0)
        {
            TNodeList<TKeyFrame>& keys = keySeq.GetKeys();
            result = true;
            char name[16];
            unsigned int count;
            char locked;
            tag_TShortVector3 shortTranslation;
            imp->m_pFile->Read(name, 15);
            imp->m_pFile->Read(&count, 4);
            imp->m_pFile->Read(&locked, 1);
            imp->m_pFile->Read(&shortTranslation, 6);
            keySeq.SetAssociatedBoneName(name);
            keySeq.SetNumKeys(count);
            keySeq.SetLocked(locked);
            SetSeqShortTranslation(shortTranslation);
            for (unsigned int i = 0; i < count; i++)
            {
                TKeyFrame* key = new TKeyFrame;
                if (!ImportKey(*key, imp))
                {
                    result = false;
                    delete key;
                    break;
                }
                keys.InsertTail(key);
            }
        }
        else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_SKEY], -1) == 0)
        {
            TNodeList<TKeyFrame>& keys = keySeq.GetKeys();
            result = true;
            char name[16];
            unsigned int count;
            char locked;
            imp->m_pFile->Read(name, 15);
            imp->m_pFile->Read(&count, 4);
            imp->m_pFile->Read(&locked, 1);
            keySeq.SetAssociatedBoneName(name);
            keySeq.SetNumKeys(count);
            keySeq.SetLocked(locked);
            for (unsigned int i = 0; i < count; i++)
            {
                TKeyFrame* key = new TKeyFrame;
                if (!ImportKey(*key, imp))
                {
                    result = false;
                    delete key;
                    break;
                }
                keys.InsertTail(key);
            }
        }
    }
    return result;
}

bool TDImp::ImportKey(TKeyFrame& key, TDImp* imp)                      // 0x16720
{
    bool result = false;
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_KEYF], -1) == 0)
        {
            float time;
            TQuaternion rotation;
            TVector4 translation;
            imp->m_pFile->Read(&time, 4);
            imp->m_pFile->Read(&rotation, 16);
            imp->m_pFile->Read(&translation, 12);
            translation.w = 1.0f;
            key.Set(time, rotation, translation);
            result = true;
        }
        else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_KEY1], -1) == 0)
        {
            unsigned short packedTime;
            short packedRotation[4];
            short packedTranslation[3];
            imp->m_pFile->Read(&packedTime, 2);
            imp->m_pFile->Read(packedRotation, 8);
            imp->m_pFile->Read(packedTranslation, 6);
            float time = packedTime * GetSequenceLength() * (1.0f / 32768.0f);
            TQuaternion rotation(packedRotation[0] * (1.0f / 16384.0f), packedRotation[1] * (1.0f / 16384.0f),
                                 packedRotation[2] * (1.0f / 16384.0f), packedRotation[3] * (1.0f / 16384.0f));
            TVector4 translation(packedTranslation[0] * (1.0f / 256.0f), packedTranslation[1] * (1.0f / 256.0f),
                                 packedTranslation[2] * (1.0f / 256.0f), 1.0f);
            key.Set(time, rotation, translation);
            result = true;
        }
        else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_QKEY], -1) == 0)
        {
            unsigned short packedTime;
            short packedRotation[4];
            imp->m_pFile->Read(&packedTime, 2);
            imp->m_pFile->Read(packedRotation, 8);
            float time = packedTime * GetSequenceLength() * (1.0f / 32768.0f);
            TQuaternion rotation(packedRotation[0] * (1.0f / 16384.0f), packedRotation[1] * (1.0f / 16384.0f),
                                 packedRotation[2] * (1.0f / 16384.0f), packedRotation[3] * (1.0f / 16384.0f));
            TVector4 translation(m_oSeqShortTranslation.x * (1.0f / 256.0f), m_oSeqShortTranslation.y * (1.0f / 256.0f),
                                 m_oSeqShortTranslation.z * (1.0f / 256.0f), 1.0f);
            key.Set(time, rotation, translation);
            result = true;
        }
    }
    return result;
}

bool TDImp::ImportMeshSet(unsigned int& a, unsigned int& b, TDImp* imp) // 0x16a60
{
    bool result = false;
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_MSE2], -1) == 0)
        {
            result = true;
            float x, y, z;
            imp->m_pFile->Read(&x, 4);
            imp->m_pFile->Read(&y, 4);
            imp->m_pFile->Read(&z, 4);
            SetBoundingCenter(x, y, z);
            imp->m_pFile->Read(&m_fBoundingRadius, 4);
            imp->m_pFile->Read(&a, 4);
            imp->m_pFile->Read(&b, 4);
        }
        else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_MSET], -1) == 0)
        {
            result = true;
            SetBoundingCenter(0.0f, 0.0f, 0.0f);
            imp->m_pFile->Read(&m_fBoundingRadius, 4);
            imp->m_pFile->Read(&a, 4);
            imp->m_pFile->Read(&b, 4);
        }
    }
    return result;
}

void TDImp::MarkMeshSet()                                               // 0x16b70
{
    TNodeList<TBoneNameNode> moving;

    for (TNodeList<TSequence>::Iterator seq = m_oSequenceSet.Begin(); seq != m_oSequenceSet.End(); ++seq)
    {
        for (TNodeList<TKeySequence>::Iterator keySeq = seq->GetKeySeqs().Begin(); keySeq != seq->GetKeySeqs().End(); ++keySeq)
        {
            bool havePrevious = false;
            TVector4 previous = TVector4::VEC_ZERO;
            TVector4 minimum(TMath::MAXFLOAT, TMath::MAXFLOAT, TMath::MAXFLOAT, 1.0f);
            TVector4 maximum(-TMath::MAXFLOAT, -TMath::MAXFLOAT, -TMath::MAXFLOAT, 1.0f);
            for (TNodeList<TKeyFrame>::Iterator key = keySeq->GetKeys().Begin(); key != keySeq->GetKeys().End(); ++key)
            {
                TVector4& translation = key->GetTranslation();
                if (havePrevious)
                {
                    TVector4 delta = translation - previous;
                    if (translation.x < minimum.x) minimum.x = translation.x;
                    if (translation.y < minimum.y) minimum.y = translation.y;
                    if (translation.z < minimum.z) minimum.z = translation.z;
                    if (translation.x > maximum.x) maximum.x = translation.x;
                    if (translation.y > maximum.y) maximum.y = translation.y;
                    if (translation.z > maximum.z) maximum.z = translation.z;
                    TVector4 range = maximum - minimum;
                    if (range.x < 0.0f) range.x = 0.0f;
                    if (range.y < 0.0f) range.y = 0.0f;
                    if (range.z < 0.0f) range.z = 0.0f;
                    if (delta.Magnitude() > s_fDeltaTransEpsilon || range.Magnitude() > s_fDeltaTransDeviation)
                    {
                        bool found = false;
                        for (TNodeList<TBoneNameNode>::Iterator name = moving.Begin(); name != moving.End(); ++name)
                        {
                            if (name->m_oName.CompareNoCase(keySeq->GetAssociatedBoneName(), -1) == 0)
                            {
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            TBoneNameNode* node = new TBoneNameNode;
                            if (node)
                            {
                                node->m_oName = keySeq->GetAssociatedBoneName();
                                moving.InsertTail(node);
                            }
                        }
                    }
                }
                previous = translation;
                havePrevious = true;
            }
        }
    }

    for (TNodeList<TSequence>::Iterator seq2 = m_oSequenceSet.Begin(); seq2 != m_oSequenceSet.End(); ++seq2)
    {
        for (TNodeList<TKeySequence>::Iterator keySeq = seq2->GetKeySeqs().Begin(); keySeq != seq2->GetKeySeqs().End(); ++keySeq)
        {
            keySeq->TranslateAnimating(false);
            for (TNodeList<TBoneNameNode>::Iterator name = moving.Begin(); name != moving.End(); ++name)
            {
                if (name->m_oName.CompareNoCase(keySeq->GetAssociatedBoneName(), -1) == 0)
                {
                    keySeq->TranslateAnimating(true);
                    break;
                }
            }
        }
    }
}

bool TDImp::TDImpMaterial::FileRead(TDImp* imp)                         // 0x16fa0
{
    bool result = false;
    DeleteResources();
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_MATL], -1) == 0)
        {
            unsigned int type;
            imp->m_pFile->Read(&type, 4);
            if (type == 0x80000000)
            {
                m_uiFlags = 0;
                imp->m_pFile->Read(&m_uiImageFormat, 4);
                imp->m_pFile->Read(&m_uiWidth, 4);
                imp->m_pFile->Read(&m_uiHeight, 4);
                imp->m_pFile->Read(&m_uiCodebookFormat, 4);
                imp->m_pFile->Read(&m_uiCodebookSize, 4);
            }
            else if (type == 0x80000001)
            {
                TRenderInterface* renderer = TRenderInterface::GetRenderer();
                m_uiFlags = 1;
                m_uiImageFormat = 0;
                m_uiWidth = 0;
                m_uiHeight = 0;
                m_uiCodebookFormat = 0;
                m_uiCodebookSize = 0;
                imp->m_pFile->Read(m_szName, 32);
                if (renderer)
                    m_pToshiMaterial = TRenderInterface::GetRenderer()->GetMaterialLibraryManager()->GetMaterial(m_szName);
            }
            else
            {
                m_uiFlags = 0;
                m_uiImageFormat = 2;
                m_uiWidth = type;
                imp->m_pFile->Read(&m_uiHeight, 4);
                m_uiCodebookFormat = 0;
                m_uiCodebookSize = 0;
            }

            unsigned int imageBytes = 0;
            unsigned int codebookBytes = 0;
            switch (m_uiCodebookFormat)
            {
            case 0: codebookBytes = m_uiCodebookSize * 4; break;
            case 1: codebookBytes = m_uiCodebookSize * 3; break;
            case 2: codebookBytes = m_uiCodebookSize * 2; break;
            }

            switch (m_uiImageFormat)
            {
            case 0:
                m_uiCodebookBytes = codebookBytes;
                imageBytes = m_uiWidth * 4 * m_uiHeight;
                m_uiImageBytes = imageBytes;
                break;
            case 1:
                m_uiCodebookBytes = codebookBytes;
                imageBytes = m_uiWidth * 3 * m_uiHeight;
                m_uiImageBytes = imageBytes;
                break;
            case 2:
                imageBytes = m_uiWidth * 2 * m_uiHeight;
                m_uiCodebookBytes = codebookBytes;
                m_uiImageBytes = imageBytes;
                break;
            case 3:
            case 12:
                m_uiCodebookBytes = codebookBytes;
                imageBytes = m_uiWidth * m_uiHeight;
                m_uiImageBytes = imageBytes;
                break;
            case 4:
            case 13:
                m_uiCodebookBytes = codebookBytes;
                imageBytes = (m_uiWidth * m_uiHeight + 1) / 2;
                m_uiImageBytes = imageBytes;
                break;
            case 5:
                imageBytes = (m_uiHeight / 2) * (m_uiWidth / 2);
                m_uiImageBytes = imageBytes;
                m_uiCodebookBytes = codebookBytes * 4;
                break;
            default:
                m_uiCodebookBytes = codebookBytes;
                m_uiImageBytes = 0;
                break;
            }

            if (imageBytes)
            {
                m_pImage = malloc(imageBytes);
                imp->m_pFile->Read(m_pImage, m_uiImageBytes);
            }
            if (m_uiCodebookBytes)
            {
                m_pCodebook = malloc(m_uiCodebookBytes);
                imp->m_pFile->Read(m_pCodebook, m_uiCodebookBytes);
            }
            result = true;
        }
    }
    return result;
}

void TDImp::TDImpMaterial::DeleteResources()                            // 0x171f0
{
    if (m_pImage)
    {
        free(m_pImage);
        m_pImage = 0;
    }
    if (m_pCodebook)
    {
        free(m_pCodebook);
        m_pCodebook = 0;
    }
}

void TDImp::TSubMesh::GetDimensions(unsigned int& numIndices, unsigned int& numVertices, unsigned int* const bones) // 0x17230
{
    numIndices = m_uiNumIndices;
    numVertices = m_uiNumVertices;
    if (bones)
        TSystem::MemCopy(bones, m_puiBones, 16);
}

bool TDImp::TSubMesh::FileRead(TDImp* imp)                              // 0x17270
{
    bool result = false;
    DeleteResources();
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_IAUX], -1) == 0)
        {
            imp->m_pFile->Read(&m_uiNumIndices, 4);
            imp->m_pFile->Read(&m_uiNumVertices, 4);
            m_puiBones = (unsigned int*)tmalloc(16, 0, -1);
            m_uiNumBones = 4;
            imp->m_pFile->Read(m_puiBones, 16);
            m_pVertices = new SkinVertex[m_uiNumVertices];
            m_pusIndices = (unsigned short*)tmalloc(m_uiNumIndices * 2, 0, -1);
            if (m_pVertices && m_pusIndices)
            {
                imp->m_pFile->Read(m_pusIndices, m_uiNumIndices * 2);
                imp->m_pFile->Read(m_pVertices, m_uiNumVertices * sizeof(SkinVertex));
                result = true;
            }
        }
        else if (TSystem::StringCompare(header, s_BlockNames[BLOCK_IAPX], -1) == 0)
        {
            imp->m_pFile->Read(&m_uiNumIndices, 4);
            imp->m_pFile->Read(&m_uiNumVertices, 4);
            imp->m_pFile->Read(&m_uiNumBones, 4);
            m_puiBones = (unsigned int*)tmalloc(m_uiNumBones * 4, 0, -1);
            imp->m_pFile->Read(m_puiBones, m_uiNumBones * 4);
            if (m_uiNumVertices == 0)
                m_pVertices = 0;
            else
                m_pVertices = new SkinVertexP[m_uiNumVertices];
            m_pusIndices = (unsigned short*)tmalloc(m_uiNumIndices * 2, 0, -1);
            if (m_pusIndices)
            {
                imp->m_pFile->Read(m_pusIndices, m_uiNumIndices * 2);
                result = true;
            }
            if (m_pVertices)
                imp->m_pFile->Read(m_pVertices, m_uiNumVertices * sizeof(SkinVertexP));
        }

        m_uiMaxIndex = 1;
        for (unsigned int i = 0; i < m_uiNumIndices; i++)
        {
            if (m_pusIndices[i] >= m_uiMaxIndex)
                m_uiMaxIndex = m_pusIndices[i] + 1;
        }
    }
    return result;
}

void TDImp::TSubMesh::DeleteResources()                                 // 0x174d0
{
    delete[] m_pVertices;
    tfree(m_pusIndices);
    tfree(m_puiBones);
    m_pVertices = 0;
    m_pusIndices = 0;
    m_puiBones = 0;
}

bool TDImp::TSkinMesh::FileRead(TDImp* imp)                             // 0x17510
{
    bool result = false;
    DeleteResources();
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_SMSH], -1) == 0 ||
            TSystem::StringCompare(header, s_BlockNames[BLOCK_SMPX], -1) == 0)
        {
            imp->m_pFile->Read(&m_eMeshType, 4);
            imp->m_pFile->Read(&m_uiNumSubMeshes, 4);
            imp->m_pFile->Read(&m_uiDim28, 4);
            imp->m_pFile->Read(&m_uiDim2C, 4);
            imp->m_pFile->Read(&m_uiNumBones, 4);
            m_pBoneNames = (char*)tmalloc(m_uiNumBones * 15, 0, -1);
            if (m_pBoneNames)
            {
                imp->m_pFile->Read(m_pBoneNames, m_uiNumBones * 15);
                result = true;
            }
        }
    }
    return result;
}

bool TDImp::TSkinMesh::FileReadSubMeshes(TDImp* imp)                    // 0x175f0
{
    for (unsigned int i = 0; i < m_uiNumSubMeshes; i++)
    {
        TSubMesh* subMesh = new TSubMesh(imp);
        if (!subMesh)
            return false;
        m_oSubMeshes.InsertTail(subMesh);
    }
    return true;
}

void TDImp::TSkinMesh::DeleteResources()                                // 0x17690
{
    while (!m_oSubMeshes.IsEmpty())
    {
        TSubMesh* subMesh = m_oSubMeshes.RemoveHead();
        delete subMesh;
    }
    tfree(m_pBoneNames);
    m_pBoneNames = 0;
}

void TDImp::TBone::GetPose(TQuaternion& rotation, TVector3& translation) // 0x17700
{
    rotation = m_oRotation;
    translation = *(TVector3*)&m_oTranslation;
}

bool TDImp::TBone::FileRead(TDImp* imp)                                 // 0x17740
{
    bool result = false;
    if (imp)
    {
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_BONE], -1) == 0)
        {
            imp->m_pFile->Read(&m_eType, 4);
            imp->m_pFile->Read(m_szName, 15);
            imp->m_pFile->Read(m_szParentName, 15);
            imp->m_pFile->Read(&m_oRotation, 16);
            imp->m_pFile->Read(&m_oTranslation, 12);
            m_oTranslation.w = 1.0f;
            result = true;
        }
    }
    return result;
}

bool TDImp::TBone::ComputeManualAndHook(TDImp* imp, TVector4& translation, TQuaternion& rotation) // 0x177d0
{
    unsigned int numKeys = 0;
    bool manual = false;
    char lowerName[256];
    strcpy(lowerName, m_szName);
    _strlwr(lowerName);
    if (strstr(lowerName, "pathhook"))
    {
        m_bPathHook = true;
        manual = true;
    }

    TVector4 firstTranslation;
    TQuaternion firstRotation;
    if (!manual)
    {
        m_bPathHook = false;
        manual = true;
        for (TNodeList<TSequence>::Iterator seq = imp->m_oSequenceSet.Begin(); seq != imp->m_oSequenceSet.End(); ++seq)
        {
            TNodeList<TKeySequence>& keySeqs = seq->GetKeySeqs();
            for (TNodeList<TKeySequence>::Iterator keySeq = keySeqs.Begin(); keySeq != keySeqs.End(); ++keySeq)
            {
                if (_stricmp(keySeq->GetAssociatedBoneName(), m_szName) == 0)
                {
                    TNodeList<TKeyFrame>& keys = keySeq->GetKeys();
                    for (TNodeList<TKeyFrame>::Iterator key = keys.Begin(); key != keys.End(); ++key)
                    {
                        TQuaternion& keyRotation = key->GetRotation();
                        TVector4& keyTranslation = key->GetTranslation();
                        if (numKeys++ == 0)
                        {
                            firstTranslation = keyTranslation;
                            firstRotation = keyRotation;
                        }
                        else
                        {
                            TQuaternion rotationDelta;
                            rotationDelta = keyRotation - firstRotation;
                            TVector4 translationDelta;
                            translationDelta = keyTranslation - firstTranslation;
                            if (sqrt(rotationDelta.x * rotationDelta.x + rotationDelta.y * rotationDelta.y +
                                     rotationDelta.z * rotationDelta.z + rotationDelta.w * rotationDelta.w) > 0.001f)
                            {
                                m_bManual = false;
                                return false;
                            }
                            if (translationDelta.Magnitude() > 0.01f)
                            {
                                m_bManual = false;
                                return false;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    if (numKeys == 0)
    {
        firstRotation = m_oRotation;
        firstTranslation = m_oTranslation;
    }
    translation = firstTranslation;
    rotation = firstRotation;
    m_bManual = manual;
    return manual;
}

void TDImp::TBone::ReorderKeySeqs(TDImp* imp)                          // 0x17bb0
{
    if (m_bManual)
    {
        for (TNodeList<TSequence>::Iterator seq = imp->m_oSequenceSet.Begin(); seq != imp->m_oSequenceSet.End(); ++seq)
        {
            TNodeList<TKeySequence>& keySeqs = seq->GetKeySeqs();
            for (TNodeList<TKeySequence>::Iterator it = keySeqs.Begin(); it != keySeqs.End();)
            {
                TKeySequence* keySeq = it.Get();
                ++it;
                if (_stricmp(keySeq->GetAssociatedBoneName(), m_szName) == 0)
                {
                    keySeqs.Remove(keySeq);
                    keySeqs.InsertTail(keySeq);
                    break;
                }
            }
        }
    }
}

bool TDImp::TBone::IsBadBone()                                          // 0x17cb0
{
    if (m_szName[0] == '@' && m_szName[1] == '@' && m_oRotation == TQuaternion::IDENTITY &&
        m_oTranslation.IsEqual(TVector4::VEC_ZERO))
        return true;
    return false;
}

void TDImp::TBone::ComposeWithParent()                                  // 0x17d00
{
    if (m_pParent)
    {
        TQuaternion::Multiply(m_oRotationC, m_oRotation, m_pParent->m_oRotationC);
        TQuaternion::RotateVector(m_oTranslationC, m_oRotation, m_pParent->m_oTranslationC);
        m_oTranslationC.Add(m_oTranslation);
    }
    else
    {
        m_oRotationC = m_oRotation;
        m_oTranslationC = m_oTranslation;
    }
}

void TDImp::TBone::GetPoseC(TQuaternion& rotation, TVector3& translation) // 0x17d80
{
    rotation = m_oRotationC;
    translation = *(TVector3*)&m_oTranslationC;
}

bool TDImp::TSkeleton::FileRead(TDImp* imp)                             // 0x17dc0
{
    bool result = false;
    DeleteResources();
    if (imp)
    {
        imp->m_pFile->Tell();
        char header[5];
        imp->m_pFile->Read(header, 5);
        if (TSystem::StringCompare(header, s_BlockNames[BLOCK_SKEL], -1) == 0)
        {
            imp->m_pFile->Read(&m_uiNumBones, 4);
            result = true;
        }
    }
    return result;
}

bool TDImp::TSkeleton::FileReadBones(TDImp* imp)                        // 0x17e20
{
    for (unsigned int i = 0; i < m_uiNumBones; i++)
    {
        TBone* bone = new TBone(imp);
        if (!bone)
            return false;
        m_oBones.InsertTail(bone);
    }
    return true;
}

void TDImp::TSkeleton::DebugDump()                                      // 0x17eb0
{
}

void TDImp::TSkeleton::DeleteResources()                                // 0x17ec0
{
    while (!m_oBones.IsEmpty())
    {
        TBone* bone = m_oBones.RemoveHead();
        delete bone;
    }
}

void TDImp::TBone::GetPoseC(TQuaternion& rotation, TVector4& translation) // 0x17f80
{
    rotation = m_oRotationC;
    translation = m_oTranslationC;
}

bool TDImp::TBone::IsBadBoneName()                                      // 0x17fc0
{
    if (m_szName[0] == '@' && m_szName[1] == '@')
        return true;
    return false;
}

bool TDImp::TBone::IsBoneNode()                                         // 0x17fe0
{
    if (m_eType == TYPE_BONE)
        return true;
    return false;
}

bool TDImp::TBone::IsTransformNode()                                    // 0x17ff0
{
    if (m_eType == TYPE_TRANSFORM)
        return true;
    return false;
}

bool TDImp::TBone::IsRootNode()                                         // 0x18010
{
    if (m_eType == TYPE_ROOT)
        return true;
    return false;
}

void TDImp::TBone::GetPose(TQuaternion& rotation, TVector4& translation) // 0x18030
{
    rotation = m_oRotation;
    translation = m_oTranslation;
}

TDImp::TBone::~TBone()                                                  // 0x18070
{
}

const char* TDImp::TSkinMesh::GetRigidBoneName()                        // 0x18080
{
    return m_pBoneNames;
}

const char* TDImp::TSkinMesh::GetBoneNames()                            // 0x18090
{
    return m_pBoneNames;
}

void TDImp::TSkinMesh::GetDimensions(unsigned int& numSubMeshes, unsigned int& a, unsigned int& b, unsigned int& numBones) // 0x180a0
{
    numSubMeshes = m_uiNumSubMeshes;
    a = m_uiDim28;
    b = m_uiDim2C;
    numBones = m_uiNumBones;
}

void TDImp::SetSeqShortTranslation(tag_TShortVector3& translation)      // 0x18150
{
    m_oSeqShortTranslation.x = translation.x;
    m_oSeqShortTranslation.y = translation.y;
    m_oSeqShortTranslation.z = translation.z;
}

bool TDImp::IsManualRestricted(TBone* bone)                             // 0x18180
{
    if (!bone->HasChildren())
        return false;
    return true;
}

} // namespace Toshi
