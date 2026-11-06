// Toshi::TDBase - the animation/skin data base types read by TDImp (TRenderInterface.dll,
// TDBase.cpp, 0x133c0-0x13f90). Key frames, key sequences (one per bone) and sequences.
#ifndef TDBASE_H
#define TDBASE_H
#include "TRenderAPI.h"
#include "TNodeList.h"
#include "TVector.h"
#include "TSystem.h"

// TDImp.cpp was built without the DLL's export define (its TRenderInterface references go through
// __imp_ pointers) while TDBase/TDImp themselves are still exported: they carry their own macro.
#ifndef TDIMP_API
#define TDIMP_API __declspec(dllexport)
#endif

namespace Toshi {

class TFile;

class TDIMP_API TDBase
{
public:
    enum TMeshType { };
    enum BLOCK
    {
        BLOCK_SMSH, BLOCK_BONE, BLOCK_KEYF, BLOCK_SKEY, BLOCK_ASEQ, BLOCK_SSET, BLOCK_MATL, BLOCK_IAUX,
        BLOCK_SKEL, BLOCK_MSET, BLOCK_SMPX, BLOCK_IAPX, BLOCK_ASE1, BLOCK_SKE1, BLOCK_KEY1, BLOCK_QKEY,
        BLOCK_ASE2, BLOCK_SKE2, BLOCK_PKEY, BLOCK_PQKE, BLOCK_MSE2, BLOCK_PRMS, BLOCK_NUMOF
    };

    struct SkinVertex                                                   // 0x30
    {
        SkinVertex() { TSystem::MemSet(this, 0, sizeof(SkinVertex)); }
        float m_afData[12];
    };

    class TDIMP_API TKeyFrame : public TNodeList<TKeyFrame>::TNode    // 0x30
    {
    public:
        TKeyFrame();
        ~TKeyFrame();
        TKeyFrame& operator=(const TKeyFrame& other);

        void Get(float& time, TQuaternion& rotation, TVector4& translation);
        void Set(const float& time, const TQuaternion& rotation, const TVector4& translation);
        static void __stdcall GetInterpolate(TKeyFrame& out, TKeyFrame& a, TKeyFrame& b, float time);
        bool IsInExclusiveTimeRange(float start, float end);
        bool IsInTimeRange(float start, float end);

        TVector4& GetTranslation() { return m_oTranslation; }
        TQuaternion& GetRotation() { return m_oRotation; }
        void SetTime(float time) { m_fTime = time; }
        float GetTime() const { return m_fTime; }

        float       m_fTime;            // +0x0c
        TQuaternion m_oRotation;        // +0x10
        TVector4    m_oTranslation;     // +0x20
    };

    class TDIMP_API TKeySequence : public TNodeList<TKeySequence>::TNode // 0x40
    {
    public:
        TKeySequence();
        ~TKeySequence() { DeleteData(); }
        TKeySequence& operator=(TKeySequence& other);

        void GenerateKey(TKeyFrame& key, float time);
        void GenerateCloseKey(TKeyFrame& key, float time);
        void GenerateExportCloseKey(TKeyFrame& key, float time);

        TNodeList<TKeyFrame>& GetKeys();
        void SetAssociatedBoneName(const char* name);
        void SetNumKeys(unsigned int count);
        const char* GetAssociatedBoneName();
        void GetNumKeys(unsigned int& count);

        void TranslateAnimating(bool animating) { m_bTranslateAnimating = animating; }
        bool IsTranslateAnimating() const { return m_bTranslateAnimating; }
        unsigned int GetBoneDepth() const { return m_uiBoneDepth; }
        void SetBoneDepth(unsigned int depth) { m_uiBoneDepth = depth; }
        bool IsLocked() const { return m_bLocked; }
        void SetLocked(bool locked) { m_bLocked = locked; }
        unsigned int GetNumKeys() { return m_uiNumKeys; }

    private:
        void DeleteData();

    public:
        char                 m_szBoneName[16];      // +0x0c
        unsigned int         m_uiNumKeys;           // +0x1c
        bool                 m_bLocked;             // +0x20
        TNodeList<TKeyFrame> m_oKeys;               // +0x24
        unsigned int         m_uiBoneDepth;         // +0x34
        float                m_fFrameTime;          // +0x38 (1/60 s)
        bool                 m_bTranslateAnimating; // +0x3c
    };

    class TDIMP_API TSequence : public TNodeList<TSequence>::TNode     // 0x34
    {
    public:
        enum FLAGS { FLAG_ONCE = 1 };

        TSequence();
        ~TSequence();

        void SetFlag(unsigned short flags);
        TNodeList<TKeySequence>& GetKeySeqs();
        void SetSeqName(const char* name);
        void SetNumKeySeqs(unsigned int count);
        const char* GetSeqName();
        void GetNumKeySeqs(unsigned int& count);

        bool IsLooping()
        {
            if (m_usFlags & FLAG_ONCE)
                return false;
            return true;
        }

    private:
        void DeleteData();

    public:
        unsigned short          m_usFlags;          // +0x0c
        unsigned int            m_uiNumKeySeqs;     // +0x10
        char                    m_szName[16];       // +0x14
        TNodeList<TKeySequence> m_oKeySeqs;         // +0x24
    };

    TDBase();

    void SetTransDeviation(float deviation) { s_fDeltaTransDeviation = deviation; }
    float GetTransDeviation() { return s_fDeltaTransDeviation; }
    void SetTransEpsilon(float epsilon) { s_fDeltaTransEpsilon = epsilon; }
    float GetTransEpsilon() { return s_fDeltaTransEpsilon; }

    static char s_BlockNames[][5];          // 0x1002c860
    static float s_fDeltaTransEpsilon;      // 0x1002c8d0
    static float s_fDeltaTransDeviation;    // 0x1002c8d4

    TFile* m_pFile;                         // +0x00 (the file being imported)
};

} // namespace Toshi
#endif
