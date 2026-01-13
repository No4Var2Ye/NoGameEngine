// Graphics/FontManager.h
#pragma once
#include "Graphics/OpenGLFont.h"
#include <unordered_map>
#include <memory>
#include <string>

class FontManager
{
public:
    static FontManager &GetInstance();

    FontManager(const FontManager &) = delete;
    FontManager &operator=(const FontManager &) = delete;

    // 字体管理
    bool LoadFont(const std::string &name, const std::string &fontName = "Arial",
                  int fontSize = 16);
    OpenGLFont *GetFont(const std::string &name = "default");
    bool SetCurrentFont(const std::string &name);

    // 渲染接口
    void RenderText(const std::string &text, float x, float y, float scale = 1.0f,
                    const float color[4] = nullptr, const std::string &fontName = "");

    void RenderText(const std::wstring &text, float x, float y, float scale = 1.0f,
                    const float color[4] = nullptr, const std::string &fontName = "");

    // 测量
    float GetTextWidth(const std::string &text, float scale = 1.0f,
                       const std::string &fontName = "");
    float GetTextHeight(const std::string &text = "", float scale = 1.0f,
                        const std::string &fontName = "");

private:
    FontManager();
    ~FontManager();

    std::unordered_map<std::string, std::unique_ptr<OpenGLFont>> m_Fonts;
    OpenGLFont *m_pCurrentFont;
    std::string m_CurrentFontName;

    static FontManager *s_pInstance; // 静态实例指针
};