// Toshi::TResource - base of every render resource (TRenderInterface.dll), size 0x30.
//   +0x00 vtable (TObject)       +0x04 TNodeTree<TResource>::TNode (tree, next, prev, parent, child)
//   +0x18 TRenderInterface*      +0x1C char name[15]     +0x2B flags     +0x2C uid
// The members defined in the class body here sit in the original's inline/COMDAT region
// (0x7d90-0x7e80); the rest are in TResource.cpp (0x12bd0-0x13030).
#ifndef TRESOURCE_H
#define TRESOURCE_H
#include "TRenderAPI.h"
#include "TObject.h"
#include "TNodeTree.h"
#ifndef TRES_PARENT
#define TRES_PARENT 4   // round 37
#endif

namespace Toshi {

class TRenderInterface;

class TRENDER_API TResource : public TObject, public TNodeTree<TResource>::TNode
{
    TOSHI_DECLARE_CLASS(TResource)
    friend class TNodeTree<TResource>;     // the tree deletes its nodes
    friend class TRenderInterface;
public:
    enum FLAG
    {
        FLAG_VALID       = 0x01,
        FLAG_CREATED     = 0x02,
        FLAG_DYING       = 0x04,
        FLAG_INPLACE     = 0x08,       // created by CreateResourceInPlace: destroyed, not freed
        FLAG_DEAD        = 0x10,
        FLAG_SCENEOBJECT = 0x20
    };

    TResource();

    virtual bool Create();                  // slot 3
    virtual bool Validate();                // slot 4
    virtual void Invalidate();              // slot 5
    virtual void DestroyResource();         // slot 6
    virtual bool OnLostDevice();            // slot 7
    virtual bool OnResetDevice();           // slot 8
protected:
    virtual void OnDestroy();               // slot 9
    virtual ~TResource();
public:
    void SetName(const char* name);
    void SetParent(TResource* parent);
    void RecurseSimple(TNodeTree<TResource>::t_RecurseSimpleCallback callback, TResource* start, void* userData);
    unsigned int GetUId() const;
    const char* GetName() const;
    TRenderInterface* GetRenderer() const;
    void MarkAsSceneObject();
    bool IsDying();
    bool IsCreated();
    bool IsValid();

    const char* GetDebugDyingInfo() { return ""; }
    const char* GetDebugInfo() { return ""; }
    TResource* Next() const { return m_pNext; }
#ifdef TRES_CHILD_NODE
    TResource* Child() const { return TNodeTree<TResource>::TNode::Child(); }
#else
    TResource* Child() const { return m_pChild; }
#endif
    TResource* Parent() const
    {
#if TRES_PARENT == 1
        TResource* parent = m_pParent;
        if (parent == 0 || parent == m_pTree->GetRoot())
            parent = 0;
        return parent;
#elif TRES_PARENT == 2
        return (m_pParent == 0 || m_pParent == m_pTree->GetRoot()) ? 0 : m_pParent;
#elif TRES_PARENT == 4
        TNodeTree<TResource>* tree = m_pTree;
        TResource* parent = m_pParent;
        if (parent == 0 || parent == tree->GetRoot())
            parent = 0;
        return parent;
#elif TRES_PARENT == 3
        TResource* root = m_pTree->GetRoot();
        TResource* parent = m_pParent;
        if (parent == 0 || parent == root)
            parent = 0;
        return parent;
#else
        if (m_pParent == 0 || m_pParent == m_pTree->GetRoot())
            return 0;
        return m_pParent;
#endif
    }
    bool IsSceneObject()
    {
        if (m_iFlags & FLAG_SCENEOBJECT)
            return true;
        return false;
    }

    TRenderInterface* m_pRenderer;          // +0x18
#ifdef TRES_NAME_U
    unsigned char     m_szName[15];         // +0x1C
#else
    char              m_szName[15];         // +0x1C
#endif
    unsigned char     m_iFlags;             // +0x2B
    unsigned int      m_uiUId;              // +0x2C
};

class TRENDER_API TNullResource : public TResource
{
    TOSHI_DECLARE_CLASS(TNullResource)
};

} // namespace Toshi
#endif
