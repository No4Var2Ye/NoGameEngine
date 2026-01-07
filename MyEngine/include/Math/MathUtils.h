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
// ======================================================================

namespace Math
{
    // ======================================================================
    // 常数
    const float PI = 3.14159265f;
    // const float PI = 3.14159265358979323846f;
    const float TWO_PI = 2.0f * PI;
    const float HALF_PI = 0.5f * PI;
    const float DEG_TO_RAD = PI / 180.0f;
    const float RAD_TO_DEG = 180.0f / PI;
    
    const float EPSILON = 1e-6f;
    const float FLOAT_MAX = std::numeric_limits<float>::max();
    const float FLOAT_MIN = std::numeric_limits<float>::lowest();
    
    // ======================================================================
    // 绝对值
    template<typename T>
    inline T Abs(T value) { return value < 0 ? -value : value; }
    
    // ======================================================================
    // 最小值/最大值
    template<typename T>
    inline T Min(T a, T b) { return a < b ? a : b; }
    
    template<typename T>
    inline T Max(T a, T b) { return a > b ? a : b; }
    
    template<typename T>
    inline T Clamp(T value, T min, T max) { 
        return value < min ? min : (value > max ? max : value); 
    }
    
    // ======================================================================
    // 线性插值
    inline float Lerp(float a, float b, float t) { 
        return a + t * (b - a); 
    }
    
    // ======================================================================
    // 角度转换
    inline float ToRadians(float degrees) { 
        return degrees * DEG_TO_RAD; 
    }
    
    inline float ToDegrees(float radians) { 
        return radians * RAD_TO_DEG; 
    }
    
    // ======================================================================
    // 浮点数比较
    inline bool IsZero(float value) { 
        return Abs(value) < EPSILON; 
    }
    
    inline bool FloatEqual(float a, float b) { 
        return Abs(a - b) < EPSILON; 
    }
    
    // ======================================================================
    // 平方和开方
    inline float Sqrt(float value) { 
        return sqrtf(value); 
    }
    
    inline float InvSqrt(float value) { 
        return 1.0f / sqrtf(value); 
    }
    
    inline float Pow(float base, float exponent) { 
        return powf(base, exponent); 
    }
    
    // ======================================================================
    // 三角函数
    inline float Sin(float angle) { 
        return sinf(angle); 
    }
    
    inline float Cos(float angle) { 
        return cosf(angle); 
    }
    
    inline float Tan(float angle) { 
        return tanf(angle); 
    }
    
    inline float Asin(float value) { 
        return asinf(value); 
    }
    
    inline float Acos(float value) { 
        return acosf(value); 
    }
    
    inline float Atan(float value) { 
        return atanf(value); 
    }
    
    inline float Atan2(float y, float x) { 
        return atan2f(y, x); 
    }
    
    // ======================================================================
    // 随机数
    inline float Random() { 
        return static_cast<float>(rand()) / RAND_MAX; 
    }
    
    inline float Random(float min, float max) { 
        return min + Random() * (max - min); 
    }
    
    // ======================================================================
    // 取整函数
    inline float Floor(float value) { 
        return floorf(value); 
    }
    
    inline float Ceil(float value) { 
        return ceilf(value); 
    }
    
    inline float Round(float value) { 
        return floorf(value + 0.5f); 
    }
}

#endif // __MATH_UTILS_H__