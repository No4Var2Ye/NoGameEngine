
// ======================================================================
#ifndef __ENTITY_H__
#define __ENTITY_H__
// ======================================================================
#include <windows.h>
#include <string>
#include <memory>
#include "EngineConfig.h"
#include "GL/gl.h"
#include "Math/Vector3.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
// ======================================================================
class CModel;
// ======================================================================
class CEntity
{
public:
    CEntity();
    virtual ~CEntity() = default;

    // 更新与渲染
    virtual void Update(float deltaTime);
    virtual void Render() = 0;

    // 实体名称
    void SetName(const std::wstring &name) { m_name = name; }
    const std::wstring &GetName() const { return m_name; }

    // 变换操作
    void SetPosition(const Vector3 &pos)
    {
        m_position = pos;
        m_isDirty = true;
    }
    void SetRotation(const Vector3 &euler)
    {
        m_rotation = Quaternion::FromEuler(euler.x, euler.y, euler.z);
        m_isDirty = true;
    }
    void SetScale(const Vector3 &scale)
    {
        m_scale = scale;
        m_isDirty = true;
    }

    // 可见性控制
    void SetVisible(BOOL visible) { m_bVisible = visible; }
    BOOL IsVisible() const { return m_bVisible; }

protected:
    std::wstring m_name;

    // 变换属性
    Vector3 m_position;
    Quaternion m_rotation;
    Vector3 m_scale;

    mutable Matrix4 m_worldMatrix;
    mutable bool m_isDirty;
    BOOL m_bVisible = true;

    void ApplyTransform() const;
};

#endif // __ENTITY_H__