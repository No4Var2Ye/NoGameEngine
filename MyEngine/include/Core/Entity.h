
// ======================================================================
#ifndef __ENTITY_H__
#define __ENTITY_H__
// ======================================================================
#include <windows.h>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include "EngineConfig.h"
#include "GL/gl.h"
#include "Math/Vector3.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
// ======================================================================
class CModel;
// ======================================================================
class CEntity : public std::enable_shared_from_this<CEntity>
{
public:
    virtual ~CEntity() = default;

    CEntity(const CEntity &) = delete;
    CEntity &operator=(const CEntity &) = delete;

    void SetParent(std::shared_ptr<CEntity> pParent);
    void AddChild(std::shared_ptr<CEntity> pChild);
    BOOL RemoveChild(unsigned int uID);
    const std::vector<std::shared_ptr<CEntity>> &GetChildren() const { return m_children; }
    std::shared_ptr<CEntity> GetChild(size_t index) const;
    // 递归查找子实体
    std::shared_ptr<CEntity> FindChildByName(const std::wstring &name);

    template <typename... Args>
    static std::shared_ptr<CEntity> Create(Args &&...args)
    {
        auto entity = std::shared_ptr<CEntity>(new CEntity(std::forward<Args>(args)...));
        entity->m_uID = ++s_nextID;
        return entity;
    }

    // 更新与渲染
    virtual void Update(float deltaTime);
    virtual void Render()
    {
        for (auto &pChild : m_children)
        {
            if (pChild)
                pChild->Render();
        }
    };

    unsigned int GetID() const { return m_uID; }

    // 实体名称
    void SetName(const std::wstring &name) { m_name = name; }
    const std::wstring &GetName() const { return m_name; }

    // 变换操作
    void SetPosition(const Vector3 &pos);
    const Vector3 &GetPosition() const { return m_position; }
    void SetRotation(const Vector3 &euler);
    void SetRotation(const Quaternion &quat);
    const Quaternion &GetRotation() const { return m_rotation; }
    void SetScale(const Vector3 &scale);
    const Vector3 &GetScale() const { return m_scale; }

    Vector3 GetWorldPosition() const;
    Matrix4 GetWorldMatrix() const;

    // 可见性控制
    void SetVisible(BOOL visible) { m_bVisible = visible; }
    BOOL IsVisible() const { return m_bVisible; }

    // 自动贴地
    void SetSnapToTerrain(BOOL enable, float offset = 0.0f);
    BOOL IsAutoSnapEnabled() const { return m_bSnapToTerrain; }
    float GetGroundOffset() const { return m_fTerrainOffset; }
    void SetLastSnapPos(const Vector3 &pos) { m_LastSnapPos = pos; }
    Vector3 GetLastSnapPos() const { return m_LastSnapPos; }

protected:
    unsigned int m_uID = 0;
    CEntity();

    static unsigned int s_nextID;
    std::wstring m_name;
    std::weak_ptr<CEntity> m_pParent;
    std::vector<std::shared_ptr<CEntity>> m_children;
    std::unordered_map<unsigned int, std::shared_ptr<CEntity>> m_childrenMap;

    void InternalAddChild(std::shared_ptr<CEntity> pChild);
    void InternalRemoveChild(unsigned int uID);

    // 变换属性
    Vector3 m_position;
    Quaternion m_rotation;
    Vector3 m_scale;

    BOOL m_bVisible;

    BOOL m_bSnapToTerrain;                                         // 是否开启自动贴地
    float m_fTerrainOffset;                                        // 高度偏移
    Vector3 m_LastSnapPos = Vector3(99999.0f, 99999.0f, 99999.0f); // 初始给个极大值确保第一次必执行

    mutable Matrix4 m_cachedWorldMatrix;
    mutable BOOL m_bWorldDirty;

    void ApplyTransform() const;
    void MarkDirty();
};

#endif // __ENTITY_H__