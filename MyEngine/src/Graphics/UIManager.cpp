
// ======================================================================
#include "stdafx.h"

#include <cstdarg>
#include <cassert>
#include "Core/Renderer.h"
#include "Graphics/UIManager.h"
#include "Math/MathUtils.h"
// ======================================================================

// ======================================================================
// =========================== 公有方法 ==================================
// ======================================================================

CUIManager::CUIManager()
    : m_hWnd(nullptr),
      m_hFont(nullptr)
{
}

CUIManager::~CUIManager()
{
    Shutdown();
}

BOOL CUIManager::Initialize(HWND hWnd, const std::wstring &fontName, int fontSize)
{
    if (!hWnd)
        return FALSE;

    if (fontSize <= 0 || fontSize > 100) // 合理的字体大小范围
    {
        fontSize = Math::Clamp(fontSize, 8, 72); // 限制在合理范围
    }

    m_hWnd = hWnd;
    m_IsInitialized = FALSE;

    BOOL fontResult = SetFont(fontName, fontSize, false);
    m_IsInitialized = fontResult;

    if (!m_IsInitialized)
    {
        OutputDebugStringW(L"CUIManager::Initialize: 失败 - 字体创建失败\n");
        return FALSE;
    }

    return (m_hFont != nullptr);
}

void CUIManager::Shutdown()
{
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = nullptr;
    }

    m_DebugTexts.clear();
    m_hWnd = nullptr;
    m_IsInitialized = FALSE;
}

BOOL CUIManager::SetFont(const std::wstring &fontName, int size, bool bold)
{
    if (!m_hWnd)
    {
        return FALSE;
    }

    HFONT newFont = InternalCreateFont(fontName, size, bold);

    if (newFont)
    {
        // 先删除旧字体，防止内存/句柄泄漏
        if (m_hFont)
            DeleteObject(m_hFont);

        m_hFont = newFont;
    }
    return TRUE;
}

void CUIManager::BeginFrame()
{
    if (!m_IsInitialized)
        return;

    m_DebugTexts.clear();
    m_DebugTexts.reserve(32); // 预分配空间提高性能
}

void CUIManager::AddDebugText(const std::wstring &text, int x, int y, COLORREF color)
{
    if (!m_IsInitialized || text.empty())
        return;

    DebugText item;
    item.text = text;
    item.x = x;
    item.y = y;
    item.color = color;

    m_DebugTexts.push_back(std::move(item));
}

void CUIManager::AddDebugTextFormat(int x, int y, COLORREF color, const wchar_t *format, ...)
{
    if (!m_IsInitialized || !format)
        return;

    va_list args;
    va_start(args, format);

    std::wstring formattedText = StringFormatter::FormatV(format, args);

    va_end(args);

    AddDebugText(formattedText, x, y, color);
}

void CUIManager::Render()
{
    if (!m_IsInitialized || !m_hWnd || m_DebugTexts.empty())
        return;

    HDC hdc = GetDC(m_hWnd);

    // 设置绘图状态
    SetBkMode(hdc, TRANSPARENT);
    HGDIOBJ oldFont = SelectObject(hdc, m_hFont);

    // 批量绘制缓冲区内的文字
    for (const auto &item : m_DebugTexts)
    {
        SetTextColor(hdc, item.color);
        TextOutW(hdc, item.x, item.y, item.text.c_str(), (int)item.text.length());
    }

    // 清理
    SelectObject(hdc, oldFont);
    ReleaseDC(m_hWnd, hdc);
}

// ======================================================================
// =========================== 私有方法 ==================================
// ======================================================================

HFONT CUIManager::InternalCreateFont(const std::wstring &fontName, int size, bool bold)
{
    return CreateFontW(
        size,                        // 字号
        0, 0, 0,                     // 宽度、倾斜、角度
        bold ? FW_BOLD : FW_NORMAL,  // 字重
        FALSE, FALSE, FALSE,         // 斜体、下划线、删除线
        DEFAULT_CHARSET,             // 字符集
        OUT_DEFAULT_PRECIS,          //
        CLIP_DEFAULT_PRECIS,         //
        CLEARTYPE_QUALITY,           // 关键：抗锯齿
        DEFAULT_PITCH | FF_DONTCARE, // 自动适配间距
        fontName.c_str()             // 字体名称
    );
}