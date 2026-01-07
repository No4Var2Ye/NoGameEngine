// ======================================================================
#include "stdafx.h"
#include "Graphics/Camera.h"
#include "Math/MathUtils.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
// ======================================================================

using namespace Math;

/**
 * @brief 构造函数
 */
CCamera::CCamera(CameraMode mode)
    : m_Mode(mode)
    , m_Position(0.0f, 0.0f, 0.0f)
    , m_Target(0.0f, 0.0f, -1.0f)
    , m_Up(0.0f, 1.0f, 0.0f)
    , m_Forward(0.0f, 0.0f, -1.0f)
    , m_Right(1.0f, 0.0f, 0.0f)
    , m_WorldUp(0.0f, 1.0f, 0.0f)
    , m_Yaw(-90.0f)
    , m_Pitch(0.0f)
    , m_Roll(0.0f)
    , m_MoveSpeed(5.0f)
    , m_RotationSpeed(1.0f)
    , m_ZoomSpeed(2.0f)
    , m_ThirdPersonTarget(0.0f, 0.0f, 0.0f)
    , m_Distance(5.0f)
    , m_Height(2.0f)
    , m_AngleAroundTarget(0.0f)
    , m_OrbitCenter(0.0f, 0.0f, 0.0f)
    , m_OrbitDistance(10.0f)
    , m_OrbitPhi(45.0f)
    , m_OrbitTheta(0.0f)
    , m_Fov(45.0f)
    , m_AspectRatio(4.0f / 3.0f)
    , m_NearPlane(0.1f)
    , m_FarPlane(1000.0f)
    , m_EnableMouseLook(FALSE)
    , m_LastMousePos({0, 0})
    , m_MouseLookActive(FALSE)
    , m_MouseSensitivity(0.1f)
    , m_ClampPitch(TRUE)
    , m_MinPitch(-89.0f)
    , m_MaxPitch(89.0f)
    , m_ClampDistance(TRUE)
    , m_MinDistance(1.0f)
    , m_MaxDistance(50.0f)
    , m_ShakeEnabled(FALSE)
    , m_ShakeIntensity(0.0f)
    , m_ShakeDuration(0.0f)
    , m_ShakeTimer(0.0f)
    , m_ShakeOffset(0.0f, 0.0f, 0.0f)
{
    srand(static_cast<unsigned>(time(NULL)));
    UpdateCameraVectors();
}

/**
 * @brief 初始化摄像机
 */
void CCamera::Initialize(const Vector3& position, const Vector3& target, const Vector3& up)
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
        if (m_Pitch > m_MaxPitch) m_Pitch = m_MaxPitch;
        if (m_Pitch < m_MinPitch) m_Pitch = m_MinPitch;
    }
    
    UpdateCameraVectors();
}

/**
 * @brief 设置投影参数
 */
void CCamera::SetProjection(FLOAT fov, FLOAT aspectRatio, FLOAT nearPlane, FLOAT farPlane)
{
    m_Fov = fov;
    m_AspectRatio = aspectRatio;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
}

/**
 * @brief 更新摄像机轴向量
 */
void CCamera::UpdateCameraVectors()
{
    // 根据欧拉角计算前方向向量
    FLOAT yawRad = ToRadians(m_Yaw);
    FLOAT pitchRad = ToRadians(m_Pitch);
    
    Vector3 forward;
    forward.x = cos(yawRad) * cos(pitchRad);
    forward.y = sin(pitchRad);
    forward.z = sin(yawRad) * cos(pitchRad);
    
    m_Forward = forward.Normalize();
    
    // 计算右方向和上方向
    m_Right = m_Forward.Cross(m_WorldUp).Normalize();
    m_Up = m_Right.Cross(m_Forward).Normalize();
}

/**
 * @brief 设置摄像机模式
 */
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

/**
 * @brief 移动摄像机
 */
void CCamera::Move(FLOAT forward, FLOAT right, FLOAT up)
{
    Vector3 moveOffset(0.0f, 0.0f, 0.0f);
    
    // 根据模式计算移动
    switch (m_Mode)
    {
    case CameraMode::FirstPerson:
    case CameraMode::FreeLook:
        moveOffset = m_Forward * forward + m_Right * right + m_Up * up;
        m_Position = m_Position + moveOffset * m_MoveSpeed;
        m_Target = m_Position + m_Forward;
        break;
        
    case CameraMode::ThirdPerson:
        // 第三人称摄像机移动目标
        m_ThirdPersonTarget = m_ThirdPersonTarget + m_Forward * forward + m_Right * right + m_Up * up;
        break;
        
    case CameraMode::Orbital:
        // 轨道摄像机移动中心
        m_OrbitCenter = m_OrbitCenter + m_Forward * forward + m_Right * right + m_Up * up;
        break;
    }
}

/**
 * @brief 旋转摄像机
 */
void CCamera::Rotate(FLOAT yaw, FLOAT pitch, FLOAT roll)
{
    m_Yaw += yaw * m_RotationSpeed;
    m_Pitch += pitch * m_RotationSpeed;
    m_Roll += roll * m_RotationSpeed;
    
    // 限制俯仰角
    if (m_ClampPitch)
    {
        if (m_Pitch > m_MaxPitch) m_Pitch = m_MaxPitch;
        if (m_Pitch < m_MinPitch) m_Pitch = m_MinPitch;
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

/**
 * @brief 缩放摄像机
 */
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
            if (m_Distance < m_MinDistance) m_Distance = m_MinDistance;
            if (m_Distance > m_MaxDistance) m_Distance = m_MaxDistance;
        }
        break;
        
    case CameraMode::Orbital:
        m_OrbitDistance -= amount * m_ZoomSpeed;
        if (m_ClampDistance)
        {
            if (m_OrbitDistance < m_MinDistance) m_OrbitDistance = m_MinDistance;
            if (m_OrbitDistance > m_MaxDistance) m_OrbitDistance = m_MaxDistance;
        }
        break;
    }
}

/**
 * @brief 设置摄像机位置
 */
void CCamera::SetPosition(const Vector3& position)
{
    m_Position = position;
    
    if (m_Mode == CameraMode::FirstPerson || m_Mode == CameraMode::FreeLook)
    {
        m_Target = m_Position + m_Forward;
    }
}

/**
 * @brief 设置观察目标
 */
void CCamera::SetTarget(const Vector3& target)
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

/**
 * @brief 看向指定位置
 */
void CCamera::LookAt(const Vector3& position)
{
    SetTarget(position);
}

/**
 * @brief 设置欧拉角
 */
void CCamera::SetEulerAngles(FLOAT yaw, FLOAT pitch, FLOAT roll)
{
    m_Yaw = yaw;
    m_Pitch = pitch;
    m_Roll = roll;
    
    if (m_ClampPitch)
    {
        if (m_Pitch > m_MaxPitch) m_Pitch = m_MaxPitch;
        if (m_Pitch < m_MinPitch) m_Pitch = m_MinPitch;
    }
    
    UpdateCameraVectors();
    
    if (m_Mode == CameraMode::FirstPerson || m_Mode == CameraMode::FreeLook)
    {
        m_Target = m_Position + m_Forward;
    }
}

/**
 * @brief 设置第三人称跟踪目标
 */
void CCamera::SetThirdPersonTarget(const Vector3& target)
{
    m_ThirdPersonTarget = target;
}

/**
 * @brief 设置第三人称距离
 */
void CCamera::SetThirdPersonDistance(FLOAT distance)
{
    m_Distance = distance;
    if (m_ClampDistance)
    {
        if (m_Distance < m_MinDistance) m_Distance = m_MinDistance;
        if (m_Distance > m_MaxDistance) m_Distance = m_MaxDistance;
    }
}

/**
 * @brief 设置第三人称高度
 */
void CCamera::SetThirdPersonHeight(FLOAT height)
{
    m_Height = height;
}

/**
 * @brief 设置第三人称角度
 */
void CCamera::SetThirdPersonAngle(FLOAT angle)
{
    m_AngleAroundTarget = angle;
}

/**
 * @brief 设置轨道中心
 */
void CCamera::SetOrbitCenter(const Vector3& center)
{
    m_OrbitCenter = center;
}

/**
 * @brief 设置轨道距离
 */
void CCamera::SetOrbitDistance(FLOAT distance)
{
    m_OrbitDistance = distance;
    if (m_ClampDistance)
    {
        if (m_OrbitDistance < m_MinDistance) m_OrbitDistance = m_MinDistance;
        if (m_OrbitDistance > m_MaxDistance) m_OrbitDistance = m_MaxDistance;
    }
}

/**
 * @brief 设置轨道角度
 */
void CCamera::SetOrbitAngles(FLOAT phi, FLOAT theta)
{
    m_OrbitPhi = phi;
    m_OrbitTheta = theta;
}

/**
 * @brief 启用鼠标视角控制
 */
void CCamera::EnableMouseLook(BOOL enable)
{
    m_EnableMouseLook = enable;
    if (!enable)
    {
        m_MouseLookActive = FALSE;
    }
}

/**
 * @brief 开始鼠标视角控制
 */
void CCamera::StartMouseLook()
{
    if (m_EnableMouseLook)
    {
        m_MouseLookActive = TRUE;
        GetCursorPos(&m_LastMousePos);
    }
}

/**
 * @brief 停止鼠标视角控制
 */
void CCamera::StopMouseLook()
{
    m_MouseLookActive = FALSE;
}

/**
 * @brief 处理鼠标移动
 */
void CCamera::ProcessMouseMovement(int x, int y)
{
    if (!m_MouseLookActive)
        return;
    
    int deltaX = x - m_LastMousePos.x;
    int deltaY = m_LastMousePos.y - y;  // 反转Y轴
    
    m_LastMousePos.x = x;
    m_LastMousePos.y = y;
    
    FLOAT xOffset = static_cast<FLOAT>(deltaX) * m_MouseSensitivity;
    FLOAT yOffset = static_cast<FLOAT>(deltaY) * m_MouseSensitivity;
    
    // 根据模式处理鼠标移动
    switch (m_Mode)
    {
    case CameraMode::FirstPerson:
    case CameraMode::FreeLook:
        Rotate(xOffset, yOffset);
        break;
        
    case CameraMode::ThirdPerson:
        m_AngleAroundTarget -= xOffset;
        break;
        
    case CameraMode::Orbital:
        m_OrbitTheta += xOffset;
        m_OrbitPhi -= yOffset;
        
        // 限制Phi角度
        if (m_OrbitPhi > 89.0f) m_OrbitPhi = 89.0f;
        if (m_OrbitPhi < 1.0f) m_OrbitPhi = 1.0f;
        break;
    }
}

/**
 * @brief 处理鼠标滚轮
 */
void CCamera::ProcessMouseWheel(int delta)
{
    FLOAT zoomAmount = static_cast<FLOAT>(delta) / 120.0f;  // 标准滚轮单位
    Zoom(zoomAmount);
}

/**
 * @brief 更新第一人称摄像机
 */
void CCamera::UpdateFirstPerson()
{
    // 第一人称摄像机位置固定，目标随旋转变化
    m_Target = m_Position + m_Forward;
}

/**
 * @brief 更新第三人称摄像机
 */
void CCamera::UpdateThirdPerson()
{
    // 计算水平距离
    FLOAT horizontalDistance = m_Distance * cos(ToRadians(m_Pitch));
    FLOAT verticalDistance = m_Distance * sin(ToRadians(m_Pitch));
    
    // 计算摄像机位置
    FLOAT theta = ToRadians(m_ThirdPersonTarget.x + m_AngleAroundTarget);
    FLOAT offsetX = horizontalDistance * sin(theta);
    FLOAT offsetZ = horizontalDistance * cos(theta);
    
    m_Position.x = m_ThirdPersonTarget.x - offsetX;
    m_Position.y = m_ThirdPersonTarget.y + verticalDistance + m_Height;
    m_Position.z = m_ThirdPersonTarget.z - offsetZ;
    
    // 摄像机看向目标
    m_Target = m_ThirdPersonTarget;
    m_Target.y += m_Height;
    
    // 更新前方向
    m_Forward = (m_Target - m_Position).Normalize();
}

/**
 * @brief 更新自由视角摄像机
 */
void CCamera::UpdateFreeLook()
{
    // 自由视角和第一人称类似
    UpdateFirstPerson();
}

/**
 * @brief 更新轨道视角摄像机
 */
void CCamera::UpdateOrbital()
{
    // 将球坐标转换为笛卡尔坐标
    FLOAT phiRad = ToRadians(m_OrbitPhi);
    FLOAT thetaRad = ToRadians(m_OrbitTheta);
    
    m_Position.x = m_OrbitCenter.x + m_OrbitDistance * sin(phiRad) * cos(thetaRad);
    m_Position.y = m_OrbitCenter.y + m_OrbitDistance * cos(phiRad);
    m_Position.z = m_OrbitCenter.z + m_OrbitDistance * sin(phiRad) * sin(thetaRad);
    
    // 摄像机看向中心
    m_Target = m_OrbitCenter;
    m_Forward = (m_Target - m_Position).Normalize();
}

/**
 * @brief 启动摄像机震动
 */
void CCamera::StartShake(FLOAT intensity, FLOAT duration)
{
    m_ShakeEnabled = TRUE;
    m_ShakeIntensity = intensity;
    m_ShakeDuration = duration;
    m_ShakeTimer = 0.0f;
    m_ShakeOffset = Vector3(0.0f, 0.0f, 0.0f);
}

/**
 * @brief 停止摄像机震动
 */
void CCamera::StopShake()
{
    m_ShakeEnabled = FALSE;
    m_ShakeIntensity = 0.0f;
    m_ShakeDuration = 0.0f;
    m_ShakeTimer = 0.0f;
    m_ShakeOffset = Vector3(0.0f, 0.0f, 0.0f);
}

/**
 * @brief 生成随机震动偏移
 */
void CCamera::GenerateShakeOffset()
{
    FLOAT x = (static_cast<FLOAT>(rand()) / RAND_MAX * 2.0f - 1.0f) * m_ShakeIntensity;
    FLOAT y = (static_cast<FLOAT>(rand()) / RAND_MAX * 2.0f - 1.0f) * m_ShakeIntensity;
    FLOAT z = (static_cast<FLOAT>(rand()) / RAND_MAX * 2.0f - 1.0f) * m_ShakeIntensity;
    
    m_ShakeOffset = Vector3(x, y, z);
}

/**
 * @brief 设置俯仰角限制
 */
void CCamera::SetPitchLimits(FLOAT minPitch, FLOAT maxPitch)
{
    m_MinPitch = minPitch;
    m_MaxPitch = maxPitch;
    m_ClampPitch = TRUE;
}

/**
 * @brief 设置距离限制
 */
void CCamera::SetDistanceLimits(FLOAT minDistance, FLOAT maxDistance)
{
    m_MinDistance = minDistance;
    m_MaxDistance = maxDistance;
    m_ClampDistance = TRUE;
}

/**
 * @brief 获取欧拉角
 */
void CCamera::GetEulerAngles(FLOAT& yaw, FLOAT& pitch, FLOAT& roll) const
{
    yaw = m_Yaw;
    pitch = m_Pitch;
    roll = m_Roll;
}

/**
 * @brief 计算观察矩阵
 */
void CCamera::CalculateLookAtMatrix(const Vector3& position, const Vector3& target, const Vector3& up, FLOAT* matrix)
{
    Vector3 zaxis = (target - position).Normalize();
    Vector3 xaxis = zaxis.Cross(up).Normalize();
    Vector3 yaxis = xaxis.Cross(zaxis);
    
    matrix[0] = xaxis.x;  matrix[4] = xaxis.y;  matrix[8]  = xaxis.z;  matrix[12] = -xaxis.Dot(position);
    matrix[1] = yaxis.x;  matrix[5] = yaxis.y;  matrix[9]  = yaxis.z;  matrix[13] = -yaxis.Dot(position);
    matrix[2] = -zaxis.x; matrix[6] = -zaxis.y; matrix[10] = -zaxis.z; matrix[14] = zaxis.Dot(position);
    matrix[3] = 0.0f;     matrix[7] = 0.0f;     matrix[11] = 0.0f;     matrix[15] = 1.0f;
}

/**
 * @brief 获取观察矩阵
 */
void CCamera::GetViewMatrix(FLOAT* matrix) const
{
    Vector3 position = m_Position;
    Vector3 target = m_Target;
    
    // 应用摄像机震动
    if (m_ShakeEnabled)
    {
        position = position + m_ShakeOffset;
        target = target + m_ShakeOffset;
    }
    
    CalculateLookAtMatrix(position, target, m_Up, matrix);
}

/**
 * @brief 获取投影矩阵
 */
void CCamera::GetProjectionMatrix(FLOAT* matrix) const
{
    FLOAT f = 1.0f / tan(ToRadians(m_Fov) * 0.5f);
    FLOAT aspect = m_AspectRatio;
    FLOAT zNear = m_NearPlane;
    FLOAT zFar = m_FarPlane;
    
    matrix[0] = f / aspect;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;
    
    matrix[4] = 0.0f;
    matrix[5] = f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    
    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = (zFar + zNear) / (zNear - zFar);
    matrix[11] = -1.0f;
    
    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    matrix[15] = 0.0f;
}

/**
 * @brief 获取视图投影矩阵
 */
void CCamera::GetViewProjectionMatrix(FLOAT* matrix) const
{
    FLOAT view[16], proj[16];
    GetViewMatrix(view);
    GetProjectionMatrix(proj);
    
    // 矩阵乘法: result = proj * view
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            matrix[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                matrix[i * 4 + j] += proj[i * 4 + k] * view[k * 4 + j];
            }
        }
    }
}

/**
 * @brief 更新摄像机
 */
void CCamera::Update(FLOAT deltaTime)
{
    // 根据模式更新摄像机位置
    switch (m_Mode)
    {
    case CameraMode::FirstPerson:
        UpdateFirstPerson();
        break;
    case CameraMode::ThirdPerson:
        UpdateThirdPerson();
        break;
    // case CameraMode::FreeLook:
    //     UpdateFreeLook();
    //     break;
    // case CameraMode::Orbital:
    //     UpdateOrbital();
    //     break;
    }
    
    // 更新摄像机震动
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
}

/**
 * @brief 应用摄像机视图矩阵
 */
void CCamera::ApplyViewMatrix() const
{
    FLOAT viewMatrix[16];
    GetViewMatrix(viewMatrix);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(viewMatrix);
}

/**
 * @brief 应用摄像机投影矩阵
 */
void CCamera::ApplyProjectionMatrix() const
{
    FLOAT projMatrix[16];
    GetProjectionMatrix(projMatrix);
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projMatrix);
}

/**
 * @brief 重置摄像机
 */
void CCamera::Reset()
{
    m_Position = Vector3(0.0f, 0.0f, 0.0f);
    m_Target = Vector3(0.0f, 0.0f, -1.0f);
    m_Up = Vector3(0.0f, 1.0f, 0.0f);
    m_WorldUp = Vector3(0.0f, 1.0f, 0.0f);
    m_Forward = Vector3(0.0f, 0.0f, -1.0f);
    m_Right = Vector3(1.0f, 0.0f, 0.0f);
    
    m_Yaw = -90.0f;
    m_Pitch = 0.0f;
    m_Roll = 0.0f;
    
    m_MoveSpeed = 5.0f;
    m_RotationSpeed = 1.0f;
    m_ZoomSpeed = 2.0f;
    
    m_ThirdPersonTarget = Vector3(0.0f, 0.0f, 0.0f);
    m_Distance = 5.0f;
    m_Height = 2.0f;
    m_AngleAroundTarget = 0.0f;
    
    m_OrbitCenter = Vector3(0.0f, 0.0f, 0.0f);
    m_OrbitDistance = 10.0f;
    m_OrbitPhi = 45.0f;
    m_OrbitTheta = 0.0f;
    
    m_Fov = 45.0f;
    m_AspectRatio = 4.0f / 3.0f;
    m_NearPlane = 0.1f;
    m_FarPlane = 1000.0f;
    
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
    
    StopShake();
}