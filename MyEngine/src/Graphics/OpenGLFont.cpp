
// ======================================================================
#include "stdafx.h"

#include <algorithm>
#include <cstring>
#include "Graphics/UI/OpenGLFont.h"
#include "Core/Renderer.h"
// ======================================================================


OpenGLFont::OpenGLFont()
    : m_hFont(nullptr)
    , m_hDC(nullptr)
    , m_hBitmap(nullptr)
    , m_TextureAtlas(0)
    , m_VAO(0)
    , m_VBO(0)
    , m_AtlasWidth(512)  // 初始纹理尺寸
    , m_AtlasHeight(512)
    , m_FontSize(16)
    , m_LineHeight(0)
    , m_FontName("Arial")
{
    m_Color[0] = 1.0f;  // R
    m_Color[1] = 1.0f;  // G
    m_Color[2] = 1.0f;  // B
    m_Color[3] = 1.0f;  // A
}

OpenGLFont::~OpenGLFont()
{
    Cleanup();
}

bool OpenGLFont::LoadSystemFont(const std::string& fontName, int fontSize)
{
    Cleanup();  // 清理旧资源
    
    m_FontName = fontName;
    m_FontSize = fontSize;
    
    // 创建内存DC
    HDC screenDC = GetDC(nullptr);
    m_hDC = CreateCompatibleDC(screenDC);
    ReleaseDC(nullptr, screenDC);
    
    if (!m_hDC)
    {
        OutputDebugStringA("OpenGLFont: 无法创建设备上下文\n");
        return false;
    }
    
    // 设置字体
    m_hFont = CreateFontA(
        -fontSize,           // 高度
        0,                   // 宽度
        0,                   // 旋转角度
        0,                   // 朝向角度
        FW_NORMAL,           // 字重
        FALSE,               // 斜体
        FALSE,               // 下划线
        FALSE,               // 删除线
        DEFAULT_CHARSET,     // 字符集
        OUT_TT_PRECIS,       // 输出精度
        CLIP_DEFAULT_PRECIS, // 裁剪精度
        CLEARTYPE_QUALITY,   // 质量
        DEFAULT_PITCH | FF_DONTCARE, // 字距和字族
        fontName.c_str()     // 字体名称
    );
    
    if (!m_hFont)
    {
        OutputDebugStringA("OpenGLFont: 无法创建字体\n");
        Cleanup();
        return false;
    }
    
    // 选择字体到DC
    SelectObject(m_hDC, m_hFont);
    
    // 创建位图
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = 256;  // 足够大的位图
    bmi.bmiHeader.biHeight = 256;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* bits = nullptr;
    m_hBitmap = CreateDIBSection(m_hDC, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!m_hBitmap)
    {
        OutputDebugStringA("OpenGLFont: 无法创建DIB位图\n");
        Cleanup();
        return false;
    }
    
    SelectObject(m_hDC, m_hBitmap);
    
    // 设置背景和文本颜色
    SetBkColor(m_hDC, RGB(0, 0, 0));      // 黑色背景
    SetTextColor(m_hDC, RGB(255, 255, 255));  // 白色文字
    SetBkMode(m_hDC, OPAQUE);
    
    // 获取字体规格
    TEXTMETRIC tm;
    if (GetTextMetrics(m_hDC, &tm))
    {
        m_LineHeight = tm.tmHeight;
    }
    else
    {
        m_LineHeight = fontSize;
    }
    
    // 创建OpenGL纹理图集
    if (!CreateFontTexture())
    {
        Cleanup();
        return false;
    }
    
    // 设置渲染数据
    SetupRenderData();
    
    return true;
}

bool OpenGLFont::CreateFontTexture()
{
    // 生成ASCII字符集（32-126）
    const int firstChar = 32;
    const int lastChar = 126;
    const int charCount = lastChar - firstChar + 1;
    
    // 计算纹理图集尺寸
    m_AtlasWidth = 512;
    m_AtlasHeight = 512;
    
    // 创建OpenGL纹理
    glGenTextures(1, &m_TextureAtlas);
    glBindTexture(GL_TEXTURE_2D, m_TextureAtlas);
    
    // 分配纹理内存
    std::vector<unsigned char> textureData(m_AtlasWidth * m_AtlasHeight, 0);
    
    int x = 0, y = 0;
    int rowHeight = 0;
    
    for (int c = firstChar; c <= lastChar; c++)
    {
        // 获取字符尺寸
        SIZE size;
        char ch = static_cast<char>(c);
        GetTextExtentPoint32A(m_hDC, &ch, 1, &size);
        
        // 如果当前行放不下，换行
        if (x + size.cx > m_AtlasWidth)
        {
            x = 0;
            y += rowHeight;
            rowHeight = 0;
        }
        
        // 更新行高
        if (size.cy > rowHeight)
            rowHeight = size.cy;
        
        // 计算字符在纹理中的位置
        GLCharInfo charInfo;
        charInfo.width = size.cx;
        charInfo.height = size.cy;
        charInfo.bearingX = 0;
        charInfo.bearingY = size.cy;  // GDI坐标系原点在左上角
        
        // 纹理坐标
        charInfo.s0 = static_cast<float>(x) / m_AtlasWidth;
        charInfo.t0 = static_cast<float>(y) / m_AtlasHeight;
        charInfo.s1 = static_cast<float>(x + size.cx) / m_AtlasWidth;
        charInfo.t1 = static_cast<float>(y + size.cy) / m_AtlasHeight;
        
        // 在内存中绘制字符
        RECT rect = {0, 0, size.cx, size.cy};
        FillRect(m_hDC, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        TextOutA(m_hDC, 0, 0, &ch, 1);
        
        // 获取位图数据
        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = size.cx;
        bmi.bmiHeader.biHeight = -size.cy;  // 负值表示从上到下
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        
        std::vector<unsigned char> bitmap(size.cx * size.cy * 4, 0);
        GetDIBits(m_hDC, m_hBitmap, 0, size.cy, bitmap.data(), &bmi, DIB_RGB_COLORS);
        
        // 提取Alpha通道（使用红色通道作为Alpha，因为白色文字在红色通道）
        for (int cy = 0; cy < size.cy; cy++)
        {
            for (int cx = 0; cx < size.cx; cx++)
            {
                int srcIdx = (cy * size.cx + cx) * 4;
                int dstIdx = ((y + cy) * m_AtlasWidth + (x + cx));
                
                unsigned char alpha = bitmap[srcIdx + 2];  // 红色通道
                textureData[dstIdx] = alpha;
            }
        }
        
        // 保存字符信息
        m_CharMap[c] = charInfo;
        
        x += size.cx;
    }
    
    // 上传纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, m_AtlasWidth, m_AtlasHeight, 
                 0, GL_ALPHA, GL_UNSIGNED_BYTE, textureData.data());
    
    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return true;
}

void OpenGLFont::SetupRenderData()
{
    // 创建VAO和VBO
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    
    // 每个字符6个顶点，每个顶点4个float（位置+纹理坐标）
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
    
    // 位置属性
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 纹理坐标属性
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void OpenGLFont::RenderText(const std::string& text, float x, float y, float scale, const float color[4])
{
    if (text.empty() || m_TextureAtlas == 0)
        return;
    
    // 保存OpenGL状态
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    
    // 设置2D正交投影
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // 渲染状态
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    
    // 设置颜色
    if (color)
    {
        glColor4f(color[0], color[1], color[2], color[3]);
    }
    else
    {
        glColor4f(m_Color[0], m_Color[1], m_Color[2], m_Color[3]);
    }
    
    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, m_TextureAtlas);
    glBindVertexArray(m_VAO);
    
    // 渲染每个字符
    float startX = x;
    for (char c : text)
    {
        auto it = m_CharMap.find(static_cast<unsigned char>(c));
        if (it == m_CharMap.end())
            continue;
        
        const GLCharInfo& ch = it->second;
        
        // 计算顶点位置
        float xpos = startX + ch.bearingX * scale;
        float ypos = y - (ch.height - ch.bearingY) * scale;
        
        float w = ch.width * scale;
        float h = ch.height * scale;
        
        // 更新VBO数据
        float vertices[6][4] = {
            { xpos,     ypos + h,   ch.s0, ch.t1 },
            { xpos,     ypos,       ch.s0, ch.t0 },
            { xpos + w, ypos,       ch.s1, ch.t0 },
            
            { xpos,     ypos + h,   ch.s0, ch.t1 },
            { xpos + w, ypos,       ch.s1, ch.t0 },
            { xpos + w, ypos + h,   ch.s1, ch.t1 }
        };
        
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // 渲染四边形
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // 移动到下一个字符位置
        startX += (ch.advance >> 6) * scale;  // 位转换为像素
    }
    
    // 清理
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // 恢复状态
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}

float OpenGLFont::GetTextWidth(const std::string& text, float scale)
{
    float width = 0.0f;
    
    for (char c : text)
    {
        auto it = m_CharMap.find(static_cast<unsigned char>(c));
        if (it != m_CharMap.end())
        {
            width += (it->second.advance >> 6) * scale;
        }
    }
    
    return width;
}

float OpenGLFont::GetTextHeight(const std::string& text, float scale)
{
    return m_LineHeight * scale;
}

void OpenGLFont::Cleanup()
{
    // 清理OpenGL资源
    if (m_TextureAtlas)
    {
        glDeleteTextures(1, &m_TextureAtlas);
        m_TextureAtlas = 0;
    }
    
    if (m_VAO)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    
    if (m_VBO)
    {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    
    // 清理GDI资源
    if (m_hBitmap)
    {
        DeleteObject(m_hBitmap);
        m_hBitmap = nullptr;
    }
    
    if (m_hFont)
    {
        DeleteObject(m_hFont);
        m_hFont = nullptr;
    }
    
    if (m_hDC)
    {
        DeleteDC(m_hDC);
        m_hDC = nullptr;
    }
    
    m_CharMap.clear();
}