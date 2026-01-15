
// ======================================================================
#ifndef __CAMERA_H__
#define __CAMERA_H__
// ======================================================================

// ======================================================================
#include <windows.h>
#include <GL/gl.h>
// ======================================================================

// 摄像机模式枚举
enum class CameraMode
{
    FirstPerson, // 第一人称
    ThirdPerson, // 第三人称
    FreeLook,    // 自由视角
    Orbital      // 轨道视角
};

// ======================================================================
class Vector3;
class Matrix4;

// ======================================================================
class CCamera
{
private:
    // 摄像机位置和方向
    Vector3 m_Position; // 摄像机位置
    Vector3 m_Target;   // 观察目标
    Vector3 m_Up;       // 上方向

    // 摄像机轴
    Vector3 m_Forward; // 前方向
    Vector3 m_Right;   // 右方向
    Vector3 m_WorldUp; // 世界上方向

    // 欧拉角
    FLOAT m_Yaw;   // 偏航角（绕Y轴旋转）
    FLOAT m_Pitch; // 俯仰角（绕X轴旋转）
    FLOAT m_Roll;  // 翻滚角（绕Z轴旋转）

    CameraMode m_Mode; // 摄像机模式

    // 第一人称/自由视角参数
    FLOAT m_MoveSpeed;     // 移动速度
    FLOAT m_RotationSpeed; // 旋转速度
    FLOAT m_ZoomSpeed;     // 缩放速度

    // 第三人称参数
    Vector3 m_ThirdPersonTarget; // 第三人称跟踪目标
    FLOAT m_Distance;            // 距离目标的距离
    FLOAT m_Height;              // 相对高度
    FLOAT m_AngleAroundTarget;   // 围绕目标的角度

    // 轨道视角参数
    Vector3 m_OrbitCenter; // 轨道中心
    FLOAT m_OrbitDistance; // 轨道距离
    FLOAT m_OrbitPhi;      // 垂直角度
    FLOAT m_OrbitTheta;    // 水平角度

    // 平滑插值
    FLOAT m_TargetDistance; // 目标距离
    FLOAT m_TargetFov;      // 目标FOV

    // 投影参数
    FLOAT m_Fov;         // 视野角度
    FLOAT m_AspectRatio; // 宽高比
    FLOAT m_NearPlane;   // 近裁剪面
    FLOAT m_FarPlane;    // 远裁剪面

    // 输入控制
    BOOL m_EnableMouseLook;   // 是否启用鼠标视角控制
    POINT m_LastMousePos;     // 上一帧鼠标位置
    BOOL m_MouseLookActive;   // 鼠标视角是否激活
    FLOAT m_MouseSensitivity; // 鼠标灵敏度

    // 边界限制
    BOOL m_ClampPitch;    // 是否限制俯仰角
    FLOAT m_MinPitch;     // 最小俯仰角
    FLOAT m_MaxPitch;     // 最大俯仰角
    BOOL m_ClampDistance; // 是否限制距离
    FLOAT m_MinDistance;  // 最小距离
    FLOAT m_MaxDistance;  // 最大距离

    // 摄像机震动
    BOOL m_ShakeEnabled;    // 是否启用震动
    FLOAT m_ShakeIntensity; // 震动强度
    FLOAT m_ShakeDuration;  // 震动持续时间
    FLOAT m_ShakeTimer;     // 震动计时器
    Vector3 m_ShakeOffset;  // 震动偏移

    FLOAT m_FollowSpeed = 10.0f;  // 跟踪平滑度
    mutable Matrix4 m_CachedView; // 缓存矩阵
    mutable Matrix4 m_CachedProj;
    mutable BOOL m_ViewDirty = true;
    mutable BOOL m_ProjDirty = true;

    void UpdateCameraVectors(); // 更新摄像机轴向量

    void UpdateFirstPerson();                // 更新第一人称摄像机
    void UpdateThirdPerson(FLOAT deltaTime); // 更新第三人称摄像机
    void UpdateFreeLook();                   // 更新自由视角摄像机
    void UpdateOrbital();                    // 更新轨道视角摄像机
    void GenerateShakeOffset();              // 生成随机震动偏移

    /**
     * @brief 计算观察矩阵
     * @param position 摄像机位置
     * @param target 观察目标
     * @param up 上方向
     * @return 观察矩阵
     */
    static void CalculateLookAtMatrix(const Vector3 &position, const Vector3 &target, const Vector3 &up, Matrix4 &matrix);

public:
    CCamera();
    ~CCamera() = default;

    // 禁止拷贝
    CCamera(const CCamera &) = delete;
    CCamera &operator=(const CCamera &) = delete;

    // ======================================================================
    // 初始化
    // ======================================================================

    /**
     * @brief 初始化摄像机
     * @param position 初始位置
     * @param target 初始观察目标
     * @param up 初始上方向
     */
    void Initialize(const Vector3 &position, const Vector3 &target, const Vector3 &up);

    /**
     * @brief 设置投影参数
     * @param fov 视野角度
     * @param aspectRatio 宽高比
     * @param nearPlane 近裁剪面
     * @param farPlane 远裁剪面
     */
    void SetProjection(FLOAT fov, FLOAT aspectRatio, FLOAT nearPlane, FLOAT farPlane);

    // ======================================================================
    // 摄像机控制
    // ======================================================================

    void SetMode(CameraMode mode);                // 设置摄像机模式
    CameraMode GetMode() const { return m_Mode; } // 获取摄像机模式

    /**
     * @brief 移动摄像机
     * @param forward 前后移动量
     * @param right 左右移动量
     * @param up 上下移动量
     */
    void Move(FLOAT forwardAmount, FLOAT rightAmount, FLOAT upAmount = 0.0f);

    /**
     * @brief 旋转摄像机
     * @param yaw 偏航角度
     * @param pitch 俯仰角度
     * @param roll 翻滚角度
     */
    void Rotate(FLOAT yaw, FLOAT pitch, FLOAT roll = 0.0f);

    void Zoom(FLOAT amount);                   // 缩放摄像机
    void SetPosition(const Vector3 &position); // 设置摄像机位置
    void SetTarget(const Vector3 &target);     // 设置观察目标
    void LookAt(const Vector3 &position);      // 看向指定位置

    /**
     * @brief 设置欧拉角
     * @param yaw 偏航角
     * @param pitch 俯仰角
     * @param roll 翻滚角
     */
    void SetEulerAngles(FLOAT yaw, FLOAT pitch, FLOAT roll = 0.0f);

    // ======================================================================
    // 第三人称控制
    // ======================================================================
    void SetThirdPersonTarget(const Vector3 &target); // 设置第三人称跟踪目标
    void SetThirdPersonDistance(FLOAT distance);      // 设置第三人称距离
    void SetThirdPersonHeight(FLOAT height);          // 设置第三人称高度
    void SetThirdPersonAngle(FLOAT angle);            // 设置第三人称角度

    // ======================================================================
    // 轨道视角控制
    // ======================================================================
    void SetOrbitCenter(const Vector3 &center); // 设置轨道中心
    void SetOrbitDistance(FLOAT distance);      // 设置轨道距离

    /**
     * @brief 设置轨道角度
     * @param phi 垂直角度
     * @param theta 水平角度
     */
    void SetOrbitAngles(FLOAT phi, FLOAT theta);

    // ======================================================================
    // 鼠标控制
    // ======================================================================

    /**
     * @brief 启用鼠标视角控制
     * @param enable 是否启用
     */
    void EnableMouseLook(BOOL enable);

    void StartMouseLook(); // 开始鼠标视角控制
    void StopMouseLook();  // 停止鼠标视角控制

    /**
     * @brief 处理鼠标移动
     * @param x 鼠标X坐标
     * @param y 鼠标Y坐标
     */
    void ProcessMouseMovement(INT dx, INT dy);

    /**
     * @brief 处理鼠标滚轮
     * @param delta 滚轮增量
     */
    void ProcessMouseWheel(INT delta);

    // ======================================================================
    // 摄像机震动
    // ======================================================================

    /**
     * @brief 启动摄像机震动
     * @param intensity 震动强度
     * @param duration 震动持续时间
     */
    void StartShake(FLOAT intensity = 0.1f, FLOAT duration = 0.5f);
    void StopShake(); // 停止摄像机震动

    // ======================================================================
    // 参数设置
    // ======================================================================

    void SetMoveSpeed(FLOAT speed) { m_MoveSpeed = speed; }                           // 设置移动速度
    void SetRotationSpeed(FLOAT speed) { m_RotationSpeed = speed; }                   // 设置旋转速度
    void SetZoomSpeed(FLOAT speed) { m_ZoomSpeed = speed; }                           // 设置缩放速度
    void SetMouseSensitivity(FLOAT sensitivity) { m_MouseSensitivity = sensitivity; } // 设置鼠标灵敏度

    /**
     * @brief 设置俯仰角限制
     * @param minPitch 最小俯仰角
     * @param maxPitch 最大俯仰角
     */
    void SetPitchLimits(FLOAT minPitch, FLOAT maxPitch);

    /**
     * @brief 设置距离限制
     * @param minDistance 最小距离
     * @param maxDistance 最大距离
     */
    void SetDistanceLimits(FLOAT minDistance, FLOAT maxDistance);

    // ======================================================================
    // 获取信息
    // ======================================================================

    Vector3 GetPosition() const { return m_Position; } // 获取摄像机位置
    Vector3 GetTarget() const { return m_Target; }     // 获取观察目标
    Vector3 GetForward() const { return m_Forward; }   // 获取前方向
    Vector3 GetRight() const { return m_Right; }       // 右方向
    Vector3 GetUp() const { return m_Up; }             // 获取上方向

    /**
     * @brief 获取欧拉角
     * @param yaw 输出偏航角
     * @param pitch 输出俯仰角
     * @param roll 输出翻滚角
     */
    void GetEulerAngles(FLOAT &yaw, FLOAT &pitch, FLOAT &roll) const;

    FLOAT GetFOV() const { return m_Fov; }                 // 获取视野角度
    FLOAT GetAspectRatio() const { return m_AspectRatio; } // 获取宽高比
    FLOAT GetNear() const {return m_NearPlane; } // 获取 近裁切面
    FLOAT GetFar() const {return m_FarPlane; } // 获取 远裁切面

    BOOL IsMouseLookActive() {return m_MouseLookActive; } // 鼠标视角是否激活

    // ======================================================================
    // 矩阵计算
    // ======================================================================

    void GetViewMatrix(Matrix4 &matrix) const;           // 获取观察矩阵
    void GetProjectionMatrix(Matrix4 &matrix) const;     // 获取投影矩阵
    void GetViewProjectionMatrix(Matrix4 &matrix) const; // 获取视图投影矩阵

    // ======================================================================
    // 更新和渲染
    // ======================================================================

    void Update(FLOAT deltaTime); // 更新摄像机
    void Reset();                 // 重置摄像机

    void ApplyViewMatrix() const;       // 应用摄像机视图矩阵
    void ApplyProjectionMatrix() const; // 应用摄像机投影矩阵
};

#endif // __CAMERA_H__