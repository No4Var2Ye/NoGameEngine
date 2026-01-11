#include "stdafx.h"
#include "Math/MathUtils.h"
#include "Math/Matrix4.h"
#include <sstream>
#include <cstring>

// 构造函数
Matrix4::Matrix4()
{
    memset(m, 0, sizeof(m));
    m00 = m11 = m22 = m33 = 1.0f; // 单位矩阵
}

Matrix4::Matrix4(float diagonal)
{
    memset(m, 0, sizeof(m));
    m00 = m11 = m22 = m33 = diagonal;
}

Matrix4::Matrix4(float _m00, float _m01, float _m02, float _m03,
                 float _m10, float _m11, float _m12, float _m13,
                 float _m20, float _m21, float _m22, float _m23,
                 float _m30, float _m31, float _m32, float _m33)
{
    m00 = _m00;
    m01 = _m01;
    m02 = _m02;
    m03 = _m03;
    m10 = _m10;
    m11 = _m11;
    m12 = _m12;
    m13 = _m13;
    m20 = _m20;
    m21 = _m21;
    m22 = _m22;
    m23 = _m23;
    m30 = _m30;
    m31 = _m31;
    m32 = _m32;
    m33 = _m33;
}

// 矩阵运算
Matrix4 Matrix4::operator+(const Matrix4 &other) const
{
    Matrix4 result;
    for (int i = 0; i < 16; ++i)
        result.m[i] = m[i] + other.m[i];
    return result;
}

Matrix4 Matrix4::operator-(const Matrix4 &other) const
{
    Matrix4 result;
    for (int i = 0; i < 16; ++i)
        result.m[i] = m[i] - other.m[i];
    return result;
}

Matrix4 Matrix4::operator*(const Matrix4 &other) const
{
    Matrix4 result;
    result.m00 = m00 * other.m00 + m01 * other.m10 + m02 * other.m20 + m03 * other.m30;
    result.m01 = m00 * other.m01 + m01 * other.m11 + m02 * other.m21 + m03 * other.m31;
    result.m02 = m00 * other.m02 + m01 * other.m12 + m02 * other.m22 + m03 * other.m32;
    result.m03 = m00 * other.m03 + m01 * other.m13 + m02 * other.m23 + m03 * other.m33;

    result.m10 = m10 * other.m00 + m11 * other.m10 + m12 * other.m20 + m13 * other.m30;
    result.m11 = m10 * other.m01 + m11 * other.m11 + m12 * other.m21 + m13 * other.m31;
    result.m12 = m10 * other.m02 + m11 * other.m12 + m12 * other.m22 + m13 * other.m32;
    result.m13 = m10 * other.m03 + m11 * other.m13 + m12 * other.m23 + m13 * other.m33;

    result.m20 = m20 * other.m00 + m21 * other.m10 + m22 * other.m20 + m23 * other.m30;
    result.m21 = m20 * other.m01 + m21 * other.m11 + m22 * other.m21 + m23 * other.m31;
    result.m22 = m20 * other.m02 + m21 * other.m12 + m22 * other.m22 + m23 * other.m32;
    result.m23 = m20 * other.m03 + m21 * other.m13 + m22 * other.m23 + m23 * other.m33;

    result.m30 = m30 * other.m00 + m31 * other.m10 + m32 * other.m20 + m33 * other.m30;
    result.m31 = m30 * other.m01 + m31 * other.m11 + m32 * other.m21 + m33 * other.m31;
    result.m32 = m30 * other.m02 + m31 * other.m12 + m32 * other.m22 + m33 * other.m32;
    result.m33 = m30 * other.m03 + m31 * other.m13 + m32 * other.m23 + m33 * other.m33;

    return result;
}

Matrix4 Matrix4::operator*(float scalar) const
{
    Matrix4 result;
    for (int i = 0; i < 16; ++i)
        result.m[i] = m[i] * scalar;
    return result;
}

Vector4 Matrix4::operator*(const Vector4 &vec) const
{
    return Vector4(
        m00 * vec.x + m01 * vec.y + m02 * vec.z + m03 * vec.w,
        m10 * vec.x + m11 * vec.y + m12 * vec.z + m13 * vec.w,
        m20 * vec.x + m21 * vec.y + m22 * vec.z + m23 * vec.w,
        m30 * vec.x + m31 * vec.y + m32 * vec.z + m33 * vec.w);
}

Vector3 Matrix4::operator*(const Vector3 &vec) const
{
    // 假设向量的w分量为1（点）
    float w = m30 * vec.x + m31 * vec.y + m32 * vec.z + m33;
    if (Math::IsZero(w))
        w = 1.0f;
    return Vector3(
        (m00 * vec.x + m01 * vec.y + m02 * vec.z + m03) / w,
        (m10 * vec.x + m11 * vec.y + m12 * vec.z + m13) / w,
        (m20 * vec.x + m21 * vec.y + m22 * vec.z + m23) / w);
}

// 赋值运算
Matrix4 &Matrix4::operator+=(const Matrix4 &other)
{
    for (int i = 0; i < 16; ++i)
        m[i] += other.m[i];
    return *this;
}

Matrix4 &Matrix4::operator-=(const Matrix4 &other)
{
    for (int i = 0; i < 16; ++i)
        m[i] -= other.m[i];
    return *this;
}

Matrix4 &Matrix4::operator*=(const Matrix4 &other)
{
    *this = *this * other;
    return *this;
}

Matrix4 &Matrix4::operator*=(float scalar)
{
    for (int i = 0; i < 16; ++i)
        m[i] *= scalar;
    return *this;
}

// 比较运算符
bool Matrix4::operator==(const Matrix4 &other) const
{
    for (int i = 0; i < 16; ++i)
        if (!Math::FloatEqual(m[i], other.m[i]))
            return false;
    return true;
}

bool Matrix4::operator!=(const Matrix4 &other) const
{
    return !(*this == other);
}

// 矩阵操作
float Matrix4::Minor(int r0, int r1, int r2, int c0, int c1, int c2) const
{
    return m[r0 * 4 + c0] * (m[r1 * 4 + c1] * m[r2 * 4 + c2] - m[r2 * 4 + c1] * m[r1 * 4 + c2]) - m[r0 * 4 + c1] * (m[r1 * 4 + c0] * m[r2 * 4 + c2] - m[r2 * 4 + c0] * m[r1 * 4 + c2]) + m[r0 * 4 + c2] * (m[r1 * 4 + c0] * m[r2 * 4 + c1] - m[r2 * 4 + c0] * m[r1 * 4 + c1]);
}

float Matrix4::Cofactor(int row, int col) const
{
    int r0 = (row == 0) ? 1 : 0;
    int r1 = (row <= 1) ? 2 : 1;
    int r2 = (row <= 2) ? 3 : 2;

    int c0 = (col == 0) ? 1 : 0;
    int c1 = (col <= 1) ? 2 : 1;
    int c2 = (col <= 2) ? 3 : 2;

    float minor = Minor(r0, r1, r2, c0, c1, c2);
    return ((row + col) % 2 == 0) ? minor : -minor;
}

float Matrix4::Determinant() const
{
    return m00 * Cofactor(0, 0) + m01 * Cofactor(0, 1) + m02 * Cofactor(0, 2) + m03 * Cofactor(0, 3);
}

Matrix4 Matrix4::Transposed() const
{
    return Matrix4(
        m00, m10, m20, m30,
        m01, m11, m21, m31,
        m02, m12, m22, m32,
        m03, m13, m23, m33);
}

Matrix4 &Matrix4::Transpose()
{
    *this = this->Transposed();
    return *this;
}

Matrix4 Matrix4::Inversed() const
{
    float det = Determinant();
    if (Math::IsZero(det))
        return Identity();

    float invDet = 1.0f / det;
    Matrix4 result;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result(i, j) = Cofactor(j, i) * invDet; // 注意：转置伴随矩阵

    return result;
}

Matrix4 &Matrix4::Inverse()
{
    *this = this->Inversed();
    return *this;
}

bool Matrix4::IsIdentity() const
{
    return (*this == Identity());
}

bool Matrix4::IsZero() const
{
    for (int i = 0; i < 16; ++i)
        if (!Math::IsZero(m[i]))
            return false;
    return true;
}

// 获取行/列
Vector4 Matrix4::GetRow(int row) const
{
    return Vector4(m4[0][row], m4[1][row], m4[2][row], m4[3][row]);
}

Vector4 Matrix4::GetColumn(int col) const
{
    return Vector4(m4[col][0], m4[col][1], m4[col][2], m4[col][3]);
}

void Matrix4::SetRow(int row, const Vector4 &vec)
{
    m4[0][row] = vec.x;
    m4[1][row] = vec.y;
    m4[2][row] = vec.z;
    m4[3][row] = vec.w;
}

void Matrix4::SetColumn(int col, const Vector4 &vec)
{
    m4[col][0] = vec.x;
    m4[col][1] = vec.y;
    m4[col][2] = vec.z;
    m4[col][3] = vec.w;
}

// 变换矩阵
Matrix4 Matrix4::Identity()
{
    return Matrix4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::Zero()
{
    return Matrix4(
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f);
}

Matrix4 Matrix4::Translation(const Vector3 &translation)
{
    return Matrix4(
        1.0f, 0.0f, 0.0f, translation.x,
        0.0f, 1.0f, 0.0f, translation.y,
        0.0f, 0.0f, 1.0f, translation.z,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::RotationX(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    return Matrix4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, c, -s, 0.0f,
        0.0f, s, c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::RotationY(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    return Matrix4(
        c, 0.0f, s, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -s, 0.0f, c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::RotationZ(float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    return Matrix4(
        c, -s, 0.0f, 0.0f,
        s, c, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::Rotation(float angle, const Vector3 &axis)
{
    Vector3 a = axis.Normalized();
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    float x = a.x;
    float y = a.y;
    float z = a.z;

    return Matrix4(
        t * x * x + c, t * x * y - s * z, t * x * z + s * y, 0.0f,
        t * x * y + s * z, t * y * y + c, t * y * z - s * x, 0.0f,
        t * x * z - s * y, t * y * z + s * x, t * z * z + c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::Rotation(const Quaternion &q)
{
    // 从四元数转换为旋转矩阵的优化版本
    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;

    return Matrix4(
        1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz), 2.0f * (xz + wy), 0.0f,
        2.0f * (xy + wz), 1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx), 0.0f,
        2.0f * (xz - wy), 2.0f * (yz + wx), 1.0f - 2.0f * (xx + yy), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::Scale(const Vector3 &scale)
{
    return Matrix4(
        scale.x, 0.0f, 0.0f, 0.0f,
        0.0f, scale.y, 0.0f, 0.0f,
        0.0f, 0.0f, scale.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::Scale(float scale)
{
    return Scale(Vector3(scale, scale, scale));
}

// 视图矩阵
Matrix4 Matrix4::LookAt(const Vector3 &eye, const Vector3 &target, const Vector3 &up)
{
    Vector3 f = (target - eye).Normalized();
    Vector3 r = Vector3::Cross(f, up).Normalized();
    Vector3 u = Vector3::Cross(r, f);

    return Matrix4(
        r.x, r.y, r.z, -Vector3::Dot(r, eye),
        u.x, u.y, u.z, -Vector3::Dot(u, eye),
        -f.x, -f.y, -f.z, Vector3::Dot(f, eye),
        0.0f, 0.0f, 0.0f, 1.0f);

    // return Matrix4(
    //     r.x, u.x, -f.x, 0.0f,  // 第一列 (X轴)
    //     r.y, u.y, -f.y, 0.0f,  // 第二列 (Y轴)
    //     r.z, u.z, -f.z, 0.0f,  // 第三列 (Z轴)
    //     -Vector3::Dot(r, eye), // 第四列 (平移X)
    //     -Vector3::Dot(u, eye), // 第四列 (平移Y)
    //     Vector3::Dot(f, eye),  // 第四列 (平移Z)
    //     1.0f                   // 第四列 (W)
    // );
}

// 投影矩阵
Matrix4 Matrix4::Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    float rl = right - left;
    float tb = top - bottom;
    float fn = farPlane - nearPlane;

    return Matrix4(
        2.0f / rl, 0.0f, 0.0f, -(right + left) / rl,
        0.0f, 2.0f / tb, 0.0f, -(top + bottom) / tb,
        0.0f, 0.0f, -2.0f / fn, -(farPlane + nearPlane) / fn,
        0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix4 Matrix4::Orthographic(float width, float height, float nearClip, float farClip)
{
    return Orthographic(-width / 2, width / 2, -height / 2, height / 2, nearClip, farClip);
}

Matrix4 Matrix4::Perspective(float fovY, float aspect, float nearClip, float farClip)
{
    float f = 1.0f / tanf(fovY * 0.5f);
    float rangeInv = 1.0f / (nearClip - farClip);

    return Matrix4(
        f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (farClip + nearClip) * rangeInv, 2 * farClip * nearClip * rangeInv,
        0.0f, 0.0f, -1.0f, 0.0f);

    // 按照 OpenGL 列优先顺序排列 (Column-Major)
    // return Matrix4(
    //     f / aspect, 0.0f, 0.0f, 0.0f,                            // 第一列
    //     0.0f, f, 0.0f, 0.0f,                                     // 第二列
    //     0.0f, 0.0f, (farClip + nearClip) * rangeInv, -1.0f,      // 第三列
    //     0.0f, 0.0f, (2.0f * farClip * nearClip) * rangeInv, 0.0f // 第四列
    // );
}

// 分解矩阵
Vector3 Matrix4::GetTranslation() const
{
    return Vector3(m03, m13, m23);
}

Vector3 Matrix4::GetScale() const
{
    return Vector3(
        Vector3(m00, m10, m20).Length(),
        Vector3(m01, m11, m21).Length(),
        Vector3(m02, m12, m22).Length());
}

// 插值
Matrix4 Matrix4::Lerp(const Matrix4 &a, const Matrix4 &b, float t)
{
    t = Math::Clamp(t, 0.0f, 1.0f);
    Matrix4 result;
    for (int i = 0; i < 16; ++i)
        result.m[i] = a.m[i] + (b.m[i] - a.m[i]) * t;
    return result;
}

// 字符串表示
std::string Matrix4::ToString() const
{
    std::stringstream ss;
    ss << "Matrix4:" << std::endl;
    ss << "[" << m00 << ", " << m01 << ", " << m02 << ", " << m03 << "]" << std::endl;
    ss << "[" << m10 << ", " << m11 << ", " << m12 << ", " << m13 << "]" << std::endl;
    ss << "[" << m20 << ", " << m21 << ", " << m22 << ", " << m23 << "]" << std::endl;
    ss << "[" << m30 << ", " << m31 << ", " << m32 << ", " << m33 << "]" << std::endl;
    return ss.str();
}