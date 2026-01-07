// ======================================================================
#ifndef __VECTOR4_H__
#define __VECTOR4_H__

// 防止Windows宏污染
#ifdef RGB
#undef RGB
#endif
// ======================================================================

#include "Math/Vector3.h"
#include <iostream>
// ======================================================================


class Vector4
{
public:
    float x, y, z, w;
    
    // 构造函数
    Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vector4(const Vector3& vec3, float w = 0.0f) : x(vec3.x), y(vec3.y), z(vec3.z), w(w) {}
    explicit Vector4(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}
    
    // 访问运算符
    float& operator[](int index) { 
        return (&x)[index]; 
    }
    
    const float& operator[](int index) const { 
        return (&x)[index]; 
    }
    
    // 向量运算
    Vector4 operator-() const { 
        return Vector4(-x, -y, -z, -w); 
    }
    
    Vector4 operator+(const Vector4& other) const { 
        return Vector4(x + other.x, y + other.y, z + other.z, w + other.w); 
    }
    
    Vector4 operator-(const Vector4& other) const { 
        return Vector4(x - other.x, y - other.y, z - other.z, w - other.w); 
    }
    
    Vector4 operator*(const Vector4& other) const { 
        return Vector4(x * other.x, y * other.y, z * other.z, w * other.w); 
    }
    
    Vector4 operator/(const Vector4& other) const { 
        return Vector4(x / other.x, y / other.y, z / other.z, w / other.w); 
    }
    
    Vector4 operator*(float scalar) const { 
        return Vector4(x * scalar, y * scalar, z * scalar, w * scalar); 
    }
    
    Vector4 operator/(float scalar) const { 
        float invScalar = 1.0f / scalar;
        return Vector4(x * invScalar, y * invScalar, z * invScalar, w * invScalar);
    }
    
    // 赋值运算
    Vector4& operator+=(const Vector4& other) { 
        x += other.x; y += other.y; z += other.z; w += other.w; return *this; 
    }
    
    Vector4& operator-=(const Vector4& other) { 
        x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; 
    }
    
    Vector4& operator*=(const Vector4& other) { 
        x *= other.x; y *= other.y; z *= other.z; w *= other.w; return *this; 
    }
    
    Vector4& operator/=(const Vector4& other) { 
        x /= other.x; y /= other.y; z /= other.z; w /= other.w; return *this; 
    }
    
    Vector4& operator*=(float scalar) { 
        x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; 
    }
    
    Vector4& operator/=(float scalar) { 
        float invScalar = 1.0f / scalar;
        x *= invScalar; y *= invScalar; z *= invScalar; w *= invScalar;
        return *this;
    }
    
    // 比较运算符
    bool operator==(const Vector4& other) const { 
        return Math::FloatEqual(x, other.x) && 
               Math::FloatEqual(y, other.y) && 
               Math::FloatEqual(z, other.z) && 
               Math::FloatEqual(w, other.w); 
    }
    
    bool operator!=(const Vector4& other) const { 
        return !(*this == other); 
    }
    
    // 向量操作
    float Dot(const Vector4& other) const { 
        return x * other.x + y * other.y + z * other.z + w * other.w; 
    }
    
    float LengthSquared() const { 
        return x * x + y * y + z * z + w * w; 
    }
    
    float Length() const { 
        return Math::Sqrt(LengthSquared()); 
    }
    
    float Distance(const Vector4& other) const { 
        return (*this - other).Length(); 
    }
    
    float DistanceSquared(const Vector4& other) const { 
        return (*this - other).LengthSquared(); 
    }
    
    Vector4 Normalized() const { 
        float len = Length();
        if (len > Math::EPSILON) {
            float invLen = 1.0f / len;
            return Vector4(x * invLen, y * invLen, z * invLen, w * invLen);
        }
        return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    
    Vector4& Normalize() { 
        float len = Length();
        if (len > Math::EPSILON) {
            float invLen = 1.0f / len;
            x *= invLen; y *= invLen; z *= invLen; w *= invLen;
        }
        return *this;
    }
    
    bool IsZero() const { 
        return Math::IsZero(x) && Math::IsZero(y) && 
               Math::IsZero(z) && Math::IsZero(w); 
    }
    
    bool IsNormalized() const { 
        return Math::FloatEqual(LengthSquared(), 1.0f); 
    }
    
    // 转换为Vector3
    Vector3 XYZ() const { 
        return Vector3(x, y, z); 
    }
    
    Vector3 RGB() const { 
        return Vector3(x, y, z); 
    }
    
    // 作为颜色处理
    unsigned int ToRGBA() const {
        unsigned int r = static_cast<unsigned int>(Math::Clamp(x, 0.0f, 1.0f) * 255.0f);
        unsigned int g = static_cast<unsigned int>(Math::Clamp(y, 0.0f, 1.0f) * 255.0f);
        unsigned int b = static_cast<unsigned int>(Math::Clamp(z, 0.0f, 1.0f) * 255.0f);
        unsigned int a = static_cast<unsigned int>(Math::Clamp(w, 0.0f, 1.0f) * 255.0f);
        return (r << 24) | (g << 16) | (b << 8) | a;
    }
    
    static Vector4 FromRGBA(unsigned int color) {
        float r = ((color >> 24) & 0xFF) / 255.0f;
        float g = ((color >> 16) & 0xFF) / 255.0f;
        float b = ((color >> 8) & 0xFF) / 255.0f;
        float a = (color & 0xFF) / 255.0f;
        return Vector4(r, g, b, a);
    }
    
    // 齐次坐标变换
    Vector3 Homogenize() const { 
        if (Math::IsZero(w)) return Vector3(x, y, z);
        return Vector3(x / w, y / w, z / w);
    }
    
    Vector4 Homogeneous() const { 
        if (Math::IsZero(w)) return *this;
        return Vector4(x / w, y / w, z / w, 1.0f);
    }
    
    // 静态方法
    static Vector4 Zero() { 
        return Vector4(0.0f, 0.0f, 0.0f, 0.0f); 
    }
    
    static Vector4 One() { 
        return Vector4(1.0f, 1.0f, 1.0f, 1.0f); 
    }
    
    static Vector4 UnitX() { 
        return Vector4(1.0f, 0.0f, 0.0f, 0.0f); 
    }
    
    static Vector4 UnitY() { 
        return Vector4(0.0f, 1.0f, 0.0f, 0.0f); 
    }
    
    static Vector4 UnitZ() { 
        return Vector4(0.0f, 0.0f, 1.0f, 0.0f); 
    }
    
    static Vector4 UnitW() { 
        return Vector4(0.0f, 0.0f, 0.0f, 1.0f); 
    }
    
    static float Dot(const Vector4& a, const Vector4& b) { 
        return a.Dot(b); 
    }
    
    static float Distance(const Vector4& a, const Vector4& b) { 
        return a.Distance(b); 
    }
    
    static Vector4 Lerp(const Vector4& a, const Vector4& b, float t) { 
        return a + (b - a) * Math::Clamp(t, 0.0f, 1.0f); 
    }
    
    // 流输出
    friend std::ostream& operator<<(std::ostream& os, const Vector4& vec) {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
        return os;
    }
};

// 标量乘法（左乘）
inline Vector4 operator*(float scalar, const Vector4& vec) { 
    return vec * scalar; 
}

#endif // __VECTOR4_H__