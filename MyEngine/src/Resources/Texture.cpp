
// ======================================================================
#include "stdafx.h"
#include "Resources/Texture.h"
#include "Utils/stb_image.h"
#include "Utils/StringUtils.h"
// ======================================================================

// ======================================================================
// ==================== 公有方法 =========================================
// ======================================================================

CTexture::CTexture()
    : m_TextureID(0), // 纹理ID
      m_Width(0),     // 纹理宽度
      m_Height(0),    // 纹理高度
      m_Channels(0),  // 通道数
      m_Path(L"")     // 路径
{
}

// 用于 CreateEmpty 场景
CTexture::CTexture(INT width, INT height)
    : m_TextureID(0),          // 纹理ID
      m_Width(width),          // 纹理宽度
      m_Height(height),        // 纹理高度
      m_Channels(0),           // 通道数
      m_Path(L"MemoryTexture") // 路径
{
}

CTexture::~CTexture()
{
    Cleanup();
}

CTexture::CTexture(CTexture &&other)
    // CTexture::CTexture(CTexture &&other) noexcept
    : m_TextureID(other.m_TextureID), // 纹理ID
      m_Width(other.m_Width),         // 纹理宽度
      m_Height(other.m_Height),       // 纹理高度
      m_Channels(other.m_Channels),   // 通道数
      m_Path(std::move(other.m_Path)) // 文件路径
{
    // 将原对象置为无效状态
    other.m_TextureID = 0;
    other.m_Width = 0;
    other.m_Height = 0;
    other.m_Channels = 0;
    other.m_Path.clear();
}

CTexture &CTexture::operator=(CTexture &&other)
// CTexture &CTexture::operator=(CTexture &&other) noexcept
{
    if (this != &other)
    {
        Cleanup(); // 清理当前资源

        // 转移资源
        m_TextureID = other.m_TextureID;
        m_Width = other.m_Width;
        m_Height = other.m_Height;
        m_Channels = other.m_Channels;
        m_Path = std::move(other.m_Path);

        // 将原对象置为无效状态
        other.m_TextureID = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_Channels = 0;
        other.m_Path.clear();
    }
    return *this;
}

BOOL CTexture::LoadFromFile(const std::wstring &filePath)
{
    // 0. 清理现有纹理
    Cleanup();
    std::wstring fullPath = filePath;
    m_Path = fullPath;

    // 1. 使用 stb_image 加载数据
    std::string narrowPath = CStringUtils::WStringToString(fullPath, CP_UTF8);
    // 强制翻转 Y 轴，因为 OpenGL 的原点在左下角，而图片在左上角
    // 重要：解决OpenGL坐标系与图片坐标系Y轴方向不一致的问题
    // OpenGL：(0,0)在左下角
    // 图片：(0,0)在左上角
    if (narrowPath.empty())
    {
        LogError(L"路径转换失败，无法处理路径: %ls.\n", fullPath.c_str());
        return FALSE;
    }

    stbi_set_flip_vertically_on_load(false);
    unsigned char *data = stbi_load(narrowPath.c_str(), &m_Width, &m_Height, &m_Channels, 0);

    if (!data)
    {
        const char *failReason = stbi_failure_reason();
        std::wstring wReason = CStringUtils::StringToWString(failReason ? failReason : "未知原因");

        if (!PathUtils::Exists(fullPath))
        {
            LogError(L"无法加载纹理: 文件不存在. 路径: %ls\n", filePath.c_str());
        }
        else
        {
            LogError(L"无法加载纹理: %ls. 路径: %ls\n", wReason.c_str(), filePath.c_str());
        }
        return FALSE;
    }

    // 2. 确定格式
    GLenum format = GL_RGB;
    GLenum internalFormat = GL_RGB8;
    switch (m_Channels)
    {
    case 1:
        format = GL_RED;
        internalFormat = GL_R8;
        break;
    case 3:
        format = GL_RGB;
        internalFormat = GL_RGB8;
        break;
    case 4:
        format = (m_Channels == 4) ? GL_RGBA : GL_RGB;
        internalFormat = GL_RGBA8;
        break;
    default:
        LogWarning(L"不支持的通道数: %d", m_Channels);
        stbi_image_free(data);
        return FALSE;
    }

    // 3. 生成 OpenGL 纹理
    glGenTextures(1, &m_TextureID);
    if (m_TextureID == 0)
    {
        LogError(L"glGenTextures失败\n");
        stbi_image_free(data);
        return FALSE;
    }
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    // 4. 设置默认纹理参数 包裹和过滤参数（固定管线常用设置）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 5. 上传数据
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);

    // 6. 上传图像数据并生成 Mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // 7. 释放内存
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

BOOL CTexture::LoadFromMemory(const unsigned char *data, INT width, INT height, INT channels, GLenum format)
{
    Cleanup();

    m_Width = width; m_Height = height; m_Channels = channels;

    if (width <= 0 || height <= 0 || channels <= 0 || channels > 4)
    {
        LogError(L"内存纹理参数无效: %dx%dx%d.\n", width, height, channels); return FALSE;
    }

    // 确定内部格式
    GLenum internalFormat = GL_RGB8;
    switch (channels)
    {
    case 1: internalFormat = GL_R8; break;
    case 2: internalFormat = GL_RG8; break;
    case 3: internalFormat = GL_RGB8; break;
    case 4: internalFormat = GL_RGBA8;break;
    }

    // 生成纹理
    glGenTextures(1, &m_TextureID);
    if (m_TextureID == 0)
    {
        LogError(L"glGenTextures失败"); return FALSE;
    }

    SetDefaultParameters();
    return UploadToGPU(data, format, internalFormat);
}

BOOL CTexture::CreateEmpty(INT width, INT height, GLenum internalFormat, GLenum format, GLenum dataType)
{
    Cleanup();

    m_Width = width;
    m_Height = height;

    // 根据格式推断通道数
    switch (format)
    {
    case GL_RED:
        m_Channels = 1;
        break;
    case GL_RG:
        m_Channels = 2;
        break;
    case GL_RGB:
        m_Channels = 3;
        break;
    case GL_RGBA:
        m_Channels = 4;
        break;
    default:
        m_Channels = 4;
        break;
    }

    glGenTextures(1, &m_TextureID);
    if (m_TextureID == 0)
    {
        std::cerr << "glGenTextures失败" << std::endl;
        return FALSE;
    }

    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, nullptr);

    SetDefaultParameters();
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void CTexture::Bind(GLenum textureUnit) const
{
    if (!IsValid())
    {
        LogError(L"尝试绑定无效纹理\n");
        return;
    }

    // 激活纹理单元
    glActiveTexture(textureUnit);

    // 绑定纹理
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    // 强制设置对齐为 1，解决非 4 倍数宽度图片花屏问题
    // glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 固定管线需要启用纹理
    glEnable(GL_TEXTURE_2D);
}

void CTexture::Unbind(GLenum textureUnit) const
{
    glActiveTexture(textureUnit);

    glBindTexture(GL_TEXTURE_2D, 0);

    glDisable(GL_TEXTURE_2D);
}

void CTexture::SetWrapMode(GLenum sWrap, GLenum tWrap)
{
    if (!IsValid())
        return;

    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, tWrap);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CTexture::SetFilterMode(GLenum minFilter, GLenum magFilter)
{
    if (!IsValid())
        return;

    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CTexture::SetAnisotropy(FLOAT level)
{
    if (!IsValid())
        return;

    // 检查是否支持各向异性过滤
    const GLubyte *extensions = glGetString(GL_EXTENSIONS);
    if (extensions && strstr(reinterpret_cast<const char *>(extensions), "GL_EXT_texture_filter_anisotropic"))
    {
        float maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        float anisotropy = std::min(level, maxAnisotropy);

        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void CTexture::SetFlipVerticallyOnLoad(BOOL flip)
{
    stbi_set_flip_vertically_on_load(flip);
}

// ======================================================================
// ==================== 私有方法 =========================================
// ======================================================================

void CTexture::Cleanup()
{
    if (m_TextureID != 0)
    {
        glDeleteTextures(1, &m_TextureID);
        m_TextureID = 0;
    }
    m_Width = 0;
    m_Height = 0;
    m_Channels = 0;
    m_Path.clear();
}

void CTexture::SetDefaultParameters()
{
    if (m_TextureID == 0)
        return;

    // 绑定纹理以设置参数
    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    // 默认包裹模式：重复
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // 默认过滤模式：线性过滤和mipmap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // 设置各向异性过滤
    if (glGetString(GL_EXTENSIONS) &&
        strstr(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)), "GL_EXT_texture_filter_anisotropic"))
    {
        float maxAnisotropy = 1.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        float anisotropy = std::min(4.0f, maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

BOOL CTexture::UploadToGPU(const unsigned char *data, GLenum format, GLenum internalFormat)
{
    if (m_TextureID == 0)
    {
        LogError(L"纹理ID无效\n");
        return FALSE;
    }

    if (m_Width <= 0 || m_Height <= 0)
    {
        LogError(L"纹理尺寸无效: %dx%d\n", m_Width, m_Height);
        return FALSE;
    }

    glBindTexture(GL_TEXTURE_2D, m_TextureID);

    // 上传纹理数据
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, format, GL_UNSIGNED_BYTE, data);

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        LogError(L"glTexImage2D错误: %d.\n", error);
        glBindTexture(GL_TEXTURE_2D, 0);
        return FALSE;
    }

    // 生成mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    error = glGetError();
    if (error != GL_NO_ERROR)
    {
        LogError(L"glGenerateMipmap错误: %d.\n", error);
        glBindTexture(GL_TEXTURE_2D, 0);
        return FALSE;
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}