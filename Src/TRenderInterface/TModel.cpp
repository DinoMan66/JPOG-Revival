// TRenderInterface.dll - TModel.cpp (0x95f0-0x9b90 in the original)
#define TRENDERINTERFACE_EXPORTS
#include "TNewDelete.h"
#include "TModel.h"
#include "TAnimation.h"
#include "TRenderInterface.h"
#include "TRenderContext.h"
#include "TShader.h"
#include "TFile.h"
#include "TCString.h"

namespace Toshi {

TModelInstance* TModel::CreateInstance()                                // 0x9670
{
    TModelInstance* instance = new TModelInstance(this);
    if (instance)
    {
        instance->m_pSkeletonInstance = m_pSkeleton->CreateInstance(true);
        if (instance->m_pSkeletonInstance)
        {
            m_iInstanceCount++;
            return instance;
        }
        delete instance;
    }
    return 0;
}

TModel::TModel()                                                        // 0x96d0
    : m_uiFlags(0), m_iInstanceCount(0), m_iLODCount(0), m_pSkeleton(0)
{
}

bool TModel::LoadTMD(const char* fileName)                              // 0x9730
{
    struct { unsigned int m_uiMagic; unsigned int m_uiVersion; unsigned int m_uiUnk; } header;
    struct
    {
        char         m_szKeyLibrary[16];    // +0x00
        unsigned int m_uiSkeletonSize;      // +0x10
        unsigned int m_uiSkeletonFlags;     // +0x14
        int          m_iTBase;              // +0x18
        int          m_iQBase;              // +0x1c
        int          m_iSBase;              // +0x20
        int          m_iTCount;             // +0x24
        int          m_iQCount;             // +0x28
        int          m_iSCount;             // +0x2c
    } skeletonHeader;

    bool result = false;
    TFile* file = TFile::Create(fileName, TFile::FileMode_Read);
    if (!file)
        return false;

    file->Read(&header, sizeof(header));
    if (header.m_uiMagic == 0x4c444d54)     // "TMDL"
    {
        file->Read(&skeletonHeader, sizeof(skeletonHeader));
        m_pSkeleton = (TSkeleton*)tmalloc(skeletonHeader.m_uiSkeletonSize, 0, -1);
        if (m_pSkeleton)
        {
            file->Read(m_pSkeleton, skeletonHeader.m_uiSkeletonSize);
            m_pSkeleton->Create(skeletonHeader.m_uiSkeletonFlags);
            TKeyframeLibrary* library =
                TRenderInterface::GetRenderer()->GetKeyframeLibraryManager().GetLibrary(skeletonHeader.m_szKeyLibrary);
            m_pSkeleton->m_oKeyLibraryInstance.CreateEx(library, skeletonHeader.m_iTBase, skeletonHeader.m_iQBase,
                                                        skeletonHeader.m_iSBase, skeletonHeader.m_iTCount,
                                                        skeletonHeader.m_iQCount, skeletonHeader.m_iSCount);
            if (LoadTRB(file))
                result = true;
        }
    }
    TFile::Destroy(file);
    return result;
}

void TModelInstance::Delete()                                           // 0x9840
{
    if (m_pSkeletonInstance)
    {
        m_pSkeletonInstance->Delete();
        m_pSkeletonInstance = 0;
    }
    m_pModel->m_iInstanceCount--;
    m_pModel = 0;
    delete this;
}

void TModelInstance::Render()                                           // 0x9880
{
    m_pSkeletonInstance->UpdateState();
    TRenderInterface::GetRenderer()->GetCurrentRenderContext()->SetSkeletonInstance(m_pSkeletonInstance);
    TModelLOD& lod = m_pModel->m_aLODs[m_iLOD];
    for (int i = 0; i < lod.m_iMeshCount; i++)
        lod.m_ppMeshes[i]->Render();
}

TModelInstance::~TModelInstance()                                       // 0x98f0
{
}

TModelInstance::TModelInstance(TModel* model)                           // 0x9900
{
    m_pModel = model;
    m_pSkeletonInstance = 0;
    m_iLOD = 0;
}

void TModelInstance::Update(float deltaTime)                            // 0x9920
{
    m_pSkeletonInstance->UpdateTime(deltaTime);
}

TModel::~TModel()                                                       // 0x9940
{
}

int TModelInstance::SetLOD(int lod)                                     // 0x99e0
{
    int old = m_iLOD;
    m_iLOD = lod;
    return old;
}

TSkeletonInstance* TModelInstance::GetSkeletonInstance()                 // 0x99f0
{
    return m_pSkeletonInstance;
}

TModel* TModelInstance::GetModel()                                      // 0x9a00
{
    return m_pModel;
}

void TModel::SetDebugName(const char* name)                             // 0x9a10
{
}

const char* TModel::GetDebugName()                                      // 0x9a20
{
    return "";
}

float TModel::GetCoarseBoundingRadius()                                 // 0x9a30
{
    return m_fCoarseBoundingRadius;
}

TSphere& TModel::GetVisSphere()                                         // 0x9a40
{
    return m_aLODs[0].m_oVisSphere;
}

TModelLOD& TModel::GetLOD(int index)                                    // 0x9a50
{
    return m_aLODs[index];
}

int TModel::GetLODCount()                                               // 0x9a70
{
    return m_iLODCount;
}

int TModel::GetInstanceCount()                                          // 0x9a80
{
    return m_iInstanceCount;
}

bool TModel::IsLoaded()                                                 // 0x9a90
{
    if (m_uiFlags & FLAG_LOADED)
        return true;
    return false;
}

bool TModel::IsCreated()                                                // 0x9aa0
{
    if (m_uiFlags & FLAG_CREATED)
        return true;
    return false;
}

TSkeleton* TModel::GetSkeleton()                                        // 0x9ab0
{
    return m_pSkeleton;
}

unsigned int TModel::GetFlags()                                         // 0x9ac0
{
    return m_uiFlags;
}

TSphere& TModelLOD::GetVisSphere()                                      // 0x9b30
{
    return m_oVisSphere;
}

TMesh* TModelLOD::GetMesh(int index)                                    // 0x9b40
{
    return m_ppMeshes[index];
}

int TModelLOD::GetMeshCount()                                           // 0x9b50
{
    return m_iMeshCount;
}

TModelLOD::TModelLOD()                                                  // 0x9b60
    : m_oVisSphere(0.0f, 0.0f, 0.0f, 0.0f), m_iMeshCount(0), m_ppMeshes(0)
{
}

} // namespace Toshi
