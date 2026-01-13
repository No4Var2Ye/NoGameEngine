
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "GL/gl.h"
#include "GL/glu.h"

#ifndef RGB
    #define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif

// 字符信息结构
struct GLCharInfo
{
    GLuint textureID;      // 字符纹理ID
    int width;            // 字符宽度
    int height;           // 字符高度
    int bearingX;         // 水平偏移
    int bearingY;         // 垂直偏移
    unsigned int advance; // 水平步进
    float s0, t0, s1, t1; // 纹理坐标
};

class OpenGLFont
{
public:
    OpenGLFont();
    ~OpenGLFont();
    
    // 加载字体
    bool LoadSystemFont(const std::string& fontName = "Arial", int fontSize = 16);
    bool LoadFromFile(const std::string& filename, int fontSize = 16);
    
    // 渲染文字
    void RenderText(const std::string& text, float x, float y, float scale = 1.0f,
                   const float color[4] = nullptr);
    void RenderText(const std::wstring& text, float x, float y, float scale = 1.0f,
                   const float color[4] = nullptr);
    
    // 测量文字
    float GetTextWidth(const std::string& text, float scale = 1.0f);
    float GetTextWidth(const std::wstring& text, float scale = 1.0f);
    float GetTextHeight(const std::string& text = "", float scale = 1.0f);
    
    // 获取字符信息
    int GetFontSize() const { return m_FontSize; }
    int GetLineHeight() const { return m_LineHeight; }
    
private:
    bool Initialize();
    bool CreateFontTexture();
    bool GenerateCharTexture(unsigned char c);
    void SetupRenderData();
    void Cleanup();
    
    // Windows GDI字体相关
    HFONT m_hFont;
    HDC m_hDC;
    HBITMAP m_hBitmap;
    
    // OpenGL相关
    std::unordered_map<unsigned char, GLCharInfo> m_CharMap;
    GLuint m_TextureAtlas;       // 字体纹理图集
    GLuint m_VAO, m_VBO;         // 顶点数据
    int m_AtlasWidth, m_AtlasHeight;  // 图集尺寸
    
    // 字体信息
    int m_FontSize;
    int m_LineHeight;
    std::string m_FontName;
    
    // 渲染状态
    float m_Color[4];  // 当前颜色
};