
// ======================================================================
#ifndef __CAMERA_ENTITY_H__
#define __CAMERA_ENTITY_H__
// ======================================================================
#include <Windows.h>
#include <memory>
#include "Core/Entity.h"
// ======================================================================
class Vector3;
class Matrix4;
class Quaternion;
// ======================================================================
// 摄像机模式枚举
enum class CameraMode
{
    FirstPerson, // 第一人称 (锁定在父节点位置)
    ThirdPerson, // 第三人称 (相对于父节点有偏移)
    FreeLook,    // 自由视角 (独立在空间移动)
    Orbital      // 轨道视角 (围绕中心点旋转)
};

// ======================================================================
// 相机实体类
class CCameraEntity : public CEntity
{

public:
    virtual ~CCameraEntity() = default;

    // 使用 CEntity 的模板创建方法
    template <typename... Args>
    static std::shared_ptr<CCameraEntity> Create(Args &&...args)
    {
        auto entity = std::shared_ptr<CCameraEntity>(new CCameraEntity(std::forward<Args>(args)...));
        entity->m_uID = ++s_nextID;
        return entity;
    }

    // ======================================================================
    // 渲染应用
    // ======================================================================
    Matrix4 GetViewMatrix() const;
    void ApplyViewMatrix() const;

    Matrix4 GetProjectionMatrix() const;
    void SetProjection(FLOAT fov, FLOAT aspect, FLOAT nearP, FLOAT farP);
    void ApplyProjectionMatrix() const;

    // ======================================================================
    // 逻辑控制
    // ======================================================================
    // 重写 Update，处理不同模式下的位移逻辑
    virtual void Update(float deltaTime) override;
    void StartShake(float intensity, float duration);

    void SetMode(CameraMode mode) { m_Mode = mode; }
    CameraMode GetMode() const { return m_Mode; }

    // ======================================================================
    // 属性存取
    // ======================================================================
    FLOAT GetFOV() const { return m_Fov; }
    FLOAT GetNear() const { return m_Near; }
    FLOAT GetFar() const { return m_Far; }

    // 获取相机在世界坐标系下的方向向量 (直接从世界矩阵提取)
    Vector3 GetForward() const;
    Vector3 GetRight() const;
    Vector3 GetUp() const;

    void SetPitchLimits(FLOAT maxPitch) { m_MaxPitchAngle = maxPitch; }

    void LookAt(const Vector3 &target);

    void SetSensitivity(FLOAT s) { m_MouseSensitivity = s; }

    void ProcessMouseMovement(INT dx, INT dy);
    void ProcessMouseWheel(INT delta);

protected:
    CCameraEntity();

    CameraMode m_Mode;

    // 投影参数
    FLOAT m_Fov, m_Aspect, m_Near, m_Far;

    // 震动属性
    BOOL m_ShakeEnabled;
    FLOAT m_ShakeTimer;
    FLOAT m_ShakeIntensity;
    Vector3 m_ShakeOffset;

    // 内部控制变量
    Quaternion m_rotation;
    FLOAT m_CurrentYaw;
    FLOAT m_CurrentPitch;
    FLOAT m_MaxPitchAngle;

    FLOAT m_MouseSensitivity;
};

#endif // __CAMERA_ENTITY_H__