
// ======================================================================
#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__
// ======================================================================
#include <string>
#include <windows.h>
// ======================================================================

class CStringUtils
{
public:
    // 将宽字符串 (UTF-16) 转换为 ANSI 字符串 (或指定编码)
    static std::string WStringToString(const std::wstring &wstr, UINT codePage = CP_ACP);

    // 将 ANSI 字符串 转换为 宽字符串 (UTF-16)
    static std::wstring StringToWString(const std::string &str, UINT codePage = CP_ACP);
};

#endif // __STRING_UTILS_H__