// Toshi::TModel / TModelLOD / TModelInstance (TRenderInterface.dll, TModel.cpp, 0x95f0-0x9b90).
// TModel is abstract: the platform renderer loads the meshes (slot 5, called by LoadTMD).
#ifndef TMODEL_H
#define TMODEL_H
#include "TRenderAPI.h"
#include "TVector.h"

namespace Toshi {

class TMesh;
class TModel;
class TFile;
class TSkeleton;
class TSkeletonInstance;

class TRENDER_API TModelLOD                                             // 0x18
{
public:
    TModelLOD();

    TSphere& GetVisSphere();
    TMesh* GetMesh(int index);
    int GetMeshCount();

    TSphere m_oVisSphere;       // +0x00
    int     m_iMeshCount;       // +0x10
    TMesh** m_ppMeshes;         // +0x14
};

class TRENDER_API TModelInstance                                        // 0xc
{
    friend class TModel;
    friend class TGFXFileInstanceObject;
public:
    void Delete();
    void Render();
    void Update(float deltaTime);
    int SetLOD(int lod);
    TSkeletonInstance* GetSkeletonInstance();
    TModel* GetModel();

protected:
    TModelInstance(TModel* model);
    ~TModelInstance();

    TModel*            m_pModel;            // +0x0
    TSkeletonInstance* m_pSkeletonInstance; // +0x4
    int                m_iLOD;              // +0x8

private:
    TModelInstance(const TModelInstance&);
    TModelInstance& operator=(const TModelInstance&);
};

class TRENDER_API TModel                                                // 0x90
{
    friend class TModelInstance;
    friend class TGFXFileInstanceObject;
public:
    enum { MAX_LODS = 5 };
    enum FLAGS { FLAG_CREATED = 1, FLAG_LOADED = 2 };

    virtual bool Create(const char* name, bool load) = 0;               // slots 0-3, 5: names unknown
    virtual void Delete() = 0;
    virtual void Render() = 0;
    virtual bool Validate() = 0;
protected:
    virtual ~TModel();                                                  // slot 4
    virtual bool LoadTRB(TFile* file) = 0;                              // slot 5 (meshes, after LoadTMD)

public:
    TModelInstance* CreateInstance();
    const char* GetDebugName();
    float GetCoarseBoundingRadius();
    TSphere& GetVisSphere();
    TModelLOD& GetLOD(int index);
    int GetLODCount();
    int GetInstanceCount();
    bool IsLoaded();
    bool IsCreated();
    TSkeleton* GetSkeleton();
    unsigned int GetFlags();

protected:
    TModel();
    bool LoadTMD(const char* fileName);
    void SetDebugName(const char* name);

    unsigned int m_uiFlags;                 // +0x04
    int          m_iInstanceCount;          // +0x08
    int          m_iLODCount;               // +0x0c
    float        m_fCoarseBoundingRadius;   // +0x10
    TSkeleton*   m_pSkeleton;               // +0x14
    TModelLOD    m_aLODs[MAX_LODS];         // +0x18

private:
    TModel(const TModel&);
    TModel& operator=(const TModel&);
};

} // namespace Toshi
#endif
