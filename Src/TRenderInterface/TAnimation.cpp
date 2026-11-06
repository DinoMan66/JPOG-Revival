// TRenderInterface.dll - skeletal animation (reconstruction)
// Small members of the animation classes (0x6830-0x6a00, 0x7e80-0x8cd0 in the original). The big
// ones (TSkeleton::Create, TSkeletonInstance::UpdateState, ...) at 0x45b0-0x6830 are still to do.
#define TRENDERINTERFACE_EXPORTS
#ifndef TQLIST_FORM
#define TQLIST_FORM 3   // round 33 (+ int IsEmpty: TSkeletonInstance::UpdateTime): matches ~TSkeletonInstance / ~TKeyframeLibraryManager
#endif
#include "TNewDelete.h"
#include "TAnimation.h"
#include "TRenderInterface.h"
#include "TSystem.h"
#include "TCString.h"
#include "TFile.h"
#include "TMath.h"

namespace Toshi {

// ---------------------------------------------------------------- TSkeletonInstance (0x47c0-0x5320)
void TSkeletonInstance::RemoveAnimation(TAnimation* animation, float blendout) // 0x47c0
{
    if (!animation)
        return;
    if (blendout > 0.0f)
    {
        animation->m_fBlendOutSpeed = blendout;
        animation->m_fDestWeight = 0.0f;
        animation->m_eMode = TAnimation::MODE_BLENDOUT;
    }
    else if (animation->IsUpdateStateOnRemove())
    {
        animation->SetUpdateStateOnRemove(false);
        animation->m_fWeight = 1.0f;
        animation->m_fBlendOutSpeed = 1.0f;
        animation->m_eMode = TAnimation::MODE_BLENDOUT;
        m_iFlags |= 1;
    }
    else
    {
        if (animation->m_pInstance->m_pSkeleton->m_ppSequences[animation->m_iSequence]->IsOverlay() == true)
            m_iOverlayAnimationCount--;
        else
            m_iBaseAnimationCount--;
        animation->m_iFlags = 0;
        animation->Remove();
        m_oFreeAnimations.InsertHead(animation);
    }
}

TAnimation* TSkeletonInstance::IsAnimating(short sequence)             // 0x48a0
{
    for (TQList<TAnimation>::Iterator it = m_oBaseAnimations.Begin(); it != m_oBaseAnimations.End(); ++it)
    {
        if ((unsigned short)it->m_iSequence == (unsigned short)sequence)
            return it;
    }
    for (TQList<TAnimation>::Iterator it2 = m_oOverlayAnimations.Begin(); it2 != m_oOverlayAnimations.End(); ++it2)
    {
        if ((unsigned short)it2->m_iSequence == (unsigned short)sequence)
            return it2;
    }
    return 0;
}

void TSkeletonInstance::SetStateFromBasePose()                          // 0x4900
{
    for (int i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++)
    {
        TSkeletonBone& bone = m_pSkeleton->m_pBones[i];
        TSkeletonInstanceBone& instanceBone = m_pBones[i];
        instanceBone.m_oRotation = bone.m_oRotation;
        instanceBone.m_oPosition = bone.m_oPosition;
        instanceBone.m_oTransform.Identity();
    }
}

void TSkeletonInstance::UpdateTime(float deltaTime)                     // 0x49c0
{
    if (m_oBaseAnimations.IsEmpty() == 1 && m_oOverlayAnimations.IsEmpty() == 1)
        return;
    unsigned int frame = TRenderInterface::GetRenderer()->m_uiFrameCount;
    if (frame == m_uiLastUpdateFrame)
        return;
    m_uiLastUpdateFrame = frame;
    m_fTotalWeight = 0.0f;
    for (TQList<TAnimation>::Iterator it = m_oBaseAnimations.Begin(); it != m_oBaseAnimations.End();)
    {
        TAnimation* animation = it;
        ++it;
        if (animation->UpdateTime(deltaTime) == true)
            m_fTotalWeight += animation->m_fWeight;
    }
    for (TQList<TAnimation>::Iterator it2 = m_oOverlayAnimations.Begin(); it2 != m_oOverlayAnimations.End();)
    {
        TAnimation* animation = it2;
        ++it2;
        animation->UpdateTime(deltaTime);
    }
    if (m_iFlags & 1)
    {
        m_iFlags &= ~1;
        UpdateState();
    }
}

TMatrix44& TSkeletonInstance::GetBoneTransformCurrent(int bone, TMatrix44& out) // 0x5230
{
    if (bone < m_pSkeleton->GetAutoBoneCount())
        out.Multiply(m_pBones[bone].m_oTransform, m_pSkeleton->m_pBones[bone].m_oTransform);
    else
        out = m_pSkeleton->m_pBones[bone].m_oTransform;
    return out;
}

const TMatrix44& TSkeletonInstance::GetBoneTransformCurrent(int bone)   // 0x52f0
{
    static TMatrix44 s_oTransform;
    return GetBoneTransformCurrent(bone, s_oTransform);
}

// ---------------------------------------------------------------- TAnimation (0x5320-0x55e0)
float TAnimation::SetDestWeight(float weight, float speed)              // 0x5320
{
    float old = m_fDestWeight;
    m_fDestWeight = weight;
    m_fBlendInSpeed = speed;
    if (speed > 0.0f)
        m_eMode = MODE_BLENDIN;
    else
    {
        m_fWeight = weight;
        m_eMode = MODE_ACTIVE;
    }
    return old;
}

float TAnimation::SetTime(float time)                                   // 0x5360
{
    float old = m_fTotalTime;
    TSkeletonSequence* sequence = m_pInstance->m_pSkeleton->m_ppSequences[m_iSequence];
    m_fTotalTime = time;
    m_fSeqTime = time;
    if (sequence->m_eLoopMode == 0 && !(m_iFlags & FLAG_MANAGED))
        m_fSeqTime -= TMath::FloorToInt(time / sequence->m_fLength) * sequence->m_fLength;
    else if (time < 0.0f)
        m_fSeqTime = 0.0f;
    else if (time > sequence->m_fLength)
        m_fSeqTime = sequence->m_fLength;
    m_eMode = MODE_ACTIVE;
    return old;
}

void TAnimation::ChangeToManaged(float speed)                           // 0x5400
{
    if (IsActive() == true && !(m_iFlags & FLAG_MANAGED))
    {
        TSkeletonSequence* sequence = m_pInstance->m_pSkeleton->m_ppSequences[m_iSequence];
        m_iFlags |= FLAG_MANAGED;
        m_fBlendOutSpeed = speed;
        m_fSeqTime -= TMath::FloorToInt(m_fSeqTime / sequence->m_fLength) * sequence->m_fLength;
        m_fTotalTime = m_fSeqTime;
    }
}

bool TAnimation::SetUpdateStateOnRemove(bool update)                    // 0x5480
{
    bool old = IsUpdateStateOnRemove();
    unsigned short flags = m_iFlags & ~FLAG_UPDATESTATEONREMOVE;
    m_iFlags = flags | (update == true ? FLAG_UPDATESTATEONREMOVE : 0);
    return old;
}

bool TAnimation::UpdateTime(float deltaTime)                            // 0x54c0
{
    bool result = true;
    TSkeletonSequence* sequence = m_pInstance->m_pSkeleton->m_ppSequences[m_iSequence];
    m_fTotalTime += deltaTime * m_fSpeed;
    m_fSeqTime = m_fTotalTime;
    if (sequence->m_eLoopMode == 0 && !(m_iFlags & FLAG_MANAGED))
        m_fSeqTime -= TMath::FloorToInt(m_fTotalTime / sequence->m_fLength) * sequence->m_fLength;
    else if (m_fTotalTime < 0.0f)
        m_fSeqTime = 0.0f;
    else if (m_fTotalTime > sequence->m_fLength)
        m_fSeqTime = sequence->m_fLength;

    switch (m_eMode)
    {
    case MODE_BLENDIN:
        m_fWeight += deltaTime / m_fBlendInSpeed;
        if (m_fWeight >= m_fDestWeight)
        {
            m_fWeight = m_fDestWeight;
            m_eMode = MODE_ACTIVE;
        }
        break;
    case MODE_ACTIVE:
        if ((m_iFlags & FLAG_MANAGED) && m_fTotalTime >= sequence->m_fLength)
            m_eMode = MODE_BLENDOUT;
        break;
    case MODE_BLENDOUT:
        m_fWeight += -deltaTime / m_fBlendOutSpeed;
        if (m_fWeight <= 0.0f)
        {
            result = IsUpdateStateOnRemove();
            m_pInstance->RemoveAnimation(this, 0.0f);
        }
        break;
    }
    return result;
}

// ---------------------------------------------------------------- keyframe libraries (0x55e0-0x5930)
TKeyframeLibrary* TKeyframeLibrary::Create(const char* name, int tcount, int qcount, int scount) // 0x55e0
{
    int tsize = tcount * sizeof(TAnimVector);
    int qsize = qcount * sizeof(TAnimQuaternion);
    TKeyframeLibrary* library = new (tmalloc(tsize + qsize + scount * sizeof(float) + sizeof(TKeyframeLibrary), 0, -1)) TKeyframeLibrary;
    if (!library)
        return 0;
    library->m_oName.SetName(name);
    library->m_iReferenceCount = 0;
    library->m_iTCount = tcount;
    library->m_iQCount = qcount;
    library->m_iSCount = scount;
    // the key arrays are stored as offsets into the key memory here
    library->m_pTKeys = (TAnimVector*)(library + 1);
    library->m_pQKeys = (TAnimQuaternion*)tsize;
    library->m_pSKeys = (float*)(qsize + tsize);
    return library;
}

void TKeyframeLibrary::DebugDump(bool full)                             // 0x5690
{
}

TKeyframeLibrary::TKeyframeLibrary()                                    // 0x56a0
{
    m_iReferenceCount = 0;
    m_iTCount = 0;
    m_iQCount = 0;
    m_iSCount = 0;
    m_pTKeys = 0;
    m_pQKeys = 0;
    m_pSKeys = 0;
}

TKeyframeLibraryInstance::TKeyframeLibraryInstance()                    // 0x56c0
{
    m_pLibrary = 0;
    m_iTBaseIndex = 0;
    m_iQBaseIndex = 0;
    m_iSBaseIndex = 0;
    m_iTCount = 0;
    m_iQCount = 0;
    m_iSCount = 0;
    m_pTKeys = 0;
    m_pQKeys = 0;
    m_pSKeys = 0;
}

TKeyframeLibrary* TKeyframeLibraryInstance::Create(TKeyframeLibrary* library) // 0x56f0
{
    return CreateEx(library, library->m_iTCount, library->m_iQCount, library->m_iSCount, 0, 0, 0);
}

TKeyframeLibrary* TKeyframeLibraryInstance::CreateEx(TKeyframeLibrary* library, int tcount, int qcount, int scount,
                                                     int tbase, int qbase, int sbase) // 0x5720
{
    TKeyframeLibrary* old = m_pLibrary;
    Destroy();
    m_pLibrary = library;
    if (library)
    {
        library->m_iReferenceCount++;
        m_iTBaseIndex = tbase;
        m_iQBaseIndex = qbase;
        m_iSBaseIndex = sbase;
        m_iTCount = tcount;
        m_iQCount = qcount;
        m_iSCount = scount;
        m_pTKeys = m_pLibrary->m_pTKeys + tbase;
        m_pQKeys = m_pLibrary->m_pQKeys + qbase;
        m_pSKeys = m_pLibrary->m_pSKeys + sbase;
    }
    return old;
}

TKeyframeLibrary* TKeyframeLibraryManager::LoadLibrary(const char* name) // 0x57a0
{
    TFile* file = TFile::Create(TCString(name), 1);
    if (!file)
        return 0;

    struct { char tag[12]; } header;
    struct { char name[16]; int tcount; int qcount; int scount; } info;
    struct { int unk[3]; int dataSize; } data;
    file->Read(&header, sizeof(header));
    file->Read(&info, sizeof(info));
    file->Read(&data, sizeof(data));
    TKeyframeLibrary* library = TKeyframeLibrary::Create(info.name, info.tcount, info.qcount, info.scount);
    file->Read(library + 1, data.dataSize);
    m_oLibraries.InsertHead(library);
    TFile::Destroy(file);
    m_iCount++;
    return library;
}

TKeyframeLibrary* TKeyframeLibraryManager::GetLibrary(const char* name)  // 0x5850
{
    for (TQList<TKeyframeLibrary>::Iterator it = m_oLibraries.Begin(); it != m_oLibraries.End(); ++it)
    {
        if (it->m_oName.Compare(name) == true)
            return it;
    }
    return 0;
}

void TKeyframeLibraryManager::DebugDump(bool full)                      // 0x58e0
{
    for (TQList<TKeyframeLibrary>::Iterator it = m_oLibraries.Begin(); it != m_oLibraries.End(); ++it)
        it->DebugDump(full);
}

// ---------------------------------------------------------------- TSkeleton
TSkeleton::TSkeleton()                                                  // 0x6620
    : m_iSize(0), m_iBoneCount(0), m_iManualBoneCount(0), m_iSequenceCount(0),
      m_iAnimationMaxCount(0), m_iInstanceCount(0), m_pBones(0), m_ppSequences(0)
{
}

// Turns the file offsets stored in a loaded skeleton into pointers (base = the offset origin).
bool TSkeleton::Create(unsigned int base)                               // 0x6170
{
    m_pBones = (TSkeletonBone*)((unsigned int)m_pBones + (unsigned int)this - base);
    m_ppSequences = (TSkeletonSequence**)((unsigned int)m_ppSequences + (unsigned int)this - base);
    for (int i = 0; i < m_iSequenceCount; i++)
    {
        m_ppSequences[i] = (TSkeletonSequence*)((unsigned int)m_ppSequences[i] + (unsigned int)this - base);
        TSkeletonSequence* seq = m_ppSequences[i];
        TSkeletonSequenceBone** bones = (TSkeletonSequenceBone**)(seq + 1);
        for (int j = 0; j < m_iBoneCount; j++)
            bones[j] = (TSkeletonSequenceBone*)((unsigned int)bones[j] + (unsigned int)this - base);
    }
    return true;
}

TSkeletonInstance* TSkeleton::CreateInstance(bool setBasePose)          // 0x61f0
{
    int autoCount = GetAutoBoneCount();
    int boneSize = autoCount * sizeof(TSkeletonInstanceBone);
    int animSize = autoCount * sizeof(TAnimationBone) + sizeof(TAnimation);
    int size = m_iAnimationMaxCount * animSize + boneSize + sizeof(TSkeletonInstance);
    TSkeletonInstance* inst = new (tmalloc(size, 0, -1)) TSkeletonInstance;
    if (!inst)
        return 0;
    inst->m_iFlags = 0;
    inst->m_iSize = size;
    inst->m_pSkeleton = this;
    inst->m_iOverlayAnimationCount = 0;
    inst->m_iBaseAnimationCount = 0;
    inst->m_pBones = (TSkeletonInstanceBone*)(inst + 1);
    inst->m_pAnimations = (TAnimation*)((char*)(inst + 1) + boneSize);
    inst->m_fTotalWeight = 0.0f;
    inst->m_uiLastUpdateFrame = 0;
    inst->m_iUnk38 = 0;
    TAnimation* anim = inst->m_pAnimations;
    for (int i = 0; i < m_iAnimationMaxCount; i++)
    {
        new (anim) TAnimation;
        anim->m_iFlags = 0;
        inst->m_oFreeAnimations.InsertHead(anim);
        anim = (TAnimation*)((char*)anim + animSize);
    }
    if (setBasePose == true)
        inst->SetStateFromBasePose();
    m_iInstanceCount++;
    return inst;
}

int TSkeleton::GetBoneID(const char* name, int length)                  // 0x62e0
{
    if (length == 0)
        length = TSystem::StringLength(name);
    for (int i = 0; i < m_iBoneCount; i++)
    {
        if (m_pBones[i].m_oName.Compare(name, length) == true)
            return i;
    }
    return -1;
}

int TSkeleton::GetSequenceID(const char* name, int length)              // 0x63a0
{
    if (length == 0)
        length = TSystem::StringLength(name);
    for (int i = 0; i < m_iSequenceCount; i++)
    {
        if (m_ppSequences[i]->m_oName.Compare(name, length) == true)
            return i;
    }
    return -1;
}

void TSkeleton::DebugDump(bool full)                                    // 0x6450
{
}

void TSkeleton::Delete()                                                // 0x6830
{
    if (m_oKeyLibraryInstance.m_pLibrary)
    {
        m_oKeyLibraryInstance.m_pLibrary->m_iReferenceCount--;
        m_oKeyLibraryInstance.m_pLibrary = 0;
    }
    tfree(this);
}

// ---------------------------------------------------------------- TKeyframeLibraryManager
void TKeyframeLibraryManager::UnloadLibrary(TKeyframeLibrary* library)  // 0x6850
{
    library->Remove();
    tfree(library);
    m_iCount--;
}

TKeyframeLibraryManager::~TKeyframeLibraryManager()                     // 0x6880
{
}

TKeyframeLibraryManager::TKeyframeLibraryManager()                      // 0x68d0
{
    m_iCount = 0;
}

// ---------------------------------------------------------------- TKeyframeLibraryInstance
#ifndef TANIM_HDR_INLINE
void TKeyframeLibraryInstance::Destroy()                                // 0x68e0
{
    if (m_pLibrary)
    {
        m_pLibrary->m_iReferenceCount--;
        m_pLibrary = 0;
    }
}
#endif

TKeyframeLibraryInstance::~TKeyframeLibraryInstance()                   // 0x68f0
{
    Destroy();
}

// ---------------------------------------------------------------- TKeyframeLibrary
TKeyframeLibrary::~TKeyframeLibrary()                                   // 0x6900
{
}

void TKeyframeLibrary::Delete()                                         // 0x6920
{
    tfree(this);
}

// ---------------------------------------------------------------- TAnimation
void TAnimation::RemoveAnimation(float blendout)                        // 0x6930
{
    if (m_pInstance)
        m_pInstance->RemoveAnimation(this, blendout);
}

float TAnimation::SetWeight(float weight)                               // 0x6950
{
    float old = m_fWeight;
    m_fWeight = weight;
    m_eMode = (MODE)1;
    return old;
}

// ---------------------------------------------------------------- TSkeletonInstance
TAnimation* TSkeletonInstance::GetFreeAnimation()                       // 0x6970
{
#if TQLIST_FORM == 2
    if (m_oFreeAnimations.IsEmpty() == 1)
        return 0;
#else
    if (m_oFreeAnimations.IsEmpty() == true)
        return 0;
#endif
    return m_oFreeAnimations.Head();
}

void TSkeletonInstance::DebugRender()                                   // 0x6990
{
}

TAnimation* TSkeletonInstance::AddManagedAnimation(short sequence, float weight, float blendin, float blendout)   // 0x69a0
{
    return AddAnimationFull(sequence, weight, blendin, blendout, 2);
}

TAnimation* TSkeletonInstance::AddAnimation(short sequence, float weight, float blendin)    // 0x69d0
{
    return AddAnimationFull(sequence, weight, blendin, 0.0f, 0);
}

void TSkeletonInstance::Delete()                                        // 0x69f0
{
    m_pSkeleton->m_iInstanceCount--;
    tfree(this);
}

// ---------------------------------------------------------------- TSkeletonSequenceBone
bool TSkeletonSequenceBone::IsLocked()                                  // 0x7e90
{
    if (m_iFlags & 2)
        return true;
    return false;
}

bool TSkeletonSequenceBone::IsTranslateAnimating()                      // 0x7ea0
{
    if (m_iFlags & 1)
        return true;
    return false;
}

unsigned short TSkeletonSequenceBone::GetKeyCount()                     // 0x7eb0
{
    return m_iKeyCount;
}

TAnimationTQKey& TSkeletonSequenceBone::GetKey(unsigned short index)    // 0x7ec0
{
    return ((TAnimationTQKey*)(this + 1))[index];
}

// ---------------------------------------------------------------- TSkeletonSequence
unsigned int TSkeletonSequence::GetUsedBoneCount()                      // 0x7f10
{
    return m_iUsedBoneCount;
}

bool TSkeletonSequence::IsLooping()                                     // 0x7f20
{
    if (m_eLoopMode == 0)
        return true;
    return false;
}

TSkeletonSequence::LOOPMODE TSkeletonSequence::GetLoopMode()            // 0x7f30
{
    return m_eLoopMode;
}

float TSkeletonSequence::GetLength()                                    // 0x7f40
{
    return m_fLength;
}

// (TSkeletonSequence::IsOverlay() is inline in TAnimation.h)

TSkeletonSequenceBone* TSkeletonSequence::GetBone(int index)            // 0x7f70
{
    return ((TSkeletonSequenceBone**)(this + 1))[index];
}

TSkeletonSequenceBone** TSkeletonSequence::GetBones()                   // 0x7f80
{
    return (TSkeletonSequenceBone**)(this + 1);
}

unsigned short TSkeletonSequence::GetFlags()                            // 0x7f90
{
    return m_iFlags;
}

ResourceName& TSkeletonSequence::GetResourceName()                      // 0x7fa0
{
    return m_oName;
}

// ---------------------------------------------------------------- TSkeleton accessors
short TSkeleton::GetInstanceCount()                                     // 0x8030
{
    return m_iInstanceCount;
}

short TSkeleton::GetAnimationMaxCount()                                 // 0x8040
{
    return m_iAnimationMaxCount;
}

short TSkeleton::GetSequenceCount()                                     // 0x8050
{
    return m_iSequenceCount;
}

short TSkeleton::GetManualBoneCount()                                   // 0x8060
{
    return m_iManualBoneCount;
}

// (TSkeleton::GetAutoBoneCount() is inline in TAnimation.h)

short TSkeleton::GetBoneCount()                                         // 0x8080
{
    return m_iBoneCount;
}

int TSkeleton::GetSize()                                                // 0x8090
{
    return m_iSize;
}

#ifndef TANIM_HDR_INLINE
TSkeletonSequence* TSkeleton::GetSequence(int index)                    // 0x80a0
{
    return m_ppSequences[index];
}
#endif

TSkeletonSequence* TSkeleton::GetSequence(const char* name, int length) // 0x80b0
{
    return GetSequence(GetSequenceID(name, length));
}

TSkeletonSequence** TSkeleton::GetSequences()                           // 0x80d0
{
    return m_ppSequences;
}

TSkeletonBone* TSkeleton::GetBone(int index)                            // 0x80e0
{
    return &m_pBones[index];
}

TSkeletonBone* TSkeleton::GetBone(const char* name, int length)         // 0x80f0
{
    return GetBone(GetBoneID(name, length));
}

TSkeletonBone* TSkeleton::GetBones()                                    // 0x8110
{
    return m_pBones;
}

TKeyframeLibraryInstance& TSkeleton::GetKeyLibraryInstance()            // 0x8120
{
    return m_oKeyLibraryInstance;
}

// ---------------------------------------------------------------- TSkeletonBone
TSkeletonBone::TSkeletonBone()                                          // 0x82c0
{
}

bool TSkeletonBone::IsManual()                                          // 0x82d0
{
    if (m_iFlags & 1)
        return true;
    return false;
}

bool TSkeletonBone::IsRoot()                                            // 0x82f0
{
    if ((unsigned short)m_iParent == 0xFFFF)
        return true;
    return false;
}

unsigned short TSkeletonBone::GetFlags()                                // 0x8310
{
    return m_iFlags;
}

short TSkeletonBone::GetParent()                                        // 0x8320
{
    return m_iParent;
}

ResourceName& TSkeletonBone::GetResourceName()                          // 0x8330
{
    return m_oName;
}

// ---------------------------------------------------------------- TKeyframeLibraryManager accessors
int TKeyframeLibraryManager::GetLibraryCount()                          // 0x8340
{
    return m_iCount;
}

TQList<TKeyframeLibrary>& TKeyframeLibraryManager::GetLibraryList()     // 0x8350
{
    return m_oLibraries;
}

// ---------------------------------------------------------------- TKeyframeLibraryInstance accessors
void TKeyframeLibraryInstance::SetSCount(int count)        { m_iSCount = count; }        // 0x83b0
void TKeyframeLibraryInstance::SetQCount(int count)        { m_iQCount = count; }        // 0x83c0
void TKeyframeLibraryInstance::SetTCount(int count)        { m_iTCount = count; }        // 0x83d0
void TKeyframeLibraryInstance::SetSBaseIndex(int index)    { m_iSBaseIndex = index; }    // 0x83e0
void TKeyframeLibraryInstance::SetQBaseIndex(int index)    { m_iQBaseIndex = index; }    // 0x83f0
void TKeyframeLibraryInstance::SetTBaseIndex(int index)    { m_iTBaseIndex = index; }    // 0x8400

float TKeyframeLibraryInstance::GetS(int index)                         // 0x8410
{
    return m_pSKeys[index];
}

TAnimQuaternion* TKeyframeLibraryInstance::GetQ(int index)              // 0x8420
{
    return &m_pQKeys[index];
}

TAnimVector* TKeyframeLibraryInstance::GetT(int index)                  // 0x8430
{
    return &m_pTKeys[index];
}

int TKeyframeLibraryInstance::GetSCount()       { return m_iSCount; }       // 0x8450
int TKeyframeLibraryInstance::GetQCount()       { return m_iQCount; }       // 0x8460
int TKeyframeLibraryInstance::GetTCount()       { return m_iTCount; }       // 0x8470
int TKeyframeLibraryInstance::GetSBaseIndex()   { return m_iSBaseIndex; }   // 0x8480
int TKeyframeLibraryInstance::GetQBaseIndex()   { return m_iQBaseIndex; }   // 0x8490
int TKeyframeLibraryInstance::GetTBaseIndex()   { return m_iTBaseIndex; }   // 0x84a0
TKeyframeLibrary* TKeyframeLibraryInstance::GetLibrary() { return m_pLibrary; }  // 0x84b0

// ---------------------------------------------------------------- TKeyframeLibrary accessors
int TKeyframeLibrary::GetSCount()               { return m_iSCount; }           // 0x8530
int TKeyframeLibrary::GetQCount()               { return m_iQCount; }           // 0x8540
int TKeyframeLibrary::GetTCount()               { return m_iTCount; }           // 0x8550
int TKeyframeLibrary::GetReferenceCount()       { return m_iReferenceCount; }   // 0x8560
float* TKeyframeLibrary::GetSKeys()             { return m_pSKeys; }            // 0x8570
TAnimQuaternion* TKeyframeLibrary::GetQKeys()   { return m_pQKeys; }            // 0x8580
TAnimVector* TKeyframeLibrary::GetTKeys()       { return m_pTKeys; }            // 0x8590

void* TKeyframeLibrary::GetKeyMemoryBase()                              // 0x85a0
{
    return this + 1;
}

float TKeyframeLibrary::GetS(int index)                                 // 0x85b0
{
    return m_pSKeys[index];
}

TAnimQuaternion* TKeyframeLibrary::GetQ(int index)                      // 0x85c0
{
    return &m_pQKeys[index];
}

TAnimVector* TKeyframeLibrary::GetT(int index)                          // 0x85d0
{
    return &m_pTKeys[index];
}

ResourceName& TKeyframeLibrary::GetResourceName()                       // 0x85f0
{
    return m_oName;
}

// ---------------------------------------------------------------- TAnimation accessors
TAnimation::~TAnimation()                                               // 0x8660
{
}

TAnimation::TAnimation()                                                // 0x8680
{
}

float TAnimation::GetSpeed()            { return m_fSpeed; }            // 0x8690
float TAnimation::GetSeqTime()          { return m_fSeqTime; }          // 0x86a0
float TAnimation::GetTotalTime()        { return m_fTotalTime; }        // 0x86b0
float TAnimation::GetBlendOutSpeed()    { return m_fBlendOutSpeed; }    // 0x86c0
float TAnimation::GetBlendInSpeed()     { return m_fBlendInSpeed; }     // 0x86d0
float TAnimation::GetDestWeight()       { return m_fDestWeight; }       // 0x86e0
float TAnimation::GetWeight()           { return m_fWeight; }           // 0x86f0
TAnimation::MODE TAnimation::GetMode()  { return m_eMode; }             // 0x8700

TAnimationBone* TAnimation::GetBone(int index)                          // 0x8710
{
    return &((TAnimationBone*)(this + 1))[index];
}

TAnimationBone* TAnimation::GetBones()                                  // 0x8720
{
    return (TAnimationBone*)(this + 1);
}

unsigned short TAnimation::GetFlags()                                   // 0x8730
{
    return m_iFlags;
}

TSkeletonSequence* TAnimation::GetSequencePtr()                         // 0x8740
{
    return m_pInstance->m_pSkeleton->GetSequence(m_iSequence);
}

short TAnimation::GetSequence()                                         // 0x8760
{
    return m_iSequence;
}

float TAnimation::SetSpeed(float speed)                                 // 0x8770
{
    float old = m_fSpeed;
    m_fSpeed = speed;
    return old;
}

void TAnimation::SetMode(MODE mode)                                     // 0x8780
{
    m_eMode = mode;
}

// (IsUpdateStateOnRemove / IsManaged / IsActive at 0x8790-0x87d0 are inline in TAnimation.h)

// ---------------------------------------------------------------- TSkeletonInstance
TSkeletonInstance::~TSkeletonInstance()                                 // 0x8800
{
}

TSkeletonInstance::TSkeletonInstance()                                  // 0x88d0
{
}

TSkeletonInstanceBone* TSkeletonInstance::GetBone(int index)            // 0x88f0
{
    return &m_pBones[index];
}

TSkeletonInstanceBone* TSkeletonInstance::GetBones()                    // 0x8900
{
    return m_pBones;
}

unsigned int TSkeletonInstance::GetSize() const                         // 0x8910
{
    return m_iSize;
}

TSkeleton* TSkeletonInstance::GetSkeleton()                             // 0x8920
{
    return m_pSkeleton;
}

int TSkeletonInstance::GetOverlayAnimationCount()                       // 0x8930
{
    return m_iOverlayAnimationCount;
}

int TSkeletonInstance::GetBaseAnimationCount()                          // 0x8940
{
    return m_iBaseAnimationCount;
}

int TSkeletonInstance::GetActiveAnimationCount()                        // 0x8950
{
    return m_iBaseAnimationCount + m_iOverlayAnimationCount;
}

TSkeletonInstanceBone::TSkeletonInstanceBone()                          // 0x8a20
{
}

// ---------------------------------------------------------------- key types
TAnimVector::TAnimVector()                                              // 0x8a70
{
}

void TAnimVector::Set(const TVector4& v)                                // 0x8a80
{
    x = v.x;
    y = v.y;
    z = v.z;
}

void TAnimVector::Get(TVector4& v)                                      // 0x8aa0
{
    v.x = x;
    v.y = y;
    v.z = z;
    v.w = 1.0f;
}

TVector4& TAnimVector::Get()                                            // 0x8ac0
{
    return *(TVector4*)this;
}

TAnimQuaternion::TAnimQuaternion()                                      // 0x8b10
{
}

void TAnimQuaternion::Set(const TQuaternion& q)                         // 0x8b20
{
    x = q.x;
    y = q.y;
    z = q.z;
    w = q.w;
}

void TAnimQuaternion::Get(TQuaternion& q)                               // 0x8b40
{
    q.x = x;
    q.y = y;
    q.z = z;
    q.w = w;
}

TQuaternion& TAnimQuaternion::Get()                                     // 0x8b60
{
    return *(TQuaternion*)this;
}

// ---------------------------------------------------------------- ResourceName
bool ResourceName::Compare(ResourceName& other)                         // 0x8b90
{
    if (m_iLength == other.m_iLength)
    {
        if (TSystem::StringCompareNoCase(m_szName, other.m_szName, other.m_iLength) == 0)
            return true;
        return false;
    }
    return false;
}

// (Compare(const char*, int) at 0x8bd0 is inline in TAnimation.h)

int ResourceName::GetNameLength()                                       // 0x8c70
{
    return m_iLength;
}

const char* ResourceName::GetName()                                     // 0x8c80
{
    return m_szName;
}

// (Compare(const char*) at 0x8c20 and SetName(const char*) at 0x8c90 are inline in TAnimation.h)

void ResourceName::SetName(const char* name, int length)                // 0x8cc0
{
    m_iLength = (unsigned char)length;
    TSystem::StringCopy(m_szName, name, -1);
}

} // namespace Toshi
