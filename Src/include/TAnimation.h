// Skeletal animation classes of TRenderInterface.dll (TSkeleton, TSkeletonInstance, TAnimation,
// keyframe libraries). Plain structs (no vtables); every member function is a real function in a
// .cpp (the original does not inline any of them, even within one file). Copy assignments and
// copy constructors are the compiler's (memberwise) - they are exported because the classes are.
// Layouts from the accessors and constructors; names partly from OpenToshi (Toshi 2.0).
#ifndef TANIMATION_H
#define TANIMATION_H
#ifndef TANIM_PACK
#define TANIM_PACK 2   // round 34: a union with a 32-bit view makes the implicit operator= copy one dword
#endif
#include "TRenderAPI.h"
#include "TVector.h"
#include "TQList.h"
#include "TSystem.h"

namespace Toshi {

class TDImp;
class TSkeleton;
class TSkeletonInstance;
class TSkeletonSequence;
class TKeyframeLibrary;

// Pascal-style short name: length byte + characters (16 bytes).
class TRENDER_API ResourceName
{
public:
    bool Compare(ResourceName& other);
    bool Compare(const char* name, int length)                          // inline in the original (GetBoneID)
    {
        if (m_iLength == (unsigned char)length)
        {
            if (TSystem::StringCompareNoCase(m_szName, name, length) == 0)
                return true;
            return false;
        }
        return false;
    }
    bool Compare(const char* name)                                      // inline in the original (GetLibrary)
    {
        unsigned char length = (unsigned char)TSystem::StringLength(name);
        if (m_iLength == length)
        {
            if (TSystem::StringCompareNoCase(m_szName, name, length) == 0)
                return true;
            return false;
        }
        return false;
    }
    int GetNameLength();
    const char* GetName();
    void SetName(const char* name)                                      // inline in the original (TKeyframeLibrary::Create)
    {
        m_iLength = (unsigned char)TSystem::StringLength(name);
        TSystem::StringCopy(m_szName, name, -1);
    }
    void SetName(const char* name, int length);

    unsigned char m_iLength;        // +0
    char          m_szName[15];     // +1
};

struct TRENDER_API TAnimVector                  // 12 bytes (translation key)
{
    TAnimVector();
    void Set(const TVector4& v);
    void Get(TVector4& v);
    TVector4& Get();

    float x, y, z;
};

struct TRENDER_API TAnimQuaternion              // 16 bytes (rotation key)
{
    TAnimQuaternion();
    void Set(const TQuaternion& q);
    void Get(TQuaternion& q);
    TQuaternion& Get();

    float x, y, z, w;
};

struct TAnimationTQKey                          // 8 bytes
{
    unsigned short m_iTime;
    unsigned short m_iQIndex;
    unsigned short m_iTIndex;
    unsigned short m_iSIndex;
};

class TRENDER_API TKeyframeLibrary : public TQList<TKeyframeLibrary>::TNode
{
public:
    static TKeyframeLibrary* __stdcall Create(const char* name, int tcount, int qcount, int scount);
    void DebugDump(bool full);
protected:
    TKeyframeLibrary();
    ~TKeyframeLibrary();
public:
    void Delete();
    int GetSCount();
    int GetQCount();
    int GetTCount();
    int GetReferenceCount();
    float* GetSKeys();
    TAnimQuaternion* GetQKeys();
    TAnimVector* GetTKeys();
    void* GetKeyMemoryBase();
    float GetS(int index);
    TAnimQuaternion* GetQ(int index);
    TAnimVector* GetT(int index);
    ResourceName& GetResourceName();

    ResourceName     m_oName;           // +0x08
    int              m_iReferenceCount; // +0x18
    int              m_iTCount;         // +0x1C
    int              m_iQCount;         // +0x20
    int              m_iSCount;         // +0x24
    int              m_iUnk28;          // +0x28
    int              m_iUnk2C;          // +0x2C
    int              m_iUnk30;          // +0x30
    TAnimVector*     m_pTKeys;          // +0x34
    TAnimQuaternion* m_pQKeys;          // +0x38
    float*           m_pSKeys;          // +0x3C
};                                      // key memory follows (+0x40)

class TRENDER_API TKeyframeLibraryInstance
{
public:
    TKeyframeLibraryInstance();
    TKeyframeLibrary* Create(TKeyframeLibrary* library);
    TKeyframeLibrary* CreateEx(TKeyframeLibrary* library, int tcount, int qcount, int scount, int tbase, int qbase, int sbase);
#ifdef TANIM_HDR_INLINE
    void Destroy()
    {
        if (m_pLibrary)
        {
            m_pLibrary->m_iReferenceCount--;
            m_pLibrary = 0;
        }
    }
#else
    void Destroy();
#endif
    ~TKeyframeLibraryInstance();
    void SetSCount(int count);
    void SetQCount(int count);
    void SetTCount(int count);
    void SetSBaseIndex(int index);
    void SetQBaseIndex(int index);
    void SetTBaseIndex(int index);
    float GetS(int index);
    TAnimQuaternion* GetQ(int index);
    TAnimVector* GetT(int index);
    int GetSCount();
    int GetQCount();
    int GetTCount();
    int GetSBaseIndex();
    int GetQBaseIndex();
    int GetTBaseIndex();
    TKeyframeLibrary* GetLibrary();

    TKeyframeLibrary* m_pLibrary;       // +0x00
    int               m_iTBaseIndex;    // +0x04
    int               m_iQBaseIndex;    // +0x08
    int               m_iSBaseIndex;    // +0x0C
    int               m_iTCount;        // +0x10
    int               m_iQCount;        // +0x14
    int               m_iSCount;        // +0x18
    TAnimVector*      m_pTKeys;         // +0x1C
    TAnimQuaternion*  m_pQKeys;         // +0x20
    float*            m_pSKeys;         // +0x24
};

class TRENDER_API TKeyframeLibraryManager
{
public:
    TKeyframeLibrary* LoadLibrary(const char* name);
    TKeyframeLibrary* GetLibrary(const char* name);
    void DebugDump(bool full);
    void UnloadLibrary(TKeyframeLibrary* library);
    ~TKeyframeLibraryManager();
    TKeyframeLibraryManager();
    int GetLibraryCount();
    TQList<TKeyframeLibrary>& GetLibraryList();

    TQList<TKeyframeLibrary> m_oLibraries;  // +0x00
    int                      m_iCount;      // +0x08
};

class TRENDER_API TSkeletonBone                 // 0xB0
{
public:
    TSkeletonBone();
    bool IsManual();
    bool IsRoot();
    unsigned short GetFlags();
    short GetParent();
    ResourceName& GetResourceName();

    TQuaternion    m_oRotation;         // +0x00
    TMatrix44      m_oTransform;        // +0x10
    TMatrix44      m_oTransformInv;     // +0x50
    ResourceName   m_oName;             // +0x90
    short          m_iParent;           // +0xA0
    unsigned short m_iFlags;            // +0xA2
    TVector3       m_oPosition;         // +0xA4
};

class TRENDER_API TSkeletonSequenceBone         // 4 bytes + keys
{
public:
    float GetKeyPair(float time, unsigned short& key, unsigned int& from, unsigned int& to);
    void IntraBlend(TKeyframeLibraryInstance* library, float time, unsigned int& from, unsigned int& to, TQuaternion& q, TVector4& t);
    bool IsLocked();
    bool IsTranslateAnimating();
    unsigned short GetKeyCount();
    TAnimationTQKey& GetKey(unsigned short index);

#if TANIM_PACK == 1
    struct { unsigned short m_iFlags; unsigned short m_iKeyCount; };            // +0, +2
#elif TANIM_PACK == 2
    union { unsigned int m_uiPacked; struct { unsigned short m_iFlags; unsigned short m_iKeyCount; }; };
#else
    unsigned short m_iFlags;            // +0
    unsigned short m_iKeyCount;         // +2
#endif
};                                      // TAnimationTQKey keys[] follow (+4)

class TRENDER_API TSkeletonSequence             // 0x20 + bone pointers
{
public:
    enum LOOPMODE { };
    unsigned int GetUsedBoneCount();
    bool IsLooping();
    LOOPMODE GetLoopMode();
    float GetLength();
    bool IsOverlay()                                                    // inline in the original
    {
        if (m_iFlags & 1)
            return true;
        return false;
    }
    TSkeletonSequenceBone* GetBone(int index);
    TSkeletonSequenceBone** GetBones();
    unsigned short GetFlags();
    ResourceName& GetResourceName();

    ResourceName   m_oName;             // +0x00
    unsigned short m_iFlags;            // +0x10
    LOOPMODE       m_eLoopMode;         // +0x14
    unsigned int   m_iUsedBoneCount;    // +0x18
    float          m_fLength;           // +0x1C
};                                      // TSkeletonSequenceBone* bones[] follow (+0x20)

class TRENDER_API TSkeleton                     // 0x40
{
public:
    static TSkeleton* __stdcall Create(TDImp* dimp, unsigned int flags);
    bool Create(unsigned int flags);
    TSkeletonInstance* CreateInstance(bool flag);
    int GetBoneID(const char* name, int length);
    int GetSequenceID(const char* name, int length);
    void DebugDump(bool full);
    unsigned short GetBoneTransformLocal(int bone, int sequence, float time, unsigned short key, TMatrix44& out);
    void GetBoneTransform(int bone, int sequence, float time, TMatrix44& out);
protected:
    TSkeleton();
public:
    void Delete();
    short GetInstanceCount();
    short GetAnimationMaxCount();
    short GetSequenceCount();
    short GetManualBoneCount();
    short GetAutoBoneCount() { return m_iBoneCount - m_iManualBoneCount; } // inline in the original
    short GetBoneCount();
    int GetSize();
#ifdef TANIM_HDR_INLINE
    TSkeletonSequence* GetSequence(int index) { return m_ppSequences[index]; }
#else
    TSkeletonSequence* GetSequence(int index);
#endif
    TSkeletonSequence* GetSequence(const char* name, int length);
    TSkeletonSequence** GetSequences();
    TSkeletonBone* GetBone(int index);
    TSkeletonBone* GetBone(const char* name, int length);
    TSkeletonBone* GetBones();
    TKeyframeLibraryInstance& GetKeyLibraryInstance();

    int                      m_iSize;               // +0x00
    short                    m_iBoneCount;          // +0x04
    short                    m_iManualBoneCount;    // +0x06
    short                    m_iSequenceCount;      // +0x08
    short                    m_iAnimationMaxCount;  // +0x0A
    short                    m_iInstanceCount;      // +0x0C
    TKeyframeLibraryInstance m_oKeyLibraryInstance; // +0x10
    TSkeletonBone*           m_pBones;              // +0x38
    TSkeletonSequence**      m_ppSequences;         // +0x3C
};

class TRENDER_API TAnimationBone                // 2 bytes
{
public:
    unsigned short m_iKey;
};

class TRENDER_API TAnimation : public TQList<TAnimation>::TNode  // 0x34 + bones
{
public:
    enum MODE { MODE_BLENDIN, MODE_ACTIVE, MODE_BLENDOUT };
    enum FLAGS { FLAG_ACTIVE = 1, FLAG_MANAGED = 2, FLAG_UPDATESTATEONREMOVE = 8 };
    float SetDestWeight(float weight, float speed);
    float SetTime(float time);
    void ChangeToManaged(float speed);
    bool SetUpdateStateOnRemove(bool update);
    bool UpdateTime(float dt);
    void RemoveAnimation(float speed);
    float SetWeight(float weight);
    ~TAnimation();
    TAnimation();
    float GetSpeed();
    float GetSeqTime();
    float GetTotalTime();
    float GetBlendOutSpeed();
    float GetBlendInSpeed();
    float GetDestWeight();
    float GetWeight();
    MODE GetMode();
    TAnimationBone* GetBone(int index);
    TAnimationBone* GetBones();
    unsigned short GetFlags();
    TSkeletonSequence* GetSequencePtr();
    short GetSequence();
    float SetSpeed(float speed);
    void SetMode(MODE mode);
    bool IsUpdateStateOnRemove()
    {
        if (m_iFlags & FLAG_UPDATESTATEONREMOVE)
            return true;
        return false;
    }
    bool IsManaged()
    {
        if (m_iFlags & FLAG_MANAGED)
            return true;
        return false;
    }
    bool IsActive()
    {
        if (m_iFlags & FLAG_ACTIVE)
            return true;
        return false;
    }

    TSkeletonInstance* m_pInstance;     // +0x08
#if TANIM_PACK == 1
    struct { short m_iSequence; unsigned short m_iFlags; };                     // +0x0C, +0x0E
#elif TANIM_PACK == 2
    union { unsigned int m_uiPacked; struct { short m_iSequence; unsigned short m_iFlags; }; };
#else
    short              m_iSequence;     // +0x0C
    unsigned short     m_iFlags;        // +0x0E
#endif
    int                m_iUnk10;        // +0x10
    MODE               m_eMode;         // +0x14
    float              m_fSpeed;        // +0x18
    float              m_fWeight;       // +0x1C
    float              m_fDestWeight;   // +0x20
    float              m_fTotalTime;    // +0x24
    float              m_fSeqTime;      // +0x28
    float              m_fBlendInSpeed; // +0x2C
    float              m_fBlendOutSpeed;// +0x30
};                                      // TAnimationBone bones[] follow (+0x34)

class TRENDER_API TSkeletonInstanceBone         // 0x5C
{
public:
    TSkeletonInstanceBone();

    TQuaternion m_oRotation;            // +0x00
    TVector3    m_oPosition;            // +0x10
    TMatrix44   m_oTransform;           // +0x1C
};

class TRENDER_API TSkeletonInstance
{
public:
    TAnimation* AddAnimationFull(short sequence, float weight, float blendin, float blendout, unsigned int flags);
    void RemoveAnimation(TAnimation* animation, float blendout);
    TAnimation* IsAnimating(short sequence);
    void SetStateFromBasePose();
    void UpdateTime(float dt);
    void UpdateState();
    TMatrix44& GetBoneTransformCurrent(int bone, TMatrix44& out);
    const TMatrix44& GetBoneTransformCurrent(int bone);
protected:
    TAnimation* GetFreeAnimation();
public:
    void DebugRender();
    TAnimation* AddManagedAnimation(short sequence, float weight, float blendin, float blendout);
    TAnimation* AddAnimation(short sequence, float weight, float blendin);
    void Delete();
    ~TSkeletonInstance();
    TSkeletonInstance();
    TSkeletonInstanceBone* GetBone(int index);
    TSkeletonInstanceBone* GetBones();
    unsigned int GetSize() const;
    TSkeleton* GetSkeleton();
    int GetOverlayAnimationCount();
    int GetBaseAnimationCount();
    int GetActiveAnimationCount();

    unsigned int           m_iFlags;                // +0x00
    unsigned int           m_iSize;                 // +0x04
    TSkeleton*             m_pSkeleton;             // +0x08
    unsigned short         m_iBaseAnimationCount;   // +0x0C
    unsigned short         m_iOverlayAnimationCount;// +0x0E
    TQList<TAnimation>     m_oBaseAnimations;       // +0x10
    TQList<TAnimation>     m_oOverlayAnimations;    // +0x18
    TQList<TAnimation>     m_oFreeAnimations;       // +0x20
    TSkeletonInstanceBone* m_pBones;                // +0x28
    TAnimation*            m_pAnimations;           // +0x2C (pool of m_iAnimationMaxCount animations)
    float                  m_fTotalWeight;          // +0x30 (sum of the base animations' weights)
    unsigned int           m_uiLastUpdateFrame;     // +0x34 (TRenderInterface frame count)
    int                    m_iUnk38;                // +0x38
};

} // namespace Toshi
#endif
