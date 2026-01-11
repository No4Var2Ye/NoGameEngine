// ======================================================================
#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

// ======================================================================
#include <cmath>
#include <limits>
#include <algorithm>
#include <random>
// ======================================================================

namespace Math
{
    // ======================================================================
    // 常数
    const FLOAT PI = 3.14159265f;
    // const FLOAT PI = 3.14159265358979323846f;
    // const FLOAT TWO_PI = 2.0f * PI;
    // const FLOAT HALF_PI = 0.5f * PI;
	const FLOAT TWO_PI = 6.2831853071f;
	const FLOAT HALF_PI = 1.5707963267f;
    const FLOAT DEG_TO_RAD = PI / 180.0f;
    const FLOAT RAD_TO_DEG = 180.0f / PI;

    const FLOAT EPSILON = 1e-6f;
    const FLOAT FLOAT_MAX = std::numeric_limits<FLOAT>::max();
    const FLOAT FLOAT_MIN = std::numeric_limits<FLOAT>::lowest();

    // ======================================================================
    // 基础运算
    // ======================================================================
    template <typename T>
    inline T Abs(T value) { return value < 0 ? -value : value; }

    template <typename T>
    inline T Min(T a, T b) { return a < b ? a : b; }

    template <typename T>
    inline T Max(T a, T b) { return a > b ? a : b; }

    template <typename T>
    inline T Clamp(T value, T min, T max)
    {
        return value < min ? min : (value > max ? max : value);
    }

    // 获取符号: 返回 -1 (负), 0 (零), 1 (正)
    template <typename T>
    inline int Sign(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

    // ======================================================================
    // 插值与映射
    // ======================================================================
    inline FLOAT Lerp(FLOAT a, FLOAT b, FLOAT t)
    {
        return a + t * (b - a);
    }

    // 平滑插值 (常用于相机跟随或平滑动画)
    inline FLOAT SmoothStep(FLOAT edge0, FLOAT edge1, FLOAT x)
    {
        FLOAT t = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    // 将 value 从 [min1, max1] 映射到 [min2, max2]
    inline FLOAT Map(FLOAT value, FLOAT min1, FLOAT max1, FLOAT min2, FLOAT max2)
    {
        return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
    }

    // ======================================================================
    // 转换与比较
    // ======================================================================
    inline FLOAT ToRadians(FLOAT degrees)
    {
        return degrees * DEG_TO_RAD;
    }

    inline FLOAT ToDegrees(FLOAT radians)
    {
        return radians * RAD_TO_DEG;
    }

    inline bool IsZero(FLOAT value)
    {
        return Abs(value) < EPSILON;
    }

    inline bool FloatEqual(FLOAT a, FLOAT b)
    {
        return Abs(a - b) < EPSILON;
    }

    // ======================================================================
    // 高级数学
    // ======================================================================
    inline FLOAT Sqrt(FLOAT value)
    {
        return sqrtf(value);
    }

    inline FLOAT InvSqrt(FLOAT value)
    {
        return (value > EPSILON) ? 1.0f / sqrtf(value) : 0.0f;
    }

    inline FLOAT Pow(FLOAT base, FLOAT exponent)
    {
        return powf(base, exponent);
    }

    // ======================================================================
    // 三角函数
    // ======================================================================
    inline FLOAT Sin(FLOAT angle)
    {
        return sinf(angle);
    }

    inline FLOAT Cos(FLOAT angle)
    {
        return cosf(angle);
    }

    inline FLOAT Tan(FLOAT angle)
    {
        return tanf(angle);
    }

    inline FLOAT Asin(FLOAT value)
    {
        // 安全裁剪，防止精度误差导致 NaN
        if (value <= -1.0f) return -HALF_PI; // -PI/2
        if (value >= 1.0f) return HALF_PI;  // PI/2
        return asinf(value);
    }

    inline FLOAT Acos(FLOAT value)
    {
        return acosf(value);
    }

    inline FLOAT Atan(FLOAT value)
    {
        return atanf(value);
    }

    inline FLOAT Atan2(FLOAT y, FLOAT x)
    {
        return atan2f(y, x);
    }

    // ======================================================================
    // 随机数
    // ======================================================================
    inline FLOAT Random()
    {
        static std::mt19937 generator(std::random_device{}());
        static std::uniform_real_distribution<FLOAT> distribution(0.0f, 1.0f);
        return distribution(generator);
    }

    inline FLOAT Random(FLOAT min, FLOAT max)
    {
        return min + Random() * (max - min);
    }

    // ======================================================================
    // 取整函数
    // ======================================================================
    inline FLOAT Floor(FLOAT value)
    {
        return floorf(value);
    }

    inline FLOAT Ceil(FLOAT value)
    {
        return ceilf(value);
    }

    inline FLOAT Round(FLOAT value)
    {
        return roundf(value);
    }
}

#endif // __MATH_UTILS_H__