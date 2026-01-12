
// ======================================================================
#include "stdafx.h"

#include "Graphics/Camera.h"
// ======================================================================

using namespace Math;

// ======================================================================
// ===================== 公用方法 ========================================
CCamera::CCamera()
    : m_Mode(CameraMode::FreeLook),          // 摄像机模式
      m_Position(0.0f, 0.0f, 0.0f),          // 摄像机位置
      m_Target(0.0f, 0.0f, 0.0f),            // 观察目标
      m_Up(0.0f, 1.0f, 0.0f),                // 上方向
      m_Forward(0.0f, 0.0f, -1.0f),          // 前方向
      m_Right(1.0f, 0.0f, 0.0f),             // 右方向
      m_WorldUp(0.0f, 1.0f, 0.0f),           // 世界上方向
      m_Yaw(-90.0f),                         // 偏航角（绕Y轴旋转）
      m_Pitch(0.0f),                         // 俯仰角（绕X轴旋转）
      m_Roll(0.0f),                          // 翻滚角（绕Z轴旋转）
      m_MoveSpeed(1.0f),                     // 移动速度
      m_RotationSpeed(1.0f),                 // 旋转速度
      m_ZoomSpeed(1.0f),                     // 缩放速度
      m_ThirdPersonTarget(0.0f, 0.0f, 0.0f), // 第三人称跟踪目标
      m_Distance(5.0f),                      // 距离目标的距离
      m_Height(2.0f),                        // 相对高度
      m_AngleAroundTarget(0.0f),             // 围绕目标的角度
      m_OrbitCenter(0.0f, 0.0f, 0.0f),       // 轨道中心
      m_OrbitDistance(10.0f),                // 轨道距离
      m_OrbitPhi(45.0f),                     // 垂直角度
      m_OrbitTheta(0.0f),                    // 水平角度
      m_Fov(45.0f),                          // 视野角度
      m_AspectRatio(16.0f / 9.0f),           // 宽高比
      m_NearPlane(0.1f),                     // 近裁剪面
      m_FarPlane(1000.0f),                   // 远裁剪面
      m_EnableMouseLook(FALSE),              // 是否启用鼠标视角控制
      m_LastMousePos({0, 0}),                // 上一帧鼠标位置
      m_MouseLookActive(FALSE),              // 鼠标视角是否激活
      m_MouseSensitivity(0.1f),              // 鼠标灵敏度
      m_ClampPitch(TRUE),                    // 是否限制俯仰角
      m_MinPitch(-89.0f),                    // 最小俯仰角
      m_MaxPitch(89.0f),                     // 最大俯仰角
      m_ClampDistance(TRUE),                 // 是否限制距离
      m_MinDistance(1.0f),                   // 最小距离
      m_MaxDistance(50.0f),                  // 最大距离
      m_ShakeEnabled(FALSE),                 // 是否启用震动
      m_ShakeIntensity(0.0f),                // 震动强度
      m_ShakeDuration(0.0f),                 // 震动持续时间
      m_ShakeTimer(0.0f),                    // 震动计时器
      m_ShakeOffset(0.0f, 0.0f, 0.0f)        // 震动偏移
{
    srand(static_cast<unsigned>(time(NULL)));
    UpdateCameraVectors();
}

void CCamera::Initialize(const Vector3 &position, const Vector3 &target, const Vector3 &up)
{
    m_Position = position;
    m_Target = target;
    m_WorldUp = up;
    m_Up = up;

    // 计算初始方向
    m_Forward = (m_Target - m_Position).Normalize();
    m_Right = m_Forward.Cross(m_WorldUp).Normalize();
    m_Up = m_Right.Cross(m_Forward).Normalize();

    // 计算初始欧拉角
    Vector3 forward = m_Forward;
    m_Yaw = atan2(forward.z, forward.x) * RAD_TO_DEG;
    m_Pitch = asin(forward.y) * RAD_TO_DEG;

    // 限制俯仰角
    if (m_ClampPitch)
    {
        if (m_Pitch > m_MaxPitch)
            m_Pitch = m_MaxPitch;
        if (m_Pitch < m_MinPitch)
            m_Pitch = m_MinPitch;
    }

    UpdateCameraVectors();
}

void CCamera::SetProjection(FLOAT fov, FLOAT aspectRatio, FLOAT nearPlane, FLOAT farPlane)
{
    if (aspectRatio <= 0.0f) aspectRatio = 1.0f;

    m_Fov = fov;
    m_AspectRatio = aspectRatio;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;

    m_ProjDirty = TRUE;
}

void CCamera::UpdateCameraVectors()
{
    // 根据欧拉角计算前方向向量
    FLOAT yawRad = ToRadians(m_Yaw);
    FLOAT pitchRad = ToRadians(m_Pitch);

    Vector3 forward;
    forward.x = Cos(yawRad) * Cos(pitchRad);
    forward.y = Sin(pitchRad);
    forward.z = Sin(yawRad) * Cos(pitchRad);

    m_Forward = forward.Normalize();

    // 计算右方向和上方向
    m_Right = m_Forward.Cross(m_WorldUp).Normalize();
    m_Up = m_Right.Cross(m_Forward).Normalize();
}

void CCamera::SetMode(CameraMode mode)
{
    if (m_Mode == mode)
        return;

    CameraMode oldMode = m_Mode;
    m_Mode = mode;

    // 模式切换时的初始化
    switch (mode)
    {
    case CameraMode::ThirdPerson:
        if (oldMode != CameraMode::ThirdPerson)
        {
            m_Distance = 5.0f;
            m_Height = 2.0f;
            m_AngleAroundTarget = 0.0f;
        }
        break;

    case CameraMode::Orbital:
        if (oldMode != CameraMode::Orbital)
        {
            m_OrbitDistance = 10.0f;
            m_OrbitPhi = 45.0f;
            m_OrbitTheta = 0.0f;
        }
        break;

    default:
        break;
    }
}

void CCamera::Move(FLOAT forwardAmount, FLOAT rightAmount, FLOAT upAmount)
{
    Vector3 moveOffset(0.0f, 0.0f, 0.0f);

    if (m_Mode == CameraMode::FreeLook || m_Mode == CameraMode::FirstPerson)
    {
        // 1. 处理前后移动 (W/S)
        // 如果你希望抬头时按 W 是往天上飞，保持使用 m_Forward
        // 如果你希望像走路一样只在水平面移动，请取消下面两行的注释：
        Vector3 flattenedForward = Vector3(m_Forward.x, 0.0f, m_Forward.z).Normalize();
        moveOffset += flattenedForward * forwardAmount;
        moveOffset = moveOffset + m_Forward * forwardAmount;

        // 2. 处理左右移动 (A/D)
        // 确保使用水平的右向量。由于 m_Right 是由 m_Forward x m_WorldUp 得到的，
        // 只要 m_WorldUp 是 (0,1,0)，它天生就是水平的，这里维持现状即可。
        moveOffset = moveOffset + m_Right * rightAmount;

        // 3. 处理上下移动 (Q/E) - 核心修复点
        // 不要使用局部的 m_Up，直接使用世界坐标系的向上向量 (0, 1, 0)
        // 这样无论你怎么抬头低头，Q/E 永远是垂直升降
        moveOffset = moveOffset + m_WorldUp * upAmount;
    }
    else
    {
        // 其他模式（如第三人称、轨道模式）的通用移动逻辑
        moveOffset = m_Forward * forwardAmount + m_Right * rightAmount + m_Up * upAmount;
    }

    // 应用位移
    m_Position = m_Position + moveOffset * m_MoveSpeed;

    // 关键修复：同步更新 Target
    // 只有 Position 和 Target 同时移动相同的向量，视角才不会产生非预期的“旋转感”
    if (m_Mode == CameraMode::FreeLook || m_Mode == CameraMode::FirstPerson)
    {
        m_Target = m_Position + m_Forward;
    }
    else if (m_Mode == CameraMode::ThirdPerson)
    {
        m_ThirdPersonTarget = m_ThirdPersonTarget + moveOffset;
    }
    else if (m_Mode == CameraMode::Orbital)
    {
        m_OrbitCenter = m_OrbitCenter + moveOffset;
    }

    m_ViewDirty = TRUE;
}

void CCamera::Rotate(FLOAT yaw, FLOAT pitch, FLOAT roll)
{

    m_Yaw += yaw * m_RotationSpeed;
    m_Pitch += pitch * m_RotationSpeed;
    m_Roll += roll * m_RotationSpeed;

    // 限制俯仰角
    if (m_ClampPitch)
    {
        if (m_Pitch > m_MaxPitch)
            m_Pitch = m_MaxPitch;
        if (m_Pitch < m_MinPitch)
            m_Pitch = m_MinPitch;
    }

    UpdateCameraVectors();

    // 根据模式更新目标
    switch (m_Mode)
    {
    case CameraMode::FirstPerson:
    case CameraMode::FreeLook:
        m_Target = m_Position + m_Forward;
        break;

    default:
        break;
    }
}

void CCamera::Zoom(FLOAT amount)
{
    switch (m_Mode)
    {
    case CameraMode::FirstPerson:
    case CameraMode::FreeLook:
        // 自由视角缩放是向前移动
        Move(amount, 0.0f, 0.0f);
        break;

    case CameraMode::ThirdPerson:
        m_Distance -= amount * m_ZoomSpeed;
        if (m_ClampDistance)
        {
            if (m_Distance < m_MinDistance)
                m_Distance = m_MinDistance;
            if (m_Distance > m_MaxDistance)
                m_Distance = m_MaxDistance;
        }
        break;

    case CameraMode::Orbital:
        m_OrbitDistance -= amount * m_ZoomSpeed;
        if (m_ClampDistance)
        {
            if (m_OrbitDistance < m_MinDistance)
                m_OrbitDistance = m_MinDistance;
            if (m_OrbitDistance > m_MaxDistance)
                m_OrbitDistance = m_MaxDistance;
        }
        break;
    }
}

void CCamera::SetPosition(const Vector3 &position)
{
    m_Position = position;

    if (m_Mode == CameraMode::FirstPerson || m_Mode == CameraMode::FreeLook)
    {
        m_Target = m_Position + m_Forward;
    }
}

void CCamera::SetTarget(const Vector3 &target)
{
    m_Target = target;

    if (m_Mode == CameraMode::FirstPerson || m_Mode == CameraMode::FreeLook)
    {
        m_Forward = (m_Target - m_Position).Normalize();

        // 重新计算欧拉角
        Vector3 forward = m_Forward;
        m_Yaw = atan2(forward.z, forward.x) * RAD_TO_DEG;
        m_Pitch = asin(forward.y) * RAD_TO_DEG;

        UpdateCameraVectors();
    }
}

void CCamera::LookAt(const Vector3 &position)
{
    SetTarget(position);
}

void CCamera::SetEulerAngles(FLOAT yaw, FLOAT pitch, FLOAT roll)
{
    m_Yaw = yaw;
    m_Pitch = pitch;
    m_Roll = roll;

    if (m_ClampPitch)
    {
        if (m_Pitch > m_MaxPitch)
            m_Pitch = m_MaxPitch;
        if (m_Pitch < m_MinPitch)
            m_Pitch = m_MinPitch;
    }

    UpdateCameraVectors();

    if (m_Mode == CameraMode::FirstPerson || m_Mode == CameraMode::FreeLook)
    {
        m_Target = m_Position + m_Forward;
    }
}

void CCamera::SetThirdPersonTarget(const Vector3 &target)
{
    m_ThirdPersonTarget = target;
}

void CCamera::SetThirdPersonDistance(FLOAT distance)
{
    m_Distance = distance;
    if (m_ClampDistance)
    {
        if (m_Distance < m_MinDistance)
            m_Distance = m_MinDistance;
        if (m_Distance > m_MaxDistance)
            m_Distance = m_MaxDistance;
    }
}

void CCamera::SetThirdPersonHeight(FLOAT height)
{
    m_Height = height;
}

void CCamera::SetThirdPersonAngle(FLOAT angle)
{
    m_AngleAroundTarget = angle;
}

void CCamera::SetOrbitCenter(const Vector3 &center)
{
    m_OrbitCenter = center;
}

void CCamera::SetOrbitDistance(FLOAT distance)
{
    m_OrbitDistance = distance;
    if (m_ClampDistance)
    {
        if (m_OrbitDistance < m_MinDistance)
            m_OrbitDistance = m_MinDistance;
        if (m_OrbitDistance > m_MaxDistance)
            m_OrbitDistance = m_MaxDistance;
    }
}

void CCamera::SetOrbitAngles(FLOAT phi, FLOAT theta)
{
    m_OrbitPhi = phi;
    m_OrbitTheta = theta;
}

void CCamera::EnableMouseLook(BOOL enable)
{
    m_EnableMouseLook = enable;
    if (!enable)
    {
        m_MouseLookActive = FALSE;
    }
}

void CCamera::StartMouseLook()
{
    if (m_EnableMouseLook)
    {
        m_MouseLookActive = TRUE;
    }
}

void CCamera::StopMouseLook()
{
    m_MouseLookActive = FALSE;
}

void CCamera::ProcessMouseMovement(INT dx, INT dy)
{
    if (!m_MouseLookActive)
        return;

    // // 添加调试输出
    // static int callCount = 0;
    // callCount++;

    // char buffer[256];
    // sprintf_s(buffer, "[Camera] ProcessMouseMovement #%d: dx=%ld, dy=%ld\n",
    //           callCount, dx, dy);
    // OutputDebugStringA(buffer);

    FLOAT xOffset = dx * m_MouseSensitivity;
    FLOAT yOffset = dy * m_MouseSensitivity;

    // 根据模式处理鼠标移动
    switch (m_Mode)
    {
    case CameraMode::FirstPerson:
    case CameraMode::FreeLook:
        Rotate(xOffset, yOffset);
        break;

    case CameraMode::ThirdPerson:
        // 第三人称：水平移动旋转环绕角，垂直移动旋转俯仰角
        m_AngleAroundTarget -= xOffset;
        m_Pitch -= yOffset; // 复用 Pitch 来控制观察高度
        if (m_ClampPitch)
        {
            m_Pitch = (m_Pitch > m_MaxPitch) ? m_MaxPitch : (m_Pitch < m_MinPitch ? m_MinPitch : m_Pitch);
        }
        break;

    case CameraMode::Orbital:
        m_OrbitTheta += xOffset;
        m_OrbitPhi -= yOffset;
        m_OrbitPhi = (m_OrbitPhi > 179.0f) ? 179.0f : (m_OrbitPhi < 1.0f ? 1.0f : m_OrbitPhi);
        break;
    }
    m_ViewDirty = TRUE;
}

void CCamera::ProcessMouseWheel(INT delta)
{
    // FLOAT zoomAmount = static_cast<FLOAT>(delta) / 120.0f; // 标准滚轮单位
    // Zoom(zoomAmount);

    // 1. 标准化输入
    // INFO: 鼠标滚动敏感度调节
    FLOAT sensitivity = 0.1f;
    FLOAT scrollAmount = static_cast<FLOAT>(delta) / 120.0f * sensitivity;

    // 2. 根据不同模式执行不同逻辑
    switch (m_Mode)
    {
    case CameraMode::FirstPerson:
    case CameraMode::FreeLook:
        // 方案 A: 改变视野 (Zoom FOV) - 更有高级感
        m_Fov -= scrollAmount * m_ZoomSpeed;
        // 限制 FOV 范围，防止镜像翻转或畸变过大 (1.0° 到 120.0°)
        if (m_Fov < 1.0f)
            m_Fov = 1.0f;
        if (m_Fov > 120.0f)
            m_Fov = 120.0f;
        m_ProjDirty = TRUE; // 记得标记投影矩阵需要更新
        break;

    case CameraMode::ThirdPerson:
    case CameraMode::Orbital:
        // 方案 B: 改变距离 (Dolly Zoom)
        m_Distance -= scrollAmount * m_ZoomSpeed;
        if (m_ClampDistance)
        {
            if (m_Distance < m_MinDistance)
                m_Distance = m_MinDistance;
            if (m_Distance > m_MaxDistance)
                m_Distance = m_MaxDistance;
        }
        m_ViewDirty = TRUE;
        break;
    }
}

void CCamera::UpdateFirstPerson()
{
    // 第一人称摄像机位置固定，目标随旋转变化
    m_Target = m_Position + m_Forward;
}

void CCamera::UpdateThirdPerson(FLOAT deltaTime)
{
    // 计算水平距离
    FLOAT horizontalDistance = m_Distance * Math::Cos(ToRadians(m_Pitch));
    FLOAT verticalDistance = m_Distance * Math::Sin(ToRadians(m_Pitch));

    // 计算摄像机位置
    FLOAT theta = ToRadians(m_AngleAroundTarget);
    FLOAT offsetX = horizontalDistance * Math::Sin(theta);
    FLOAT offsetZ = horizontalDistance * Math::Cos(theta);

    Vector3 desiredPosition;
    desiredPosition.x = m_ThirdPersonTarget.x - offsetX;
    desiredPosition.y = m_ThirdPersonTarget.y + verticalDistance + m_Height;
    desiredPosition.z = m_ThirdPersonTarget.z - offsetZ;

    // 平滑插值 (Lerp): 防止相机跟随角色时产生高频抖动
    m_Position = Vector3::Lerp(m_Position, desiredPosition, deltaTime * m_FollowSpeed);

    // 始终看向目标中心（可根据高度偏移）
    m_Target = m_ThirdPersonTarget + Vector3(0, m_Height * 0.5f, 0);

    // 更新前方向
    m_Forward = (m_Target - m_Position).Normalize();
}

void CCamera::UpdateFreeLook()
{
    // 自由视角和第一人称类似
    UpdateFirstPerson();
}

void CCamera::UpdateOrbital()
{
    // 将球坐标转换为笛卡尔坐标
    FLOAT phiRad = ToRadians(m_OrbitPhi);
    FLOAT thetaRad = ToRadians(m_OrbitTheta);

    m_Position.x = m_OrbitCenter.x + m_OrbitDistance * Math::Sin(phiRad) * Math::Cos(thetaRad);
    m_Position.y = m_OrbitCenter.y + m_OrbitDistance * Math::Cos(phiRad);
    m_Position.z = m_OrbitCenter.z + m_OrbitDistance * Math::Sin(phiRad) * Math::Sin(thetaRad);

    // 摄像机看向中心
    m_Target = m_OrbitCenter;
    m_Forward = (m_Target - m_Position).Normalize();
}

void CCamera::StartShake(FLOAT intensity, FLOAT duration)
{
    m_ShakeEnabled = TRUE;
    m_ShakeIntensity = intensity;
    m_ShakeDuration = duration;
    m_ShakeTimer = 0.0f;
    m_ShakeOffset = Vector3(0.0f, 0.0f, 0.0f);
}

void CCamera::StopShake()
{
    m_ShakeEnabled = FALSE;
    m_ShakeIntensity = 0.0f;
    m_ShakeDuration = 0.0f;
    m_ShakeTimer = 0.0f;
    m_ShakeOffset = Vector3(0.0f, 0.0f, 0.0f);
}

void CCamera::GenerateShakeOffset()
{
    FLOAT x = (static_cast<FLOAT>(rand()) / RAND_MAX * 2.0f - 1.0f) * m_ShakeIntensity;
    FLOAT y = (static_cast<FLOAT>(rand()) / RAND_MAX * 2.0f - 1.0f) * m_ShakeIntensity;
    FLOAT z = (static_cast<FLOAT>(rand()) / RAND_MAX * 2.0f - 1.0f) * m_ShakeIntensity;

    m_ShakeOffset = Vector3(x, y, z);
}

void CCamera::SetPitchLimits(FLOAT minPitch, FLOAT maxPitch)
{
    m_MinPitch = minPitch;
    m_MaxPitch = maxPitch;
    m_ClampPitch = TRUE;
}

void CCamera::SetDistanceLimits(FLOAT minDistance, FLOAT maxDistance)
{
    m_MinDistance = minDistance;
    m_MaxDistance = maxDistance;
    m_ClampDistance = TRUE;
}

void CCamera::GetEulerAngles(FLOAT &yaw, FLOAT &pitch, FLOAT &roll) const
{
    yaw = m_Yaw;
    pitch = m_Pitch;
    roll = m_Roll;
}

void CCamera::CalculateLookAtMatrix(const Vector3 &position, const Vector3 &target, const Vector3 &up, Matrix4 &matrix)
{
    matrix = Matrix4::LookAt(position, target, up);
}

void CCamera::GetViewMatrix(Matrix4 &matrix) const
{
    if (m_ViewDirty || m_ShakeEnabled) // 有震动时必须实时计算
    {
        Vector3 finalPos = m_Position;
        Vector3 finalTarget = m_Target;

        if (m_ShakeEnabled)
        {
            finalPos = finalPos + m_ShakeOffset;
            finalTarget = finalTarget + m_ShakeOffset;
        }

        CalculateLookAtMatrix(finalPos, finalTarget, m_Up, m_CachedView);
        m_ViewDirty = FALSE;
    }
    matrix = m_CachedView;
}

void CCamera::GetProjectionMatrix(Matrix4 &matrix) const
{
    matrix = Matrix4::Perspective(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
}

void CCamera::GetViewProjectionMatrix(Matrix4 &matrix) const
{
    if (m_ProjDirty)
    {
        m_CachedProj = Matrix4::Perspective(m_Fov, m_AspectRatio, m_NearPlane, m_FarPlane);
        m_ProjDirty = FALSE;
    }

    // 计算 matrix = proj * view
    // 在列优先布局下，matrix[c][r] = sum(proj[k][r] * view[c][k])
    // 其中 c 为列索引，r 为行索引

    matrix = m_CachedProj;
}

void CCamera::Update(FLOAT deltaTime)
{
    // static int updateCount = 0;
    // updateCount++;

    // char buffer[256];
    // sprintf_s(buffer, "[Camera] Update #%d: deltaTime=%.4f, Mode=%d, MouseLook=%d\n",
    //           updateCount, deltaTime, static_cast<int>(m_Mode), m_EnableMouseLook);
    // OutputDebugStringA(buffer);

    // 1. 根据模式更新摄像机位置
    if (m_Mode == CameraMode::FreeLook || m_Mode == CameraMode::FirstPerson)
    {
        UpdateCameraVectors();
        m_Target = m_Position + m_Forward;
    }
    else if (m_Mode == CameraMode::ThirdPerson)
    {
        UpdateThirdPerson(deltaTime);
    }
    else if (m_Mode == CameraMode::Orbital)
    {
        UpdateOrbital();
    }

    // 2. 更新摄像机震动
    if (m_ShakeEnabled)
    {
        m_ShakeTimer += deltaTime;
        if (m_ShakeTimer >= m_ShakeDuration)
        {
            StopShake();
        }
        else
        {
            // 每帧生成新的震动偏移
            GenerateShakeOffset();
        }
    }

    // 3. 标记视图矩阵需要重新生成
    m_ViewDirty = TRUE;
}

void CCamera::ApplyViewMatrix() const
{
    Matrix4 viewMatrix;
    GetViewMatrix(viewMatrix);

    // std::cout << "Applying View..." << std::endl;

    // std::cout << "[调试] 应用视图矩阵:" << std::endl;
    // std::cout << "  位置: (" << m_Position.x << ", " << m_Position.y << ", " << m_Position.z << ")" << std::endl;
    // std::cout << "  目标: (" << m_Target.x << ", " << m_Target.y << ", " << m_Target.z << ")" << std::endl;

    // for (int i = 0; i < 4; i++)
    // {
    //     std::cout << "  ";
    //     for (int j = 0; j < 4; j++)
    //     {
    //         std::cout << viewMatrix.m[i * 4 + j] << " ";
    //     }
    //     std::cout << std::endl;
    // }

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewMatrix.m);

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cout << "[错误] glLoadMatrixf失败: " << error << std::endl;
    }
}

void CCamera::ApplyProjectionMatrix() const
{
    Matrix4 projMatrix;
    GetProjectionMatrix(projMatrix);

    // std::cout << "[调试] 应用投影矩阵:" << std::endl;
    // std::cout << "  FOV: " << m_Fov << ", 宽高比: " << m_AspectRatio << std::endl;

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projMatrix.m);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cout << "[错误] glLoadMatrixf失败: " << error << std::endl;
    }
}

void CCamera::Reset()
{
    this->SetMode(CameraMode::FreeLook);

    m_Position = Vector3(0.0f, 0.0f, 10.0f);
    m_Target = Vector3(0.0f, 0.0f, 0.0f);
    m_Up = Vector3(0.0f, 1.0f, 0.0f);

    m_WorldUp = Vector3(0.0f, 1.0f, 0.0f);
    m_Forward = Vector3(0.0f, 0.0f, -1.0f);
    m_Right = Vector3(1.0f, 0.0f, 0.0f);

    m_Yaw = -90.0f;
    m_Pitch = 0.0f;
    m_Roll = 0.0f;

    // m_MoveSpeed = 1.0f;
    // m_RotationSpeed = 1.0f;
    // m_ZoomSpeed = 1.0f;

    m_ThirdPersonTarget = Vector3(0.0f, 0.0f, 0.0f);
    m_Distance = 5.0f;
    m_Height = 2.0f;
    m_AngleAroundTarget = 0.0f;

    m_OrbitCenter = Vector3(0.0f, 0.0f, 0.0f);
    m_OrbitDistance = 10.0f;
    m_OrbitPhi = 45.0f;
    m_OrbitTheta = 0.0f;

    m_Fov = 45.0f;
    // m_AspectRatio = 16.0f / 9.0f; // 不需要改动
    // m_NearPlane = 0.1f;
    // m_FarPlane = 1000.0f;

    m_EnableMouseLook = FALSE;
    m_LastMousePos = {0, 0};
    m_MouseLookActive = FALSE;
    m_MouseSensitivity = 0.1f;

    m_ClampPitch = TRUE;
    m_MinPitch = -89.0f;
    m_MaxPitch = 89.0f;

    m_ClampDistance = TRUE;
    m_MinDistance = 1.0f;
    m_MaxDistance = 50.0f;

    m_ViewDirty = TRUE;
    m_ProjDirty = TRUE;

    StopShake();
}