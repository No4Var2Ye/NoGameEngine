#include "stdafx.h"
#include "Math/Quaternion.h"
#include <cmath>
#include <sstream>

// 构造函数
Quaternion::Quaternion() : x(0), y(0), z(0), w(1) {}

Quaternion::Quaternion(float _x, float _y, float _z, float _w) 
    : x(_x), y(_y), z(_z), w(_w) {}

Quaternion::Quaternion(const Vector3& axis, float angle)
{
    float halfAngle = angle * 0.5f;
    float sinHalf = sinf(halfAngle);
    float cosHalf = cosf(halfAngle);
    
    Vector3 normalizedAxis = axis.Normalized();
    x = normalizedAxis.x * sinHalf;
    y = normalizedAxis.y * sinHalf;
    z = normalizedAxis.z * sinHalf;
    w = cosHalf;
}

Quaternion::Quaternion(float pitch, float yaw, float roll)
{
    // 转换为四元数（YXZ顺序，与通常的欧拉角顺序一致）
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);
    
    w = cy * cp * cr + sy * sp * sr;
    x = cy * sp * cr + sy * cp * sr;
    y = sy * cp * cr - cy * sp * sr;
    z = cy * cp * sr - sy * sp * cr;
}

// 运算
Quaternion Quaternion::operator+(const Quaternion& other) const
{
    return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
}

Quaternion Quaternion::operator-(const Quaternion& other) const
{
    return Quaternion(x - other.x, y - other.y, z - other.z, w - other.w);
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
    return Quaternion(
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y + y * other.w + z * other.x - x * other.z,
        w * other.z + z * other.w + x * other.y - y * other.x,
        w * other.w - x * other.x - y * other.y - z * other.z
    );
}

Quaternion Quaternion::operator*(float scalar) const
{
    return Quaternion(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector3 Quaternion::operator*(const Vector3& vec) const
{
    // 使用四元数旋转向量
    Quaternion v(vec.x, vec.y, vec.z, 0.0f);
    Quaternion conjugate = Conjugate();
    Quaternion result = (*this) * v * conjugate;
    return Vector3(result.x, result.y, result.z);
}

// 共轭/逆
Quaternion Quaternion::Conjugate() const
{
    return Quaternion(-x, -y, -z, w);
}

Quaternion Quaternion::Inverse() const
{
    float lenSq = LengthSquared();
    if (Math::IsZero(lenSq))
        return Identity();
    
    float invLenSq = 1.0f / lenSq;
    return Conjugate() * invLenSq;
}

// 归一化
Quaternion Quaternion::Normalized() const
{
    float len = Length();
    if (len > Math::EPSILON)
    {
        float invLen = 1.0f / len;
        return Quaternion(x * invLen, y * invLen, z * invLen, w * invLen);
    }
    return Identity();
}

Quaternion& Quaternion::Normalize()
{
    float len = Length();
    if (len > Math::EPSILON)
    {
        float invLen = 1.0f / len;
        x *= invLen;
        y *= invLen;
        z *= invLen;
        w *= invLen;
    }
    else
    {
        *this = Identity();
    }
    return *this;
}

// 长度
float Quaternion::Length() const
{
    return sqrtf(LengthSquared());
}

float Quaternion::LengthSquared() const
{
    return x * x + y * y + z * z + w * w;
}

// 点积
float Quaternion::Dot(const Quaternion& other) const
{
    return x * other.x + y * other.y + z * other.z + w * other.w;
}

// 插值
Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t)
{
    t = Math::Clamp(t, 0.0f, 1.0f);
    
    float cosHalfTheta = q1.Dot(q2);
    
    // 如果点积为负，四元数取反以获得最短路径
    Quaternion q2_temp = q2;
    if (cosHalfTheta < 0.0f)
    {
        q2_temp = q2_temp * -1.0f;
        cosHalfTheta = -cosHalfTheta;
    }
    
    // 如果角度很小，使用线性插值
    if (cosHalfTheta > 0.9999f)
    {
        return Nlerp(q1, q2_temp, t);
    }
    
    float halfTheta = acosf(cosHalfTheta);
    float sinHalfTheta = sqrtf(1.0f - cosHalfTheta * cosHalfTheta);
    
    float ratioA = sinf((1.0f - t) * halfTheta) / sinHalfTheta;
    float ratioB = sinf(t * halfTheta) / sinHalfTheta;
    
    return q1 * ratioA + q2_temp * ratioB;
}

Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t)
{
    t = Math::Clamp(t, 0.0f, 1.0f);
    return q1 + (q2 - q1) * t;
}

Quaternion Quaternion::Nlerp(const Quaternion& q1, const Quaternion& q2, float t)
{
    return Lerp(q1, q2, t).Normalized();
}

// 创建旋转
Quaternion Quaternion::FromEuler(float pitch, float yaw, float roll)
{
    return Quaternion(pitch, yaw, roll);
}

Quaternion Quaternion::FromAxisAngle(const Vector3& axis, float angle)
{
    return Quaternion(axis, angle);
}

Quaternion Quaternion::FromMatrix(const Matrix4& matrix)
{
    float trace = matrix.m00 + matrix.m11 + matrix.m22;
    
    if (trace > 0)
    {
        float s = 0.5f / sqrtf(trace + 1.0f);
        return Quaternion(
            (matrix.m21 - matrix.m12) * s,
            (matrix.m02 - matrix.m20) * s,
            (matrix.m10 - matrix.m01) * s,
            0.25f / s
        );
    }
    else if (matrix.m00 > matrix.m11 && matrix.m00 > matrix.m22)
    {
        float s = 2.0f * sqrtf(1.0f + matrix.m00 - matrix.m11 - matrix.m22);
        float invS = 1.0f / s;
        return Quaternion(
            0.25f * s,
            (matrix.m01 + matrix.m10) * invS,
            (matrix.m02 + matrix.m20) * invS,
            (matrix.m21 - matrix.m12) * invS
        );
    }
    else if (matrix.m11 > matrix.m22)
    {
        float s = 2.0f * sqrtf(1.0f + matrix.m11 - matrix.m00 - matrix.m22);
        float invS = 1.0f / s;
        return Quaternion(
            (matrix.m01 + matrix.m10) * invS,
            0.25f * s,
            (matrix.m12 + matrix.m21) * invS,
            (matrix.m02 - matrix.m20) * invS
        );
    }
    else
    {
        float s = 2.0f * sqrtf(1.0f + matrix.m22 - matrix.m00 - matrix.m11);
        float invS = 1.0f / s;
        return Quaternion(
            (matrix.m02 + matrix.m20) * invS,
            (matrix.m12 + matrix.m21) * invS,
            0.25f * s,
            (matrix.m10 - matrix.m01) * invS
        );
    }
}

// 转换为其他表示
Vector3 Quaternion::ToEuler() const
{
    // 转换为欧拉角（YXZ顺序）
    float sinr_cosp = 2.0f * (w * x + y * z);
    float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
    float roll = atan2f(sinr_cosp, cosr_cosp);
    
    float sinp = 2.0f * (w * y - z * x);
    float pitch;
    if (fabsf(sinp) >= 1.0f)
        pitch = copysignf(Math::PI / 2.0f, sinp);
    else
        pitch = asinf(sinp);
    
    float siny_cosp = 2.0f * (w * z + x * y);
    float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
    float yaw = atan2f(siny_cosp, cosy_cosp);
    
    return Vector3(pitch, yaw, roll);
}

Matrix4 Quaternion::ToMatrix() const
{
    return Matrix4::Rotation(*this);
}

// 常用四元数
Quaternion Quaternion::Identity()
{
    return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);
}

Quaternion Quaternion::Zero()
{
    return Quaternion(0.0f, 0.0f, 0.0f, 0.0f);
}

// 方向
Vector3 Quaternion::Forward(const Quaternion& q)
{
    return Vector3(
        2.0f * (q.x * q.z + q.w * q.y),
        2.0f * (q.y * q.z - q.w * q.x),
        1.0f - 2.0f * (q.x * q.x + q.y * q.y)
    );
}

Vector3 Quaternion::Right(const Quaternion& q)
{
    return Vector3(
        1.0f - 2.0f * (q.y * q.y + q.z * q.z),
        2.0f * (q.x * q.y + q.w * q.z),
        2.0f * (q.x * q.z - q.w * q.y)
    );
}

Vector3 Quaternion::Up(const Quaternion& q)
{
    return Vector3(
        2.0f * (q.x * q.y - q.w * q.z),
        1.0f - 2.0f * (q.x * q.x + q.z * q.z),
        2.0f * (q.y * q.z + q.w * q.x)
    );
}