
// ======================================================================
#ifndef __LOG_UTILS_H__
#define __LOG_UTILS_H__
// ======================================================================
#include <windows.h>
#include <iostream>
#include <string>
#include <cstdarg>
#include <vector>
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
    const wchar_t *prefix = L"";

    switch (level)
    {
    case LogLevel::MYERROR:
        prefix = L"[ERROR] ";
        break;
    case LogLevel::WARNING:
        prefix = L"[WARNING] ";
        break;
    case LogLevel::INFO:
        prefix = L"[INFO] ";
        break;
    case LogLevel::DEBUG:
        prefix = L"[DEBUG] ";
        break;
    }

    std::wcout << prefix << message << std::endl;

#ifdef MYDEBUG
    // 3. 调试输出也加上前缀和换行
    std::wstring dbg = prefix;
    dbg += message;
    dbg += L"\n";
    OutputDebugStringW(dbg.c_str());
#endif
}

inline void LogFormatV(LogLevel level, const wchar_t *format, va_list args)
{
    // 1. 第一次调用：计算格式化后需要的空间大小
    va_list argsCopy;
    va_copy(argsCopy, args); // 必须拷贝 args，因为 vswprintf 会改变它
    int size = _vscwprintf(format, argsCopy);
    va_end(argsCopy);

    if (size <= 0)
        return;

    // 2. 分配足够的缓冲区 (+1 为了存储 \0)
    std::vector<wchar_t> buffer(size + 1);

    // 3. 第二次调用：实际将内容写入缓冲区
    _vsnwprintf_s(buffer.data(), buffer.size(), _TRUNCATE, format, args);
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