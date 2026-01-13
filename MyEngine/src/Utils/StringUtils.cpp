
// ======================================================================
#include "stdafx.h"

#include "Utils/StringUtils.h"
// ======================================================================

std::string CStringUtils::WStringToString(const std::wstring& wstr, UINT codePage)
{
    if (wstr.empty()) return std::string();

    // 1. 获取目标字符串所需的长度
    int sizeNeeded = WideCharToMultiByte(codePage, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    
    // 2. 分配缓冲区并执行转换
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(codePage, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
    
    return strTo;
}

std::wstring CStringUtils::StringToWString(const std::string& str, UINT codePage)
{
    if (str.empty()) return std::wstring();

    int sizeNeeded = MultiByteToWideChar(codePage, 0, &str[0], (int)str.size(), NULL, 0);
    
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(codePage, 0, &str[0], (int)str.size(), &wstrTo[0], sizeNeeded);
    
    return wstrTo;
}