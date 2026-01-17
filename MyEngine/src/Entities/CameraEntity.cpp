
// ======================================================================
#include "stdafx.h"
#include "Entities/CameraEntity.h"
#include "Core/GameEngine.h"
#include "Core/Renderer.h"
#include "Core/Entity.h"
// ======================================================================

CCameraEntity::CCameraEntity()
    : CEntity(),                    //
      m_Mode(CameraMode::FreeLook), //
      m_Fov(45.0f),                 //
      m_Aspect(1.33f),              //
      m_Near(0.1f),                 //
      m_Far(1000.0f),               //
      m_ShakeEnabled(FALSE),        //
      m_ShakeTimer(0.0f),           //
      m_ShakeIntensity(0.0f),       //
      m_MouseSensitivity(0.1f),     //
      m_CurrentYaw(-90.0f),         // 初始化：看向-Z方向
      m_CurrentPitch(0.0f),         // 初始化：水平
      m_MaxPitchAngle(89.0f)        //
{
    SetName(L"MainCamera");

    // 修正：正确初始化旋转（看向-Z方向）
    // 默认欧拉角：偏航角-90度（看向-Z轴），俯仰角0度（水平）
    m_rotation = Quaternion::FromEuler(m_CurrentPitch, m_CurrentYaw, 0.0f);
    SetRotation(m_rotation);
}

// ======================================================================
// 渲染应用
// ======================================================================
Matrix4 CCameraEntity::GetViewMatrix() const
{
    // 核心逻辑：View矩阵 = 相机世界变换矩阵的逆
    Matrix4 view = GetWorldMatrix();
    view.Inverse();

    // 叠加震动偏移（仅影响视图，不修改实体实际位置）
    if (m_ShakeEnabled)
    {
        Matrix4 shake = Matrix4::Translation(m_ShakeOffset);
        view = shake * view;
    }

    return view;
}

void CCameraEntity::ApplyViewMatrix() const
{
    Matrix4 view = GetViewMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(view.m);
}

void CCameraEntity::SetProjection(FLOAT fov, FLOAT aspect, FLOAT nearP, FLOAT farP)
{
    m_Fov = fov;
    m_Aspect = aspect;
    m_Near = nearP;
    m_Far = farP;
}

Matrix4 CCameraEntity::GetProjectionMatrix() const
{
    return Matrix4::Perspective(m_Fov, m_Aspect, m_Near, m_Far);
}

void CCameraEntity::ApplyProjectionMatrix() const
{
    auto renderer = CGameEngine::GetInstance().GetRenderer();
    float currentAspect = renderer->GetAspectRatio();

    // 2. 获取最新的投影矩阵
    Matrix4 proj = Matrix4::Perspective(m_Fov, currentAspect, m_Near, m_Far);

    // 3. 应用到 OpenGL 管线
    glMatrixMode(GL_PROJECTION);
    // CAUTION
    // glLoadIdentity();
    glLoadMatrixf(proj.m);

// 检查 OpenGL 错误 (保持你原来的调试风格)
#ifdef _DEBUG
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        LogError(L"相机投影矩阵加载失败.\n");
    }
#endif
}

// ======================================================================
// 逻辑控制
// ======================================================================

void CCameraEntity::Update(float deltaTime)
{
    // 1. 根据模式处理特定逻辑
    switch (m_Mode)
    {
    case CameraMode::FirstPerson:
        // 第一人称：完全跟随父节点，旋转由父节点控制
        if (auto parent = m_pParent.lock())
        {
            // 直接使用父节点的四元数旋转
            m_rotation = parent->GetRotation();
            SetRotation(m_rotation);

            // 同步欧拉角状态，确保鼠标控制平滑过渡
            Vector3 euler = m_rotation.ToEuler();
            m_CurrentPitch = euler.x;
            m_CurrentYaw = euler.y;
        }
        break;

    case CameraMode::FreeLook:
        // 自由视角：使用独立的旋转控制
        SetRotation(m_rotation);
        break;

    case CameraMode::ThirdPerson:
        // 第三人称：围绕父节点旋转
        if (auto parent = m_pParent.lock())
        {
            // 获取父节点位置作为焦点
            Vector3 parentPos = parent->GetWorldPosition();

            // 计算相机位置（球坐标）
            float distance = 5.0f; // 可配置的距离
            Vector3 offset = m_rotation * Vector3(0, 0, distance);
            SetPosition(parentPos + offset);

            // 看向父节点
            LookAt(parentPos);
        }
        break;

    case CameraMode::Orbital:
    {                                 // 轨道视角：围绕固定点旋转
        Vector3 orbitCenter(0, 0, 0); // 可配置的轨道中心

        // 计算相机位置
        float orbitDistance = 10.0f;
        Vector3 orbitOffset = m_rotation * Vector3(0, 0, orbitDistance);
        SetPosition(orbitCenter + orbitOffset);

        // 看向轨道中心
        LookAt(orbitCenter);
    }
    break;
    }
    // 2. 更新震动计时器
    if (m_ShakeEnabled)
    {
        m_ShakeTimer -= deltaTime;
        if (m_ShakeTimer <= 0.0f)
        {
            m_ShakeEnabled = FALSE;
            m_ShakeOffset = Vector3(0, 0, 0);
        }
        else
        {
            // 产生随机震动
            m_ShakeOffset.x = ((rand() % 100) / 50.0f - 1.0f) * m_ShakeIntensity;
            m_ShakeOffset.y = ((rand() % 100) / 50.0f - 1.0f) * m_ShakeIntensity;
            m_ShakeOffset.z = ((rand() % 100) / 50.0f - 1.0f) * m_ShakeIntensity;
        }
    }

    // 3. 调用基类 Update，递归计算整个场景树的世界矩阵
    CEntity::Update(deltaTime);
}

void CCameraEntity::StartShake(float intensity, float duration)
{
    m_ShakeEnabled = TRUE;
    m_ShakeIntensity = intensity;
    m_ShakeTimer = duration;
}

// ======================================================================
// 属性存取
// ======================================================================

Vector3 CCameraEntity::GetForward() const
{
    // 从四元数直接计算前向向量 (通常 OpenGL 是 -Z 为前)
    return m_rotation * Vector3(0, 0, -1);
}

Vector3 CCameraEntity::GetRight() const
{
    return m_rotation * Vector3(1, 0, 0);
}

Vector3 CCameraEntity::GetUp() const
{
    return m_rotation * Vector3(0, 1, 0);
}

void CCameraEntity::LookAt(const Vector3 &target)
{
    Vector3 position = GetWorldPosition();
    Vector3 forward = (target - position).Normalize();

    // 计算看向目标的旋转
    if (forward.LengthSquared() > 0.001f) // 避免除以零
    {
        Vector3 worldUp(0, 1, 0);
        Matrix4 lookAtMatrix = Matrix4::LookAt(position, target, worldUp);

        // 1. 更新四元数缓存
        m_rotation = lookAtMatrix.GetRotation();

        // 2. 关键优化：反向同步 Euler 状态，确保鼠标接管时平滑过渡
        // 注意：只有在这里（由程序控制视角时）才允许调用 ToEuler
        Vector3 euler = m_rotation.ToEuler();
        m_CurrentPitch = euler.x;
        m_CurrentYaw = euler.y;

        // 3. 同步到基类
        SetRotation(m_rotation);
    }
}

void CCameraEntity::ProcessMouseMovement(INT dx, INT dy)
{
    if (m_Mode != CameraMode::FreeLook && m_Mode != CameraMode::Orbital)
        return; // 只有自由视角和轨道视角需要鼠标控制

    // 1. 直接修改持久化的 float 变量
    m_CurrentYaw += static_cast<float>(dx) * m_MouseSensitivity;
    m_CurrentPitch -= static_cast<float>(dy) * m_MouseSensitivity;
    m_CurrentPitch = Math::Clamp(m_CurrentPitch, -m_MaxPitchAngle, m_MaxPitchAngle);

    // 2. 合成四元数 (单一源头)
    m_rotation = Quaternion::FromEuler(m_CurrentPitch, m_CurrentYaw, 0.0f);

    // 3. 传递给基类
    SetRotation(m_rotation);
}

void CCameraEntity::ProcessMouseWheel(INT delta)
{
    // 鼠标滚轮控制：自由视角调整FOV，轨道/第三人称调整距离
    float zoomSpeed = 1.0f;
    float zoomAmount = static_cast<float>(delta) * 0.1f * zoomSpeed;

    switch (m_Mode)
    {
    case CameraMode::FreeLook:
        // 自由视角：调整视野
        m_Fov -= zoomAmount;
        m_Fov = Math::Clamp(m_Fov, 1.0f, 120.0f);
        break;

    case CameraMode::ThirdPerson:
    case CameraMode::Orbital:
        // TODO:: 这些模式通常有距离概念，这里可以添加距离控制逻辑
        // 例如：m_Distance -= zoomAmount;
        break;
    }
}