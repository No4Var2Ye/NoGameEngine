
#ifndef __DEBUG_UTILS_H__
#define __DEBUG_UTILS_H__

#include <Windows.h>

// 简单的调试输出宏
#ifdef MYDEBUG
    // 格式化输出
    #define LOG(...) \
        do { \
            char buffer[512]; \
            sprintf_s(buffer, sizeof(buffer), __VA_ARGS__); \
            OutputDebugStringA(buffer); \
            OutputDebugStringA("\n"); \
        } while(0)
    
    // 带标签的输出
    #define LOG_INFO(...) LOG("[INFO] " __VA_ARGS__)
    #define LOG_WARN(...) LOG("[WARN] " __VA_ARGS__)
    #define LOG_ERROR(...) LOG("[ERROR] " __VA_ARGS__)
    
    // 跟踪输出
    #define LOG_TRACE() LOG("[TRACE] %s:%d", __FILE__, __LINE__)
#else
    // 发布版本为空
    #define LOG(...)
    #define LOG_INFO(...)
    #define LOG_WARN(...)
    #define LOG_ERROR(...)
    #define LOG_TRACE()
#endif

#endif // __DEBUG_UTILS_H__