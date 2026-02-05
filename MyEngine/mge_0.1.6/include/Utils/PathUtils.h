// ======================================================================
#ifndef __PATH_UTILS_H__
#define __PATH_UTILS_H__
// ======================================================================

#include <Windows.h>
#include <string>

namespace PathUtils
{
    // 检查路径是否存在 (inline 允许直接在头文件定义实现)
    inline bool Exists(const std::wstring &path)
    {
        if (path.empty())
            return false;
        DWORD attr = GetFileAttributesW(path.c_str());
        return (attr != INVALID_FILE_ATTRIBUTES);
    }

    inline bool Exists(const std::string &path)
    {
        if (path.empty())
            return false;
        // VS2013 环境下简单的转换逻辑
        int len = MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, NULL, 0);
        if (len <= 0)
            return false;
        wchar_t *wbuf = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, wbuf, len);
        bool result = Exists(wbuf);
        delete[] wbuf;
        return result;
    }
}

#endif // __PATH_UTILS_H__