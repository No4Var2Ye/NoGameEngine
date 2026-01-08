// ======================================================================
/**
 * @file Math/Vector2.h
 *
 * @brief 二维向量
 */
// ======================================================================
#ifndef __VECTOR2_H__
#define __VECTOR2_H__
// ======================================================================

#include "Math/MathUtils.h"
#include <iostream>
// ======================================================================

class Vector2
{
public:
    float x, y;

    // 构造函数
    Vector2() : x(0.0f), y(0.0f) {}
    Vector2(float x, float y) : x(x), y(y) {}
    explicit Vector2(float scalar) : x(scalar), y(scalar) {}

    // 拷贝构造函数
    Vector2(const Vector2 &other) = default;

    // 赋值运算符
    Vector2 &operator=(const Vector2 &other) = default;

    // 访问运算符
    float &operator[](int index)
    {
        return (&x)[index];
    }

    const float &operator[](int index) const
    {
        return (&x)[index];
    }

    // 向量运算
    Vector2 operator-() const
    {
        return Vector2(-x, -y);
    }

    Vector2 operator+(const Vector2 &other) const
    {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2 &other) const
    {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator*(const Vector2 &other) const
    {
        return Vector2(x * other.x, y * other.y);
    }

    Vector2 operator/(const Vector2 &other) const
    {
        return Vector2(x / other.x, y / other.y);
    }

    Vector2 operator*(float scalar) const
    {
        return Vector2(x * scalar, y * scalar);
    }

    Vector2 operator/(float scalar) const
    {
        float invScalar = 1.0f / scalar;
        return Vector2(x * invScalar, y * invScalar);
    }

    // 赋值运算
    Vector2 &operator+=(const Vector2 &other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vector2 &operator-=(const Vector2 &other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vector2 &operator*=(const Vector2 &other)
    {
        x *= other.x;
        y *= other.y;
        return *this;
    }

    Vector2 &operator/=(const Vector2 &other)
    {
        x /= other.x;
        y /= other.y;
        return *this;
    }

    Vector2 &operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vector2 &operator/=(float scalar)
    {
        float invScalar = 1.0f / scalar;
        x *= invScalar;
        y *= invScalar;
        return *this;
    }

    // 比较运算符
    bool operator==(const Vector2 &other) const
    {
        return Math::FloatEqual(x, other.x) && Math::FloatEqual(y, other.y);
    }

    bool operator!=(const Vector2 &other) const
    {
        return !(*this == other);
    }

    // 向量操作
    float Dot(const Vector2 &other) const
    {
        return x * other.x + y * other.y;
    }

    float Cross(const Vector2 &other) const
    {
        return x * other.y - y * other.x;
    }

    float LengthSquared() const
    {
        return x * x + y * y;
    }

    float Length() const
    {
        return Math::Sqrt(LengthSquared());
    }

    float Distance(const Vector2 &other) const
    {
        return (*this - other).Length();
    }

    float DistanceSquared(const Vector2 &other) const
    {
        return (*this - other).LengthSquared();
    }

    // 单位化
    Vector2 Normalized() const
    {
        float len = Length();
        if (len > Math::EPSILON)
        {
            float invLen = 1.0f / len;
            return Vector2(x * invLen, y * invLen);
        }
        return Vector2(0.0f, 0.0f);
    }

    Vector2 &Normalize()
    {
        float len = Length();
        if (len > Math::EPSILON)
        {
            float invLen = 1.0f / len;
            x *= invLen;
            y *= invLen;
        }
        return *this;
    }

    float GetAngle() const
    {
        return Math::Atan2(y, x);
    }

    Vector2 Rotate(float angle) const
    {
        float cosA = Math::Cos(angle);
        float sinA = Math::Sin(angle);
        return Vector2(x * cosA - y * sinA, x * sinA + y * cosA);
    }

    // 是否为零向量
    bool IsZero() const
    {
        return Math::IsZero(x) && Math::IsZero(y);
    }

    // 是否为单位向量
    bool IsNormalized() const
    {
        return Math::FloatEqual(LengthSquared(), 1.0f);
    }

    // 静态方法
    static Vector2 Zero()
    {
        return Vector2(0.0f, 0.0f);
    }

    static Vector2 One()
    {
        return Vector2(1.0f, 1.0f);
    }

    static Vector2 UnitX()
    {
        return Vector2(1.0f, 0.0f);
    }

    static Vector2 UnitY()
    {
        return Vector2(0.0f, 1.0f);
    }

    static float Dot(const Vector2 &a, const Vector2 &b)
    {
        return a.Dot(b);
    }

    static float Cross(const Vector2 &a, const Vector2 &b)
    {
        return a.Cross(b);
    }

    static float Distance(const Vector2 &a, const Vector2 &b)
    {
        return a.Distance(b);
    }

    // 线性插值
    static Vector2 Lerp(const Vector2 &a, const Vector2 &b, float t)
    {
        return a + (b - a) * Math::Clamp(t, 0.0f, 1.0f);
    }

    // 反射向量
    static Vector2 Reflect(const Vector2 &direction, const Vector2 &normal)
    {
        return direction - normal * 2.0f * direction.Dot(normal);
    }

    // 投影向量
    static Vector2 Project(const Vector2 &vector, const Vector2 &onto)
    {
        float lenSq = onto.LengthSquared();
        if (lenSq < Math::EPSILON)
            return Vector2::Zero();
        return onto * (vector.Dot(onto) / lenSq);
    }

    // 流输出
    friend std::ostream &operator<<(std::ostream &os, const Vector2 &vec)
    {
        os << "(" << vec.x << ", " << vec.y << ")";
        return os;
    }
};

// 标量乘法（左乘）
inline Vector2 operator*(float scalar, const Vector2 &vec)
{
    return vec * scalar;
}

#endif // VECTOR2_H