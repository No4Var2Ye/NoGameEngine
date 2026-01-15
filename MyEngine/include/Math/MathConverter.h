
// ======================================================================
#ifndef __MATH_CONVERTER_H__
#define __MATH_CONVERTER_H__
// ======================================================================
#include "assimp/matrix4x4.h"
#include "assimp/vector3.h"
#include "assimp/quaternion.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
// ======================================================================

/**
 * @brief 负责 Assimp 数据类型与自定义数学类之间的转换
 */
class CMathConverter
{
public:
    // 向量转换
    static inline Vector3 ToVector3(const aiVector3D &v)
    {
        return Vector3(v.x, v.y, v.z);
    }

    // 颜色转换 (aiColor3D -> Vector3)
    static inline Vector3 ToVector3(const aiColor3D &c)
    {
        return Vector3(c.r, c.g, c.b);
    }

    // 颜色转换 (aiColor4D -> Vector4)
    static inline Vector4 ToVector4(const aiColor4D &c)
    {
        return Vector4(c.r, c.g, c.b, c.a);
    }

    // 颜色转换 (aiColor3D -> Vector4)，alpha默认为1.0
    static inline Vector4 ToVector4(const aiColor3D &c, float alpha = 1.0f)
    {
        return Vector4(c.r, c.g, c.b, alpha);
    }

    // 四元数转换
    static inline Quaternion ToQuaternion(const aiQuaternion &q)
    {
        // 注意映射顺序：Assimp 为 (w, x, y, z)，你的构造函数需确认顺序
        // 构造函数是 Quaternion(x, y, z, w)
        return Quaternion(q.x, q.y, q.z, q.w);
    }

    // 矩阵转换
    /**
     * Assimp 使用行主序 (Row-Major)，内存布局为 a1,a2,a3,a4...
     *  Matrix4 是列主序 (Column-Major)，且构造函数接收行值
     */
    static inline Matrix4 ToMatrix4(const aiMatrix4x4 &m)
    {
        // 这种写法会将 Assimp 的行值填入你 Matrix4 构造函数的行参数中
        // 由于你的 Matrix4 内部是列优先存储，直接这样传参通常能完成转置效果
        return Matrix4(
            m.a1, m.a2, m.a3, m.a4,
            m.b1, m.b2, m.b3, m.b4,
            m.c1, m.c2, m.c3, m.c4,
            m.d1, m.d2, m.d3, m.d4);
    }

    // 逆转换：Vector3 -> aiVector3D
    static inline aiVector3D ToAiVector3D(const Vector3 &v)
    {
        return aiVector3D(v.x, v.y, v.z);
    }

    // 逆转换：Quaternion -> aiQuaternion
    static inline aiQuaternion ToAiQuaternion(const Quaternion &q)
    {
        return aiQuaternion(q.w, q.x, q.y, q.z);
    }

    // 材质辅助函数
    static inline void GetMaterialColor(aiMaterial *mat, const char *pKey,
                                        unsigned int type, unsigned int idx,
                                        Vector3 &outColor)
    {
        aiColor3D color(0.0f, 0.0f, 0.0f);
        if (mat->Get(pKey, type, idx, color) == AI_SUCCESS)
        {
            outColor = ToVector3(color);
        }
    }

    static inline void GetMaterialColor(aiMaterial *mat, const char *pKey,
                                        unsigned int type, unsigned int idx,
                                        Vector4 &outColor)
    {
        aiColor4D color(0.0f, 0.0f, 0.0f, 1.0f);
        if (mat->Get(pKey, type, idx, color) == AI_SUCCESS)
        {
            outColor = ToVector4(color);
        }
    }

    static inline float GetMaterialFloat(aiMaterial *mat, const char *pKey,
                                         unsigned int type, unsigned int idx,
                                         float defaultValue = 0.0f)
    {
        float value = defaultValue;
        mat->Get(pKey, type, idx, value);
        return value;
    }

    static inline int GetMaterialInt(aiMaterial *mat, const char *pKey,
                                     unsigned int type, unsigned int idx,
                                     int defaultValue = 0)
    {
        int value = defaultValue;
        mat->Get(pKey, type, idx, value);
        return value;
    }
};

#endif // __MATH_CONVERTER_H__