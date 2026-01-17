
// ======================================================================
#include "stdafx.h"

#include <cassert>
#include "Graphics/UI/FontManager.h"
// ======================================================================


FontManager* FontManager::s_pInstance = nullptr;

FontManager& FontManager::GetInstance()
{
    if (!s_pInstance)
    {
        s_pInstance = new FontManager();
    }
    return *s_pInstance;
}

FontManager::FontManager()
    : m_pCurrentFont(nullptr)
{
    // 可以在这里初始化默认字体
}

FontManager::~FontManager()
{
    m_Fonts.clear();
    m_pCurrentFont = nullptr;
    
    // 清理单例实例
    if (s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

bool FontManager::LoadFont(const std::string& name, const std::string& fontName, int fontSize)
{
    // 如果已存在，先删除
    auto it = m_Fonts.find(name);
    if (it != m_Fonts.end())
    {
        m_Fonts.erase(it);
    }
    
    // 创建新字体
    auto font = std::make_unique<OpenGLFont>();
    if (!font->LoadSystemFont(fontName, fontSize))
    {
        return false;
    }
    
    m_Fonts[name] = std::move(font);
    
    // 如果是第一个字体，设为当前字体
    if (!m_pCurrentFont)
    {
        m_pCurrentFont = m_Fonts[name].get();
        m_CurrentFontName = name;
    }
    
    return true;
}

OpenGLFont* FontManager::GetFont(const std::string& name)
{
    if (name.empty())
    {
        return m_pCurrentFont;
    }
    
    auto it = m_Fonts.find(name);
    if (it != m_Fonts.end())
    {
        return it->second.get();
    }
    
    return nullptr;
}

bool FontManager::SetCurrentFont(const std::string& name)
{
    auto it = m_Fonts.find(name);
    if (it == m_Fonts.end())
    {
        return false;
    }
    
    m_pCurrentFont = it->second.get();
    m_CurrentFontName = name;
    return true;
}

void FontManager::RenderText(const std::string& text, float x, float y, float scale,
                            const float color[4], const std::string& fontName)
{
    OpenGLFont* pFont = GetFont(fontName);
    if (pFont)
    {
        pFont->RenderText(text, x, y, scale, color);
    }
}

float FontManager::GetTextWidth(const std::string& text, float scale, const std::string& fontName)
{
    OpenGLFont* pFont = GetFont(fontName);
    if (pFont)
    {
        return pFont->GetTextWidth(text, scale);
    }
    return 0.0f;
}

float FontManager::GetTextHeight(const std::string& text, float scale, const std::string& fontName)
{
    OpenGLFont* pFont = GetFont(fontName);
    if (pFont)
    {
        return pFont->GetTextHeight(text, scale);
    }
    return 0.0f;
}