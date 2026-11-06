// Toshi::TDImp - imports a skinned model (materials, skin meshes, skeleton, animation sequences)
// from a TDB file (TRenderInterface.dll, TDImp.cpp, 0x13f90-0x18690; its inline accessors are
// emitted in TAnimation.cpp at 0x8ce0-0x9380).
#ifndef TDIMP_H
#define TDIMP_H
#include "TDBase.h"

namespace Toshi {

class TMaterial;
class TAABox;

struct tag_TShortVector3
{
    short x, y, z;
};

class TDIMP_API TDImp : public TDBase
{
public:
    class TDIMP_API TDImpMaterial : public TNodeList<TDImpMaterial>::TNode // 0x58
    {
    public:
        TDImpMaterial() { m_pImage = 0; m_pCodebook = 0; }
        TDImpMaterial(TDImp* imp) { m_pImage = 0; m_pCodebook = 0; FileRead(imp); }
        ~TDImpMaterial() { DeleteResources(); }

        bool FileRead(TDImp* imp);

        const char* GetName() { return m_szName; }
        unsigned int GetFlags() const { return m_uiFlags; }
        TMaterial* GetToshiMaterial() const { return m_pToshiMaterial; }
        void* GetCodebook() const { return m_pCodebook; }
        void* GetImage() const { return m_pImage; }
        unsigned int GetCodebookBytes() const { return m_uiCodebookBytes; }
        unsigned int GetCodebookSize() const { return m_uiCodebookSize; }
        unsigned int GetCodebookFormat() const { return m_uiCodebookFormat; }
        unsigned int GetImageBytes() const { return m_uiImageBytes; }
        unsigned int GetHeight() const { return m_uiHeight; }
        unsigned int GetWidth() const { return m_uiWidth; }
        unsigned int GetImageFormat() const { return m_uiImageFormat; }

    private:
        void DeleteResources();

    public:
        unsigned int m_uiFlags;             // +0x0c (1 = a named Toshi material)
        unsigned int m_uiImageFormat;       // +0x10
        unsigned int m_uiWidth;             // +0x14
        unsigned int m_uiHeight;            // +0x18
        unsigned int m_uiCodebookFormat;    // +0x1c
        unsigned int m_uiCodebookSize;      // +0x20
        unsigned int m_uiImageBytes;        // +0x24
        unsigned int m_uiCodebookBytes;     // +0x28
        void*        m_pImage;              // +0x2c
        void*        m_pCodebook;           // +0x30
        TMaterial*   m_pToshiMaterial;      // +0x34
        char         m_szName[32];          // +0x38
    };

    class TDIMP_API TSubMesh : public TNodeList<TSubMesh>::TNode         // 0x28
    {
    public:
        TSubMesh() { m_puiBones = 0; m_pVertices = 0; m_pusIndices = 0; }
        TSubMesh(TDImp* imp) { m_puiBones = 0; m_pVertices = 0; m_pusIndices = 0; FileRead(imp); }
        ~TSubMesh() { DeleteResources(); }

        void GetDimensions(unsigned int& numIndices, unsigned int& numVertices, unsigned int* const bones);
        bool FileRead(TDImp* imp);
        void DeleteResources();

        unsigned short* GetIndices() { return m_pusIndices; }
        SkinVertex* GetVertices() { return m_pVertices; }

        unsigned int    m_uiNumIndices;     // +0x0c
        unsigned int    m_uiNumVertices;    // +0x10
        unsigned int    m_uiNumBones;       // +0x14
        unsigned int*   m_puiBones;         // +0x18
        SkinVertex*     m_pVertices;        // +0x1c
        unsigned short* m_pusIndices;       // +0x20
        unsigned int    m_uiMaxIndex;       // +0x24 (highest index + 1)
    };

    class TDIMP_API TSkinMesh : public TNodeList<TSkinMesh>::TNode       // 0x38
    {
    public:
        TSkinMesh() { m_pBoneNames = 0; }
        TSkinMesh(TDImp* imp) { m_pBoneNames = 0; FileRead(imp); }
        ~TSkinMesh() { DeleteResources(); }

        bool FileRead(TDImp* imp);
        bool FileReadSubMeshes(TDImp* imp);
        const char* GetRigidBoneName();
        const char* GetBoneNames();
        void GetDimensions(unsigned int& numSubMeshes, unsigned int& a, unsigned int& b, unsigned int& numBones);

        unsigned int GetNumBones() { return m_uiNumBones; }
        TMeshType GetMeshType() { return m_eMeshType; }
        void SetMaterial(TDImpMaterial* material) { m_pMaterial = material; }
        TDImpMaterial* GetMaterial() { return m_pMaterial; }
        TNodeList<TSubMesh>* GetSubMeshList() { return &m_oSubMeshes; }

    private:
        void DeleteResources();

    public:
        TDImpMaterial*      m_pMaterial;        // +0x0c
        TNodeList<TSubMesh> m_oSubMeshes;       // +0x10
        TMeshType           m_eMeshType;        // +0x20
        unsigned int        m_uiNumSubMeshes;   // +0x24
        unsigned int        m_uiDim28;          // +0x28
        unsigned int        m_uiDim2C;          // +0x2c
        unsigned int        m_uiNumBones;       // +0x30
        char*               m_pBoneNames;       // +0x34 (m_uiNumBones names of 15 chars)
    };

    class TDIMP_API TBone : public TNodeList<TBone>::TNode               // 0x80
    {
    public:
        enum TYPE { TYPE_BONE, TYPE_TRANSFORM, TYPE_ROOT };

        TBone() {}
        TBone(TDImp* imp) { FileRead(imp); }
        ~TBone();

        bool FileRead(TDImp* imp);
        bool ComputeManualAndHook(TDImp* imp, TVector4& translation, TQuaternion& rotation);
        void ReorderKeySeqs(TDImp* imp);
        bool IsBadBone();
        void ComposeWithParent();
        void GetPose(TQuaternion& rotation, TVector3& translation);
        void GetPose(TQuaternion& rotation, TVector4& translation);
        void GetPoseC(TQuaternion& rotation, TVector3& translation);
        void GetPoseC(TQuaternion& rotation, TVector4& translation);
        bool IsBadBoneName();
        bool IsBoneNode();
        bool IsTransformNode();
        bool IsRootNode();

        void SetHasChildren(bool hasChildren) { m_bHasChildren = hasChildren; }
        bool HasChildren() { return m_bHasChildren; }
        TBone* GetParent() { return m_pParent; }
        void SetParent(TBone* parent) { m_pParent = parent; }
        unsigned int GetIndex2() { return m_uiIndex2; }
        unsigned int GetIndex() { return m_uiIndex; }
        void SetIndex2(unsigned int index) { m_uiIndex2 = index; }
        void SetIndex(unsigned int index) { m_uiIndex = index; }
        void SetManual(bool manual) { m_bManual = manual; }
        bool IsPathHook() { return m_bPathHook; }
        bool IsManual() { return m_bManual; }
        TYPE GetType() { return m_eType; }
        void SetPose(TQuaternion& rotation, TVector4& translation)
        {
            m_oRotation = rotation;
            m_oTranslation = translation;
        }
        const char* GetParentName() const { return m_szParentName; }
        const char* GetName() const { return m_szName; }

        unsigned int m_uiIndex;             // +0x0c
        unsigned int m_uiIndex2;            // +0x10
        TBone*       m_pParent;             // +0x14
        TYPE         m_eType;               // +0x18
        char         m_szName[15];          // +0x1c
        char         m_szParentName[15];    // +0x2b
        TQuaternion  m_oRotation;           // +0x3c
        TVector4     m_oTranslation;        // +0x4c
        TQuaternion  m_oRotationC;          // +0x5c (composed with the parents)
        TVector4     m_oTranslationC;       // +0x6c
        bool         m_bManual;             // +0x7c
        bool         m_bPathHook;           // +0x7d
        bool         m_bHasChildren;        // +0x7e
    };

    class TDIMP_API TSkeleton                                           // 0x14
    {
    public:
        TSkeleton() {}
        TSkeleton(TDImp* imp) { FileRead(imp); }
        ~TSkeleton() { DeleteResources(); }

        bool FileRead(TDImp* imp);
        bool FileReadBones(TDImp* imp);
        void DebugDump();

        TNodeList<TBone>* GetBoneList() { return &m_oBones; }
        void GetDimensions(unsigned int& numBones) { numBones = m_oBones.Count(); }

    private:
        void DeleteResources();

    public:
        unsigned int     m_uiNumBones;      // +0x00
        TNodeList<TBone> m_oBones;          // +0x04
    };

    TDImp();
    TDImp(const char* fileName);
    ~TDImp();

    bool Create(const char* fileName);
    bool ComputeBoneBox(TDImp* imp, TBone* bone, bool flag);
    void BoneSpaceTransform(TBone* bone, TVector3& point, TAABox& box, bool& first);

    unsigned int* GetManualRemapTableInverse() { return m_puiManualRemapTableInverse; }
    unsigned int* GetManualRemapTable() { return m_puiManualRemapTable; }
    unsigned int GetManualCount() const { return m_uiManualCount; }
    void SetBoundingRadius(float radius) { m_fBoundingRadius = radius; }
    float GetBoundingRadius() { return m_fBoundingRadius; }
    void SetBoundingCenter(float x, float y, float z)
    {
        m_vBoundingCenter.x = x;
        m_vBoundingCenter.y = y;
        m_vBoundingCenter.z = z;
    }
    TVector3 GetBoundingCenter() { return m_vBoundingCenter; }
    unsigned int GetLODCount() const { return m_uiLODCount; }
    TNodeList<TSkinMesh>* GetSkinMeshList(unsigned int lod) { return &m_aSkinMeshes[lod]; }
    TNodeList<TSequence>* GetSequenceSet() { return &m_oSequenceSet; }
    TSkeleton* GetSkeleton() { return m_pSkeleton; }

private:
    tag_TShortVector3* GetSeqShortTranslation() { return &m_oSeqShortTranslation; }
    float GetSequenceLength() { return m_fSequenceLength; }
    void SetSequenceLength(float length) { m_fSequenceLength = length; }
    void BuildImpParentChildHierarchy();
    void BoneRemap();
    void ComposeHierarchy();
    void ReorderAllKeySeqs();
    void CollapseSequences(TBone* bone, TVector4& translation, TQuaternion& rotation);
    bool ImportSequenceSet(TNodeList<TSequence>& set, TDImp* imp);
    bool ImportSequence(TSequence& sequence, TDImp* imp);
    bool ImportKeySequence(TKeySequence& keySeq, TDImp* imp);
    bool ImportKey(TKeyFrame& key, TDImp* imp);
    bool ImportMeshSet(unsigned int& a, unsigned int& b, TDImp* imp);
    void MarkMeshSet();
    void SetSeqShortTranslation(tag_TShortVector3& translation);
    bool IsManualRestricted(TBone* bone);

public:
    enum { MAX_LODS = 20 };

    TVector3                m_vBoundingCenter;              // +0x004
    float                   m_fBoundingRadius;              // +0x010
    unsigned int            m_uiLODCount;                   // +0x014
    TNodeList<TDImpMaterial> m_aMaterials[MAX_LODS];        // +0x018
    TNodeList<TSkinMesh>    m_aSkinMeshes[MAX_LODS];        // +0x158
    TSkeleton*              m_pSkeleton;                    // +0x298
    TNodeList<TSequence>    m_oSequenceSet;                 // +0x29c
    unsigned int            m_uiManualCount;                // +0x2ac
    unsigned int*           m_puiManualRemapTable;          // +0x2b0
    unsigned int*           m_puiManualRemapTableInverse;   // +0x2b4
    float                   m_fSequenceLength;              // +0x2b8
    tag_TShortVector3       m_oSeqShortTranslation;         // +0x2bc
};

} // namespace Toshi
#endif
