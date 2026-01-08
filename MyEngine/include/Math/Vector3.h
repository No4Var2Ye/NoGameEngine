// ======================================================================
#ifndef __VECTOR3_H__
#define __VECTOR3_H__
// ======================================================================

#include "Math/Vector2.h"
#include <iostream>
// ======================================================================

class Vector3
{
public:
    float x, y, z;

    // 构造函数
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vector3(const Vector2 &vec2, float z = 0.0f) : x(vec2.x), y(vec2.y), z(z) {}
    explicit Vector3(float scalar) : x(scalar), y(scalar), z(scalar) {}

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
    Vector3 operator-() const
    {
        return Vector3(-x, -y, -z);
    }

    Vector3 operator+(const Vector3 &other) const
    {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3 &other) const
    {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(const Vector3 &other) const
    {
        return Vector3(x * other.x, y * other.y, z * other.z);
    }

    Vector3 operator/(const Vector3 &other) const
    {
        return Vector3(x / other.x, y / other.y, z / other.z);
    }

    Vector3 operator*(float scalar) const
    {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3 operator/(float scalar) const
    {
        float invScalar = 1.0f / scalar;
        return Vector3(x * invScalar, y * invScalar, z * invScalar);
    }

    // 赋值运算
    Vector3 &operator+=(const Vector3 &other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vector3 &operator-=(const Vector3 &other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vector3 &operator*=(const Vector3 &other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        return *this;
    }

    Vector3 &operator/=(const Vector3 &other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        return *this;
    }

    Vector3 &operator*=(float scalar)
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vector3 &operator/=(float scalar)
    {
        float invScalar = 1.0f / scalar;
        x *= invScalar;
        y *= invScalar;
        z *= invScalar;
        return *this;
    }

    // 比较运算符
    bool operator==(const Vector3 &other) const
    {
        return Math::FloatEqual(x, other.x) &&
               Math::FloatEqual(y, other.y) &&
               Math::FloatEqual(z, other.z);
    }

    bool operator!=(const Vector3 &other) const
    {
        return !(*this == other);
    }

    // 向量操作
    float Dot(const Vector3 &other) const
    {
        return x * other.x + y * other.y + z * other.z;
    }

    Vector3 Cross(const Vector3 &other) const
    {
        return Vector3(y * other.z - z * other.y,
                       z * other.x - x * other.z,
                       x * other.y - y * other.x);
    }

    float LengthSquared() const
    {
        return x * x + y * y + z * z;
    }

    float Length() const
    {
        return Math::Sqrt(LengthSquared());
    }

    float Distance(const Vector3 &other) const
    {
        return (*this - other).Length();
    }

    float DistanceSquared(const Vector3 &other) const
    {
        return (*this - other).LengthSquared();
    }

    Vector3 Normalized() const
    {
        float len = Length();
        if (len > Math::EPSILON)
        {
            float invLen = 1.0f / len;
            return Vector3(x * invLen, y * invLen, z * invLen);
        }
        return Vector3(0.0f, 0.0f, 0.0f);
    }

    Vector3 &Normalize()
    {
        float len = Length();
        if (len > Math::EPSILON)
        {
            float invLen = 1.0f / len;
            x *= invLen;
            y *= invLen;
            z *= invLen;
        }
        return *this;
    }

    Vector3 Reflect(const Vector3 &normal) const
    {
        return *this - normal * 2.0f * this->Dot(normal);
    }

    Vector3 Project(const Vector3 &onto) const
    {
        float lenSq = onto.LengthSquared();
        if (lenSq < Math::EPSILON)
            return Vector3::Zero();
        return onto * (this->Dot(onto) / lenSq);
    }

    float GetAngle(const Vector3 &other) const
    {
        float dot = Dot(other);
        float lenProduct = Length() * other.Length();
        if (lenProduct < Math::EPSILON)
            return 0.0f;
        return Math::Acos(Math::Clamp(dot / lenProduct, -1.0f, 1.0f));
    }

    bool IsZero() const
    {
        return Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z);
    }

    bool IsNormalized() const
    {
        return Math::FloatEqual(LengthSquared(), 1.0f);
    }

    // 转换为Vector2
    Vector2 XY() const
    {
        return Vector2(x, y);
    }

    Vector2 XZ() const
    {
        return Vector2(x, z);
    }

    Vector2 YZ() const
    {
        return Vector2(y, z);
    }

    // 静态方法
    static Vector3 Zero()
    {
        return Vector3(0.0f, 0.0f, 0.0f);
    }

    static Vector3 One()
    {
        return Vector3(1.0f, 1.0f, 1.0f);
    }

    static Vector3 UnitX()
    {
        return Vector3(1.0f, 0.0f, 0.0f);
    }

    static Vector3 UnitY()
    {
        return Vector3(0.0f, 1.0f, 0.0f);
    }

    static Vector3 UnitZ()
    {
        return Vector3(0.0f, 0.0f, 1.0f);
    }

    static Vector3 Up()
    {
        return Vector3(0.0f, 1.0f, 0.0f);
    }

    static Vector3 Down()
    {
        return Vector3(0.0f, -1.0f, 0.0f);
    }

    static Vector3 Right()
    {
        return Vector3(1.0f, 0.0f, 0.0f);
    }

    static Vector3 Left()
    {
        return Vector3(-1.0f, 0.0f, 0.0f);
    }

    static Vector3 Forward()
    {
        return Vector3(0.0f, 0.0f, 1.0f);
    }

    static Vector3 Backward()
    {
        return Vector3(0.0f, 0.0f, -1.0f);
    }

    static float Dot(const Vector3 &a, const Vector3 &b)
    {
        return a.Dot(b);
    }

    static Vector3 Cross(const Vector3 &a, const Vector3 &b)
    {
        return a.Cross(b);
    }

    static float Distance(const Vector3 &a, const Vector3 &b)
    {
        return a.Distance(b);
    }

    static Vector3 Lerp(const Vector3 &a, const Vector3 &b, float t)
    {
        return a + (b - a) * Math::Clamp(t, 0.0f, 1.0f);
    }

    static Vector3 Slerp(const Vector3 &a, const Vector3 &b, float t)
    {
        float dot = Math::Clamp(Dot(a, b), -1.0f, 1.0f);
        float theta = Math::Acos(dot) * t;
        Vector3 relative = b - a * dot;
        relative.Normalize();
        return a * Math::Cos(theta) + relative * Math::Sin(theta);
    }

    static Vector3 Min(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(Math::Min(a.x, b.x), Math::Min(a.y, b.y), Math::Min(a.z, b.z));
    }

    static Vector3 Max(const Vector3 &a, const Vector3 &b)
    {
        return Vector3(Math::Max(a.x, b.x), Math::Max(a.y, b.y), Math::Max(a.z, b.z));
    }

    // 流输出
    friend std::ostream &operator<<(std::ostream &os, const Vector3 &vec)
    {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }
};

// 标量乘法（左乘）
inline Vector3 operator*(float scalar, const Vector3 &vec)
{
    return vec * scalar;
}

#endif // __VECTOR3_H__