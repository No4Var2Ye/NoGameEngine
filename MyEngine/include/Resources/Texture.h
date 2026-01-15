// ======================================================================
#ifndef __TEXTURE_H__
#define __TEXTURE_H__
// ======================================================================
#include <string>
#include <GL/gl.h>
#include <GL/glext.h>
// ======================================================================

class CTexture
{
public:
    CTexture();
    CTexture(INT width, INT height);
    ~CTexture();

    // 禁用拷贝构造/赋值运算符
    CTexture(const CTexture &) = delete;
    CTexture &operator=(const CTexture &) = delete;

    // 移动语义
    CTexture(CTexture &&other);
    CTexture &operator=(CTexture &&other);
    // CTexture(CTexture &&other) noexcept;
    // CTexture &operator=(CTexture &&other) noexcept;

    // 加载纹理
    BOOL CreateTestTexture();
    BOOL LoadFromFile(const std::wstring &filePath); // 加载纹理逻辑
    BOOL LoadFromMemory(const unsigned char *data,
                        INT width, INT height,
                        INT channels, GLenum format = GL_RGBA); // 从内存加载纹理
    // 创建空纹理
    BOOL CreateEmpty(INT width, INT height,
                     GLenum internalFormat = GL_RGBA8,
                     GLenum format = GL_RGBA,
                     GLenum dataType = GL_UNSIGNED_BYTE);

    // 在固定管线渲染前调用
    void Bind(GLenum textureUnit = GL_TEXTURE0) const;   // 绑定
    void Unbind(GLenum textureUnit = GL_TEXTURE0) const; // 解绑

    // 参数配置
    void SetWrapMode(GLenum sWrap, GLenum tWrap);
    void SetFilterMode(GLenum minFilter, GLenum magFilter);
    void SetAnisotropy(FLOAT level = 4.0f); // 各向异性过滤

    // 获取属性
    void SetName(const std::wstring& name) { m_name = name; }
    const std::wstring& GetName() const { return m_name; }
    GLuint GetID() const { return m_TextureID; }
    INT GetWidth() const { return m_Width; }
    INT GetHeight() const { return m_Height; }
    INT GetChannels() const { return m_Channels; }
    BOOL IsValid() const { return m_TextureID != 0 && m_Width > 0 && m_Height > 0; }
    const std::wstring &GetPath() const { return m_Path; }

    // 工具函数
    static void SetFlipVerticallyOnLoad(BOOL flip);

private:
    std::wstring m_name;
    GLuint m_TextureID; // OpenGL纹理ID
    INT m_Width;        // 纹理宽度
    INT m_Height;       // 纹理高度
    INT m_Channels;     // 通道数
    std::wstring m_Path;

    void Cleanup();              // 清理资源
    void SetDefaultParameters(); // 设置纹理默认参数
    BOOL UploadToGPU(const unsigned char *data,
                     GLenum format,
                     GLenum internalFormat);
};

#endif // __TEXTURE_H__