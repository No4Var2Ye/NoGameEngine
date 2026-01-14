
// ======================================================================
#ifndef __LOG_UTILS_H__
#define __LOG_UTILS_H__
// ======================================================================
#include <windows.h>
#include <iostream>
#include <string>
#include <cstdarg>
// ======================================================================

enum class LogLevel
{
    MYERROR,
    WARNING,
    INFO,
    DEBUG
};

inline void Log(LogLevel level, const wchar_t *message)
{
    switch (level)
    {
    case LogLevel::MYERROR:
    {
        std::wcerr << L"[ERROR] " << message << std::endl;
#ifdef MYDEBUG
        OutputDebugStringW((std::wstring(L"[ERROR] ") + message + L"\n").c_str());
#endif
        break;
    }

    case LogLevel::WARNING:
    {
        std::wcerr << L"[WARNING] " << message << std::endl;
#ifdef MYDEBUG
        OutputDebugStringW((std::wstring(L"[WARNING] ") + message + L"\n").c_str());
#endif
        break;
    }

    case LogLevel::INFO:
        std::wcout << L"[INFO] " << message << std::endl;
        break;
    case LogLevel::DEBUG:
#ifdef MYDEBUG
        std::wcout << L"[DEBUG] " << message << std::endl;
        OutputDebugStringW((std::wstring(L"[DEBUG] ") + message + L"\n").c_str());
#endif
        break;
    }
}

inline void LogFormatV(LogLevel level, const wchar_t *format, va_list args)
{
    // 1. 第一次调用：计算格式化后需要的空间大小
    int size = vswprintf(nullptr, 0, format, args);
    if (size <= 0)
        return;

    // 2. 分配足够的缓冲区 (+1 为了存储 \0)
    std::vector<wchar_t> buffer(size + 1);

    // 3. 第二次调用：实际将内容写入缓冲区
    vswprintf(buffer.data(), buffer.size(), format, args);

    // 4. 调用原始的 Log 函数
    Log(level, buffer.data());
}

inline void LogError(const wchar_t *format, ...)
{
    va_list args;
    va_start(args, format);
    LogFormatV(LogLevel::MYERROR, format, args);
    va_end(args);
}

inline void LogWarning(const wchar_t *format, ...)
{
    va_list args;
    va_start(args, format);
    LogFormatV(LogLevel::WARNING, format, args);
    va_end(args);
}

inline void LogInfo(const wchar_t *format, ...)
{
    va_list args;
    va_start(args, format);
    LogFormatV(LogLevel::INFO, format, args);
    va_end(args);
}

inline void LogDebug(const wchar_t *format, ...)
{
#ifdef MYDEBUG
    va_list args;
    va_start(args, format);
    LogFormatV(LogLevel::DEBUG, format, args);
    va_end(args);
#endif
}

#endif // __LOG_UTILS_H__