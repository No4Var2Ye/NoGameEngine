// ======================================================================
#ifndef __QUATERNION_H__
#define __QUATERNION_H__
// ======================================================================

#include "Math/Vector3.h"
#include "Math/Matrix4.h"
#include <cmath>
#include <string>
// ======================================================================

class Matrix4;

class Quaternion
{
public:
    float x, y, z, w;

    // 构造函数
    Quaternion();
    Quaternion(float x, float y, float z, float w);
    Quaternion(const Vector3 &axis, float angle);
    Quaternion(float pitch, float yaw, float roll);

    // 运算
    Quaternion operator+(const Quaternion &other) const;
    Quaternion operator-(const Quaternion &other) const;
    Quaternion operator*(const Quaternion &other) const;
    Quaternion operator*(float scalar) const;
    Vector3 operator*(const Vector3 &vec) const;

    // 共轭/逆
    Quaternion Conjugate() const;
    Quaternion Inverse() const;

    // 归一化
    Quaternion Normalized() const;
    Quaternion &Normalize();

    // 长度
    float Length() const;
    float LengthSquared() const;

    // 点积
    float Dot(const Quaternion &other) const;

    // 插值
    static Quaternion Slerp(const Quaternion &q1, const Quaternion &q2, float t);
    static Quaternion Lerp(const Quaternion &q1, const Quaternion &q2, float t);
    static Quaternion Nlerp(const Quaternion &q1, const Quaternion &q2, float t);

    // 创建旋转
    static Quaternion FromEuler(float pitch, float yaw, float roll);
    static Quaternion FromAxisAngle(const Vector3 &axis, float angle);
    static Quaternion FromMatrix(const Matrix4 &matrix);

    // 转换为其他表示
    Vector3 ToEuler() const;
    Matrix4 ToMatrix() const;

    // 常用四元数
    static Quaternion Identity();
    static Quaternion Zero();

    // 方向
    static Vector3 Forward(const Quaternion &q);
    static Vector3 Right(const Quaternion &q);
    static Vector3 Up(const Quaternion &q);
};

#endif // __QUATERNION_H__