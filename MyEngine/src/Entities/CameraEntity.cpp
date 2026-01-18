
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
      m_MouseSensitivity(0.1f),     // 鼠标灵敏度
      m_CurrentYaw(0.0f),           // 初始化：看向-Z方向
      m_CurrentPitch(0.0f),         // 初始化：水平
      m_MaxPitchAngle(89.0f)        //
{
    m_Type = EntityType::Camera;
    m_bVisible = FALSE; // 相机默认不可见
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
    // 传统方法：直接求逆世界矩阵
    // Matrix4 view = GetWorldMatrix();
    // view.Inverse();

    // 优化方法：显式分解为旋转和平移的逆，性能更好且数值稳定
    // 1. 旋转逆 = 旋转矩阵的转置（对于正交矩阵）
    Matrix4 rotInv = Matrix4::Rotation(m_rotation).Transposed();

    // 2. 平移逆 = 反向平移（取反位置向量）
    Matrix4 transInv = Matrix4::Translation(-GetWorldPosition());

    // 3. 视图矩阵组合：先进行旋转逆，再进行平移逆
    // 数学原理：View = (T * R)^-1 = R^-1 * T^-1 = R^T * (-translation)
    Matrix4 view = rotInv * transInv;

    // 4. 叠加震动偏移效果（仅影响视图，不修改实体实际位置）
    // 震动在视图空间应用，模拟相机抖动效果
    if (m_ShakeEnabled)
    {
        // 创建震动平移矩阵
        Matrix4 shake = Matrix4::Translation(m_ShakeOffset);
        // 将震动变换应用到视图矩阵（后乘，在相机变换后应用震动）
        view = shake * view;

        // 注意：震动偏移是在相机本地空间应用的
        // 这会产生屏幕抖动效果，但不会实际移动相机在世界中的位置
    }

    // 5. 可选的缩放逆（如果需要处理非均匀缩放）
    // 对于相机视图，通常不需要处理缩放，因为相机应该是单位缩放的
    // 但如果需要支持相机缩放，可以添加：
    // Matrix4 scaleInv = Matrix4::Scaling(Vector3::One / GetWorldScale());
    // view = scaleInv * view;  // 注意顺序：缩放逆应该在旋转逆之前？

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

    glMatrixMode(GL_MODELVIEW);

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
            Vector3 parentPos = parent->GetWorldPosition();
            SetPosition(parentPos + Vector3(0, 1.8f, 0));

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
            Vector3 parentPos = parent->GetWorldPosition();

            // 计算球坐标位置
            float distance = 5.0f;
            float yawRad = m_CurrentYaw * 0.01745329f; // 度转弧度
            float pitchRad = m_CurrentPitch * 0.01745329f;

            // 球坐标转直角坐标
            float x = distance * sinf(yawRad) * cosf(pitchRad);
            float y = distance * sinf(pitchRad);
            float z = distance * cosf(yawRad) * cosf(pitchRad);

            Vector3 offset(x, y, z);
            SetPosition(parentPos + offset);

            // 看向父节点（稍微偏上）
            LookAt(parentPos + Vector3(0, 1.5f, 0));
        }
        break;

    case CameraMode::Orbital:
        if (auto parent = m_pParent.lock())
        {
            Vector3 parentPos = parent->GetWorldPosition();

            // 计算球坐标位置
            float orbitDistance = 8.0f;
            float yawRad = m_CurrentYaw * 0.01745329f;
            float pitchRad = m_CurrentPitch * 0.01745329f;

            float x = orbitDistance * sinf(yawRad) * cosf(pitchRad);
            float y = orbitDistance * sinf(pitchRad);
            float z = orbitDistance * cosf(yawRad) * cosf(pitchRad);

            Vector3 orbitOffset(x, y, z);
            SetPosition(parentPos + orbitOffset);

            // 看向父节点
            LookAt(parentPos);
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
        m_rotation = lookAtMatrix.Matrix4::GetRotation();

        // 2. 关键优化：反向同步 Euler 状态，确保鼠标接管时平滑过渡
        // 注意：只有在这里（由程序控制视角时）才允许调用 ToEuler
        Vector3 euler = m_rotation.ToEuler();
        m_CurrentPitch = euler.x;
        m_CurrentYaw = euler.y;

        // 3. 同步到基类
        SetRotation(m_rotation);
    }
}

void CCameraEntity::ResetOrientation(float yaw, float pitch)
{
    m_CurrentYaw = yaw;
    m_CurrentPitch = pitch;

    // 立即根据新的偏航俯仰角更新四元数
    m_rotation = Quaternion::FromEuler(m_CurrentPitch, m_CurrentYaw, 0.0f);
    // MarkDirty();
}

void CCameraEntity::ProcessMouseMovement(INT dx, INT dy)
{
    // if (m_Mode != CameraMode::FreeLook && m_Mode != CameraMode::Orbital)
    //     return; // 只有自由视角和轨道视角需要鼠标控制

    const float kSensitivityScale = 0.05f;

    // 1. 直接修改持久化的 float 变量
    m_CurrentYaw -= static_cast<float>(dx) * m_MouseSensitivity * kSensitivityScale;
    m_CurrentPitch -= static_cast<float>(dy) * m_MouseSensitivity * kSensitivityScale;
    m_CurrentPitch = Math::Clamp(m_CurrentPitch, -m_MaxPitchAngle, m_MaxPitchAngle);

    // 限制俯仰角范围
    m_CurrentPitch = Math::Clamp(m_CurrentPitch, -m_MaxPitchAngle, m_MaxPitchAngle);

    // 规范化偏航角到0-360度范围
    while (m_CurrentYaw >= 360.0f)
        m_CurrentYaw -= 360.0f;
    while (m_CurrentYaw < 0.0f)
        m_CurrentYaw += 360.0f;

    // 2. 合成四元数 (单一源头)
    m_rotation = Quaternion::FromEuler(m_CurrentPitch, m_CurrentYaw, 0.0f);

    // 3. 传递给基类
    SetRotation(m_rotation);
}

void CCameraEntity::ProcessMouseWheel(INT delta)
{
    FLOAT zoomAmount = static_cast<float>(delta) / 120 * 2.0f;

    // 第一人称通常调整 FOV
    if (m_Mode == CameraMode::FirstPerson)
    {
        m_Fov -= zoomAmount;
        m_Fov = Math::Clamp(m_Fov, 30.0f, 90.0f);
    }
}

void CCameraEntity::ProcessKeyboardMovement(FLOAT forward, FLOAT right, FLOAT up, FLOAT deltaTime)
{
    float moveSpeed = 5.0f * deltaTime;
    Vector3 currentPos = GetPosition();
    Vector3 moveVec(0.0f, 0.0f, 0.0f);

    if (m_Mode == CameraMode::FreeLook)
    {
        // 自由视角：键盘控制移动
        if (forward != 0.0f)
        {
            Vector3 dir = GetForward();
            dir.Normalize();
            moveVec += dir * forward;
        }

        if (right != 0.0f)
        {
            Vector3 side = GetRight();
            side.Normalize();
            moveVec += side * right;
        }

        if (up != 0.0f)
        {
            moveVec += Vector3(0, 1, 0) * up;
        }
    }
    else if (m_Mode == CameraMode::FirstPerson)
    {
        // 第一人称：WASD控制转向，不控制移动（移动由父实体控制）
        float turnSpeed = 90.0f * deltaTime; // 每秒90度转向速度

        if (right > 0.0f) // D键：向右转
        {
            m_CurrentYaw -= turnSpeed;
            LogDebug(L"第一人称转向：向右转，当前Yaw: %.1f°\n", m_CurrentYaw);
        }
        else if (right < 0.0f) // A键：向左转
        {
            m_CurrentYaw += turnSpeed;
            LogDebug(L"第一人称转向：向左转，当前Yaw: %.1f°\n", m_CurrentYaw);
        }

        if (forward < 0.0f) // S键：向后转
        {
            m_CurrentYaw += 180.0f * deltaTime;
            LogDebug(L"第一人称转向：向后转，当前Yaw: %.1f°\n", m_CurrentYaw);
        }

        // 规范化角度
        while (m_CurrentYaw >= 360.0f)
            m_CurrentYaw -= 360.0f;
        while (m_CurrentYaw < 0.0f)
            m_CurrentYaw += 360.0f;

        // 更新旋转
        m_rotation = Quaternion::FromEuler(m_CurrentPitch, m_CurrentYaw, 0.0f);
        SetRotation(m_rotation);

        return; // 第一人称模式下，相机不移动，只转向
    }
    if (moveVec.LengthSquared() > 0.0f && m_Mode == CameraMode::FreeLook)
    {
        moveVec.Normalize();
        SetPosition(currentPos + moveVec * moveSpeed);
    }
}