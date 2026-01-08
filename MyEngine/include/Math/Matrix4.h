// ======================================================================
#ifndef __MATRIX4_H__
#define __MATRIX4_H__
// ======================================================================

#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Quaternion.h"
#include <cmath>
// ======================================================================

class Quaternion;

class Matrix4
{
public:
    // 按列优先存储 (OpenGL风格)
    union
    {
        float m[16];
        float m4[4][4];
        struct
        {
            float m00, m10, m20, m30;
            float m01, m11, m21, m31;
            float m02, m12, m22, m32;
            float m03, m13, m23, m33;
        };
    };

    // 构造函数
    Matrix4();
    explicit Matrix4(float diagonal);
    Matrix4(float m00, float m01, float m02, float m03,
            float m10, float m11, float m12, float m13,
            float m20, float m21, float m22, float m23,
            float m30, float m31, float m32, float m33);
    Matrix4(const Matrix4 &other) = default;

    // 赋值运算符
    Matrix4 &operator=(const Matrix4 &other) = default;

    // 访问运算符
    float &operator()(int row, int col)
    {
        return m4[col][row]; // 列优先
    }

    const float &operator()(int row, int col) const
    {
        return m4[col][row];
    }

    // 矩阵运算
    Matrix4 operator+(const Matrix4 &other) const;
    Matrix4 operator-(const Matrix4 &other) const;
    Matrix4 operator*(const Matrix4 &other) const;
    Matrix4 operator*(float scalar) const;
    Vector4 operator*(const Vector4 &vec) const;
    Vector3 operator*(const Vector3 &vec) const;

    // 赋值运算
    Matrix4 &operator+=(const Matrix4 &other);
    Matrix4 &operator-=(const Matrix4 &other);
    Matrix4 &operator*=(const Matrix4 &other);
    Matrix4 &operator*=(float scalar);

    // 比较运算符
    bool operator==(const Matrix4 &other) const;
    bool operator!=(const Matrix4 &other) const;

    // 矩阵操作
    float Determinant() const;
    Matrix4 Transposed() const;
    Matrix4 &Transpose();
    Matrix4 Inversed() const;
    Matrix4 &Inverse();
    bool IsIdentity() const;
    bool IsZero() const;

    // 获取行/列
    Vector4 GetRow(int row) const;
    Vector4 GetColumn(int col) const;
    void SetRow(int row, const Vector4 &vec);
    void SetColumn(int col, const Vector4 &vec);

    // 变换矩阵
    static Matrix4 Identity();
    static Matrix4 Zero();
    static Matrix4 Translation(const Vector3 &translation);
    static Matrix4 RotationX(float angle);
    static Matrix4 RotationY(float angle);
    static Matrix4 RotationZ(float angle);
    static Matrix4 Rotation(float angle, const Vector3 &axis);
    static Matrix4 Rotation(const Quaternion &q);
    static Matrix4 Scale(const Vector3 &scale);
    static Matrix4 Scale(float scale);

    // 视图矩阵
    static Matrix4 LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up);
    static Matrix4 LookAt(const Vector3 &position, const Vector3 &forward, const Vector3 &up, const Vector3 &right);

    // 投影矩阵
    static Matrix4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    static Matrix4 Orthographic(float width, float height, float nearClip, float farClip);
    static Matrix4 Perspective(float fovY, float aspect, float nearClip, float farClip);
    static Matrix4 Perspective(float left, float right, float bottom, float top, float nearClip, float farClip);

    // 分解矩阵
    Vector3 GetTranslation() const;
    Quaternion GetRotation() const;
    Vector3 GetScale() const;

    // 插值
    static Matrix4 Lerp(const Matrix4 &a, const Matrix4 &b, float t);

    // 字符串表示
    std::string ToString() const;

private:
    float Minor(int r0, int r1, int r2, int c0, int c1, int c2) const;
    float Cofactor(int row, int col) const;
};

#endif // __MATRIX4_H__