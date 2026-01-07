
#include "stdafx.h"
#include "Core/Renderer.h"
#include <cassert>
#include <sstream>
#include <iomanip>

/**
 * @brief 构造函数
 */
CRenderer::CRenderer()
    : m_hWnd(nullptr)
    , m_hDC(nullptr)
    , m_hRC(nullptr)
    , m_Width(0)
    , m_Height(0)
    , m_AspectRatio(0.0f)
    , m_VSyncEnabled(TRUE)
    , m_FrameCount(0)
    , m_LastFPSUpdate(0)
    , m_FPS(0)
    , m_GLInitialized(FALSE)
    , m_WGLSwapControlSupported(FALSE)
{
    // 初始化清除颜色
    m_ClearColor[0] = 0.2f;  // R
    m_ClearColor[1] = 0.3f;  // G
    m_ClearColor[2] = 0.5f;  // B
    m_ClearColor[3] = 1.0f;  // A
}

/**
 * @brief 析构函数
 */
CRenderer::~CRenderer()
{
    Shutdown();
}

/**
 * @brief 初始化渲染器
 */
BOOL CRenderer::Initialize(HWND hWnd)
{
    assert(hWnd != nullptr);
    
    m_hWnd = hWnd;
    
    // 1. 获取设备上下文
    m_hDC = GetDC(m_hWnd);
    if (!m_hDC)
    {
        MessageBoxW(NULL, L"获取设备上下文失败!", L"渲染器错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    
    // 2. 设置像素格式
    if (!SetupPixelFormat())
    {
        MessageBoxW(NULL, L"设置像素格式失败!", L"渲染器错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    
    // 3. 创建OpenGL渲染上下文
    m_hRC = wglCreateContext(m_hDC);
    if (!m_hRC)
    {
        MessageBoxW(NULL, L"创建OpenGL渲染上下文失败!", L"渲染器错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    
    // 4. 设置当前渲染上下文
    if (!wglMakeCurrent(m_hDC, m_hRC))
    {
        MessageBoxW(NULL, L"设置当前渲染上下文失败!", L"渲染器错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    
    // 5. 检查OpenGL扩展
    CheckGLExtensions();
    
    // 6. 设置垂直同步
    InternalSetVerticalSync(m_VSyncEnabled);
    
    // 7. 设置渲染状态
    SetupRenderState();
    
    // 8. 获取窗口客户区大小
    RECT clientRect = {};
    GetClientRect(m_hWnd, &clientRect);
    m_Width = clientRect.right - clientRect.left;
    m_Height = clientRect.bottom - clientRect.top;
    m_AspectRatio = (m_Height > 0) ? static_cast<FLOAT>(m_Width) / static_cast<FLOAT>(m_Height) : 1.0f;
    
    // 9. 设置视口
    SetViewport(0, 0, m_Width, m_Height);
    
    // 10. 设置默认投影矩阵
    SetPerspectiveProjection(45.0f, m_AspectRatio, 0.1f, 1000.0f);
    
    m_GLInitialized = TRUE;
    
    // 输出OpenGL信息
    std::string info = GetGLInfo();
    OutputDebugStringA(info.c_str());
    
    return TRUE;
}

/**
 * @brief 关闭渲染器
 */
void CRenderer::Shutdown()
{
    if (m_GLInitialized)
    {
        // 重置渲染上下文
        wglMakeCurrent(nullptr, nullptr);
        
        // 删除渲染上下文
        if (m_hRC)
        {
            wglDeleteContext(m_hRC);
            m_hRC = nullptr;
        }
        
        // 释放设备上下文
        if (m_hDC && m_hWnd)
        {
            ReleaseDC(m_hWnd, m_hDC);
            m_hDC = nullptr;
        }
        
        m_GLInitialized = FALSE;
    }
    
    m_hWnd = nullptr;
}

/**
 * @brief 开始渲染一帧
 */
void CRenderer::BeginFrame()
{
    if (!m_GLInitialized)
        return;
    
    // 清除缓冲区
    Clear();
    
    // 重置模型视图矩阵
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/**
 * @brief 结束渲染一帧
 */
void CRenderer::EndFrame()
{
    if (!m_GLInitialized)
        return;
    
    // 交换缓冲区
    SwapBuffers(m_hDC);
    
    // 更新帧率统计
    UpdateFPS();
}

/**
 * @brief 重置渲染器
 */
void CRenderer::Reset(INT width, INT height)
{
    if (!m_GLInitialized)
        return;
    
    m_Width = width;
    m_Height = height;
    m_AspectRatio = (height > 0) ? static_cast<FLOAT>(width) / static_cast<FLOAT>(height) : 1.0f;
    
    // 更新视口
    SetViewport(0, 0, m_Width, m_Height);
    
    // 更新投影矩阵
    SetPerspectiveProjection(45.0f, m_AspectRatio, 0.1f, 1000.0f);
}

/**
 * @brief 设置清除颜色
 */
void CRenderer::SetClearColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
{
    m_ClearColor[0] = r;
    m_ClearColor[1] = g;
    m_ClearColor[2] = b;
    m_ClearColor[3] = a;
    
    glClearColor(r, g, b, a);
    CheckGLError("glClearColor");
}

/**
 * @brief 设置清除颜色（数组形式）
 */
void CRenderer::SetClearColor(const FLOAT color[4])
{
    SetClearColor(color[0], color[1], color[2], color[3]);
}

/**
 * @brief 切换垂直同步状态
 */
void CRenderer::ToggleVerticalSync()
{
    SetVerticalSync(!m_VSyncEnabled);
}

/**
 * @brief 设置线框模式
 */
void CRenderer::SetWireframeMode(BOOL enable)
{
    if (enable)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    CheckGLError("glPolygonMode");
}

/**
 * @brief 设置背面剔除
 */
void CRenderer::SetBackfaceCulling(BOOL enable)
{
    if (enable)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }
    CheckGLError("glCullFace");
}

/**
 * @brief 设置深度测试
 */
void CRenderer::SetDepthTest(BOOL enable)
{
    if (enable)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
    CheckGLError("glDepthFunc");
}

/**
 * @brief 设置混合模式
 */
void CRenderer::SetBlending(BOOL enable)
{
    if (enable)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
    CheckGLError("glBlend");
}

/**
 * @brief 设置混合函数
 */
void CRenderer::SetBlendFunc(GLenum srcFactor, GLenum dstFactor)
{
    glBlendFunc(srcFactor, dstFactor);
    CheckGLError("glBlendFunc");
}

/**
 * @brief 设置视口
 */
void CRenderer::SetViewport(INT x, INT y, INT width, INT height)
{
    glViewport(x, y, width, height);
    CheckGLError("glViewport");
}

/**
 * @brief 设置投影矩阵（透视投影）
 */
void CRenderer::SetPerspectiveProjection(FLOAT fovY, FLOAT aspect, FLOAT zNear, FLOAT zFar)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    FLOAT f = 1.0f / tan(fovY * 0.5f * 3.14159265f / 180.0f);
    
    FLOAT m[16] = {0};
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (zFar + zNear) / (zNear - zFar);
    m[11] = -1.0f;
    m[14] = (2.0f * zFar * zNear) / (zNear - zFar);
    m[15] = 0.0f;
    
    glLoadMatrixf(m);
    CheckGLError("SetPerspectiveProjection");
}

/**
 * @brief 设置投影矩阵（正交投影）
 */
void CRenderer::SetOrthoProjection(FLOAT left, FLOAT right, FLOAT bottom, FLOAT top, FLOAT zNear, FLOAT zFar)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    FLOAT m[16] = {0};
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (zFar - zNear);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(zFar + zNear) / (zFar - zNear);
    m[15] = 1.0f;
    
    glLoadMatrixf(m);
    CheckGLError("SetOrthoProjection");
}

/**
 * @brief 清除缓冲区
 */
void CRenderer::Clear(BOOL clearColor, BOOL clearDepth, BOOL clearStencil)
{
    GLbitfield mask = 0;
    
    if (clearColor)
        mask |= GL_COLOR_BUFFER_BIT;
    if (clearDepth)
        mask |= GL_DEPTH_BUFFER_BIT;
    if (clearStencil)
        mask |= GL_STENCIL_BUFFER_BIT;
    
    if (mask != 0)
    {
        glClear(mask);
        CheckGLError("glClear");
    }
}

/**
 * @brief 保存当前OpenGL状态
 */
void CRenderer::PushState()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    CheckGLError("PushState");
}

/**
 * @brief 恢复保存的OpenGL状态
 */
void CRenderer::PopState()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
    CheckGLError("PopState");
}

/**
 * @brief 重置渲染器到默认状态
 */
void CRenderer::ResetState()
{
    // 重置矩阵
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // 重置颜色
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    
    // 重置纹理
    glDisable(GL_TEXTURE_2D);
    
    // 重置光照
    glDisable(GL_LIGHTING);
    
    // 重置混合
    glDisable(GL_BLEND);
    
    // 重置线宽
    glLineWidth(1.0f);
    
    // 重置点大小
    glPointSize(1.0f);
    
    CheckGLError("ResetState");
}

/**
 * @brief 获取OpenGL信息
 */
std::string CRenderer::GetGLInfo() const
{
    std::ostringstream oss;
    oss << "=== OpenGL 信息 ===\n";
    oss << "OpenGL版本: " << GetGLVersion() << "\n";
    oss << "渲染器: " << GetGLRenderer() << "\n";
    oss << "供应商: " << GetGLVendor() << "\n";
    oss << "窗口尺寸: " << m_Width << "x" << m_Height << "\n";
    oss << "宽高比: " << std::fixed << std::setprecision(2) << m_AspectRatio << "\n";
    oss << "垂直同步: " << (m_VSyncEnabled ? "启用" : "禁用") << "\n";
    oss << "===================\n";
    
    return oss.str();
}

// ==================== 私有方法实现 ====================

/**
 * @brief 设置像素格式
 */
BOOL CRenderer::SetupPixelFormat()
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  // 结构大小
        1,                              // 版本号
        PFD_DRAW_TO_WINDOW |            // 支持窗口
        PFD_SUPPORT_OPENGL |            // 支持OpenGL
        PFD_DOUBLEBUFFER,               // 双缓冲
        PFD_TYPE_RGBA,                  // RGBA类型
        32,                             // 32位颜色深度
        0, 0, 0, 0, 0, 0,               // 颜色位忽略
        0,                              // 无alpha缓冲区
        0,                              // 忽略移位
        0,                              // 无累积缓冲区
        0, 0, 0, 0,                     // 累积位忽略
        24,                             // 24位深度缓冲区
        8,                              // 8位模板缓冲区
        0,                              // 无辅助缓冲区
        PFD_MAIN_PLANE,                 // 主平面
        0,                              // 保留
        0, 0, 0                         // 层掩码忽略
    };
    
    // 选择最匹配的像素格式
    INT pixelFormat = ChoosePixelFormat(m_hDC, &pfd);
    if (pixelFormat == 0)
    {
        return FALSE;
    }
    
    // 设置像素格式
    if (!SetPixelFormat(m_hDC, pixelFormat, &pfd))
    {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * @brief 检查OpenGL扩展
 */
void CRenderer::CheckGLExtensions()
{
    // 检查WGL扩展支持
    const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    if (extensions && strstr(extensions, "WGL_EXT_swap_control"))
    {
        m_WGLSwapControlSupported = TRUE;
    }
    
    CheckGLError("CheckGLExtensions");
}

/**
 * @brief 初始化OpenGL渲染状态
 */
void CRenderer::SetupRenderState()
{
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // 启用背面剔除
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // 设置多边形模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // 设置颜色
    glClearColor(m_ClearColor[0], m_ClearColor[1], m_ClearColor[2], m_ClearColor[3]);
    
    // 设置着色模型
    glShadeModel(GL_SMOOTH);
    
    // 启用归一化
    glEnable(GL_NORMALIZE);
    
    // 设置点/线平滑
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
    
    // 设置混合函数
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    CheckGLError("SetupRenderState");
}

/**
 * @brief 更新帧率统计
 */
void CRenderer::UpdateFPS()
{
    m_FrameCount++;
    
    DWORD currentTime = GetTickCount();
    DWORD elapsed = currentTime - m_LastFPSUpdate;
    
    if (elapsed >= 1000)  // 每1秒更新一次
    {
        m_FPS = (m_FrameCount * 1000) / elapsed;
        m_FrameCount = 0;
        m_LastFPSUpdate = currentTime;
    }
}

/**
 * @brief 检查OpenGL错误
 */
BOOL CRenderer::CheckGLError(const char* function)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::string errorMsg;
        switch (error)
        {
        case GL_INVALID_ENUM: errorMsg = "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE: errorMsg = "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: errorMsg = "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW: errorMsg = "GL_STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW: errorMsg = "GL_STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY: errorMsg = "GL_OUT_OF_MEMORY"; break;
        default: errorMsg = "Unknown error"; break;
        }
        
        char buffer[256];
        sprintf_s(buffer, sizeof(buffer), 
                 "OpenGL错误: %s\n函数: %s\n", errorMsg.c_str(), function);
        OutputDebugStringA(buffer);
        
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief 设置垂直同步
 */
// 公有方法的实现
void CRenderer::SetVerticalSync(BOOL enable)
{
    InternalSetVerticalSync(enable);
}
// 私有方法的实现
BOOL CRenderer::InternalSetVerticalSync(BOOL enable)
{
    m_VSyncEnabled = enable;
    
    if (m_WGLSwapControlSupported)
    {
        typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int);
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = 
            (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        
        if (wglSwapIntervalEXT)
        {
            return wglSwapIntervalEXT(enable ? 1 : 0);
        }
    }
    
    return FALSE;
}

/**
 * @brief 获取OpenGL版本信息
 */
std::string CRenderer::GetGLVersion() const
{
    const GLubyte* version = glGetString(GL_VERSION);
    if (version)
    {
        return reinterpret_cast<const char*>(version);
    }
    return "Unknown";
}

/**
 * @brief 获取OpenGL渲染器信息
 */
std::string CRenderer::GetGLRenderer() const
{
    const GLubyte* renderer = glGetString(GL_RENDERER);
    if (renderer)
    {
        return reinterpret_cast<const char*>(renderer);
    }
    return "Unknown";
}

/**
 * @brief 获取OpenGL供应商信息
 */
std::string CRenderer::GetGLVendor() const
{
    const GLubyte* vendor = glGetString(GL_VENDOR);
    if (vendor)
    {
        return reinterpret_cast<const char*>(vendor);
    }
    return "Unknown";
}