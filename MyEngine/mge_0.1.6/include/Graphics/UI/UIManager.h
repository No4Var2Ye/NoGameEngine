
// ======================================================================
#ifndef __UI_MANAGER_H__
#define __UI_MANAGER_H__

// ======================================================================
#include <windows.h>
#include <string>
#include <vector>
#include <GL/gl.h>

#ifndef RGB
    #define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif
// ======================================================================

// ======================================================================
// 简单的调试文本结构
struct DebugText
{
    std::wstring text;
    int x, y;
    COLORREF color;
};

class StringFormatter
{
public:
    static std::wstring Format(const wchar_t *format, ...)
    {
        if (!format)
            return L"";

        va_list args;
        va_start(args, format);
        std::wstring result = FormatV(format, args);
        va_end(args);

        return result;
    }

    static std::wstring FormatV(const wchar_t *format, va_list args)
    {
        if (!format)
            return L"";

        // 获取需要的缓冲区大小
        va_list args_copy;
        va_copy(args_copy, args);
#ifdef _WIN32
        int neededSize = _vscwprintf(format, args_copy);
#else
        int neededSize = vswprintf(nullptr, 0, format, args_copy);
#endif
        va_end(args_copy);

        if (neededSize < 0)
            return L"[Format Error]";

        // 分配缓冲区
        std::vector<wchar_t> buffer(neededSize + 1);

        int result = vswprintf(buffer.data(), buffer.size(), format, args);

        if (result >= 0)
            return std::wstring(buffer.data());
        else
            return L"[Format Error]";
    }
};

class CUIManager
{
public:
    CUIManager();
    ~CUIManager();

    BOOL Initialize(HWND hWnd, const std::wstring &fontName, int fontSize);
    void Shutdown();

    BOOL SetFont(const std::wstring &fontName, int size, bool bold);

    void BeginFrame(); // 每一帧开始时调用，清空上一帧的临时文字

    // 调试信息
    void AddDebugText(const std::wstring &text, int x, int y, COLORREF color);
    void AddDebugTextFormat(int x, int y, COLORREF color, const wchar_t *format, ...);

    // 最终渲染函数
    void Render();

private:
    HWND m_hWnd;   // 缓存窗口句柄
    HFONT m_hFont; // 缓存字体
    std::vector<DebugText> m_DebugTexts;

    BOOL m_IsInitialized = FALSE; // 初始化状态

    // 统一处理字体创建逻辑
    HFONT InternalCreateFont(const std::wstring &fontName, int size, bool bold);
};

#endif // __UI_MANAGER_H__