
// ======================================================================
#include "stdafx.h"

#include <sstream>
#include <cassert>
#include <iomanip>
#include "Core/Renderer.h"
#include "Math/MathUtils.h"
// ======================================================================

// ======================================================================
// ==================== 公有方法实现 =====================================

CRenderer::CRenderer()
    : m_hWnd(nullptr),                  // 窗口句柄
      m_hDC(nullptr),                   // 设备上下文
      m_hRC(nullptr),                   // OpenGL渲染上下文
      m_Width(0),                       // 渲染区宽度
      m_Height(0),                      // 渲染去高度
      m_AspectRatio(0.0f),              // 宽高比
      m_VSyncEnabled(TRUE),             // 垂直同步是否开启
      m_FrameCount(0),                  // 帧计数器
      m_LastFPSUpdate(0),               // 上次FPS更新时间
      m_FPS(0),                         // 默认帧率
      m_FPSSmooth(0.0f),                // 初始化平滑值
      m_FPSSmoothingFactor(0.2f),       // 设置默认平滑系数
      m_GLInitialized(FALSE),           // OpenGL是否已初始化
      m_WGLSwapControlSupported(FALSE), // 交换控制是否支持
      m_NextIndex(0),                   // 环形缓冲区索引归零
      m_DeltaTime(0.0f),                // 帧间隔时间
      m_MinDeltaTime(0.0f),             // 最小帧时间
      m_MaxDeltaTime(0.1f)              // 最大帧时间, 100ms 阈值
{
    // TODO: 初始化清除颜色
    // m_ClearColor[0] = 0.2f; // R
    // m_ClearColor[1] = 0.3f; // G
    // m_ClearColor[2] = 0.8f; // B
    // m_ClearColor[3] = 1.0f; // A
    m_ClearColor[0] = 1.0f; // R
    m_ClearColor[1] = 1.0f; // G
    m_ClearColor[2] = 1.0f; // B
    m_ClearColor[3] = 1.0f; // A

    m_LastCounter.QuadPart = 0;

    // 重点：初始化帧耗时历史记录
    // 填充 0.0166f 可以让程序启动即拥有平滑的平均 FPS 数据
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        m_FrameTimeHistory[i] = 0.0166f;
    }
}

CRenderer::~CRenderer()
{
    Shutdown();
}

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
    // 告诉Windows需要什么样的图形缓冲区
    if (!SetupPixelFormat())
    {
        MessageBoxW(NULL, L"设置像素格式失败!", L"渲染器错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 3. 创建OpenGL渲染上下文
    // OpenGL的工作环境, 记录状态
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
    // wglSwapIntervalEXT 必须在 OpenGL 上下文 (HGLRC) 绑定后 才能通过 wglGetProcAddress 获取。
    // 但如果在切换全屏/窗口时重新初始化，需确保之前的状态被正确清理。

    // 7. 设置渲染状态
    SetupRenderState();

    // 8. 获取窗口客户区大小
    RECT clientRect = {};
    GetClientRect(m_hWnd, &clientRect);
    m_Width = clientRect.right - clientRect.left;
    m_Height = clientRect.bottom - clientRect.top;
    m_AspectRatio = (m_Height > 0) ? static_cast<FLOAT>(m_Width) / static_cast<FLOAT>(m_Height) : 1.0f;

    // 9. 设置视口
    // 定义窗口坐标的NDC标准化设备坐标的映射
    // 告诉OpenGL在窗口的哪个区域绘制
    SetViewport(0, 0, m_Width, m_Height);

    // 10. 设置默认投影矩阵
    // 定义3D到2D的投影变换
    // 透视投影模拟人眼视角
    // SetOrthoProjection(FLOAT left, FLOAT right, FLOAT bottom, FLOAT top, FLOAT zNear, FLOAT zFar);
    SetPerspectiveProjection(45.0f, m_AspectRatio, 0.1f, 1000.0f);

    // 输出OpenGL信息
    std::string info = GetGLInfo();
    OutputDebugStringA(info.c_str());

    // 初始化高精度计时器
    static LARGE_INTEGER s_Frequency;
    QueryPerformanceFrequency(&s_Frequency);
    QueryPerformanceCounter(&m_LastCounter);

    // 初始化 FPS 统计时间
    m_LastFPSUpdate = GetTickCount();
    m_LastFPSTime = std::chrono::high_resolution_clock::now();

    m_GLInitialized = TRUE;

    return TRUE;
}

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

void CRenderer::BeginFrame()
{
    // std::cout << "Rendering..." << std::endl;
    if (!m_GLInitialized)
        return;

    glViewport(0, 0, m_Width, m_Height);

    // 1. 确保写入权限开启（防止 Clear 无效）
    // 写入掩码函数, 控制哪些缓冲区可以被写入
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // 2. 清除缓冲区
    Clear(TRUE, TRUE, FALSE);
}

void CRenderer::EndFrame()
{
    if (!m_GLInitialized)
        return;

    // 1. 错误检查 (Debug 模式下非常有用)
#ifdef MYDEBUG
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    { /* 记录错误日志 */
    }
#endif

    // 2. 绘制调试信息 (Debug Overlay)
    // 渲染 FPS 等文字通常放在交换缓冲之前的最后一步
    // RenderDebugInfo()

    // 3. 交换缓冲区
    SwapBuffers(m_hDC);

    // 4. 更新帧率统计
    UpdateFPS();

    // 5. 状态收尾
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void CRenderer::Reset(INT width, INT height)
{
    // OutputDebugStringA("--- [Debug] CRenderer::Reset 真正开始了 ---\n");

    // 1. 极小值保护
    if (width <= 0 || height <= 0)
        return;

    if (!m_GLInitialized)
    {
        OutputDebugStringA("!!! [Error] Reset 失败：m_GLInitialized 是 FALSE\n");
        return;
    }

    // char buffer[256];
    // sprintf_s(buffer, sizeof(buffer),
    //           "渲染器重置: %dx%d\n", width, height);
    // OutputDebugStringA(buffer);

    // 3. 确保上下文在这个线程是激活的 (如果是在主线程回调中)
    // wglMakeCurrent(m_hDC, m_hGLRC);

    m_Width = width;
    m_Height = height;
    m_AspectRatio = (FLOAT)width / (FLOAT)height;

    // m_AspectRatio = (height > 0) ? static_cast<FLOAT>(width) / static_cast<FLOAT>(height) : 1.0f;

    // 更新视口
    SetViewport(0, 0, m_Width, m_Height);

    // // 更新投影矩阵
    // SetPerspectiveProjection(45.0f, m_AspectRatio, 0.1f, 1000.0f);

    // 3. 清除OpenGL错误
    GLenum error = glGetError();
    // if (error != GL_NO_ERROR)
    // {
    //     sprintf_s(buffer, sizeof(buffer),
    //               "OpenGL错误在Reset后: 0x%X\n", error);
    //     OutputDebugStringA(buffer);
    // }
}

void CRenderer::SetClearColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
{
    m_ClearColor[0] = r;
    m_ClearColor[1] = g;
    m_ClearColor[2] = b;
    m_ClearColor[3] = a;

    glClearColor(r, g, b, a);
    CheckGLError("glClearColor");
}

void CRenderer::SetClearColor(const FLOAT color[4])
{
    SetClearColor(color[0], color[1], color[2], color[3]);
}

void CRenderer::SetVerticalSync(BOOL enable)
{
    InternalSetVerticalSync(enable);
}

void CRenderer::ToggleVerticalSync()
{
    SetVerticalSync(!m_VSyncEnabled);
}

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

void CRenderer::SetBlendFunc(GLenum srcFactor, GLenum dstFactor)
{
    glBlendFunc(srcFactor, dstFactor);
    CheckGLError("glBlendFunc");
}

void CRenderer::SetViewport(INT x, INT y, INT width, INT height)
{
    glViewport(x, y, width, height);
    CheckGLError("glViewport");

    // 调试打印：确认坐标真的传进去了
    // char buf[128];
    // sprintf_s(buf, "glViewport 已设置为: %d, %d, %d, %d\n", x, y, width, height);
    // OutputDebugStringA(buf);
}

void CRenderer::SetPerspectiveProjection(FLOAT fovY, FLOAT aspect, FLOAT zNear, FLOAT zFar)
{
    // 投影矩阵
    // | f/a  0        0        0 |
    // | 0    f        0        0 |
    // | 0    0  (fz+n)/(n-fz) -1 |
    // | 0    0  (2fz*n)/(n-fz) 0 |

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); // 将当前矩阵设置为单位矩阵

    // FLOAT f = 1.0f / tan(fovY * 0.5f * Math::PI / 180.0f);
    FLOAT f = 1.0f / Math::Tan(fovY * Math::PI / 360.0f);

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

void CRenderer::SetOrthoProjection(FLOAT left, FLOAT right, FLOAT bottom, FLOAT top, FLOAT zNear, FLOAT zFar)
{
    // |      2/(r-l)         0             0      0 |
    // |       0             2/(t-b)        0      0 |
    // |       0              0           -2/(f-n) 0 |
    // | -(r+l)/(r-l)   -(t+b)/t-b    -(f+n)/(f-n) 1 |

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

void CRenderer::Clear(BOOL clearColor, BOOL clearDepth, BOOL clearStencil)
{
    GLbitfield mask = 0;

    if (clearColor)
        // 颜色缓冲区
        mask |= GL_COLOR_BUFFER_BIT;
    if (clearDepth)
        // 深度缓冲区
        mask |= GL_DEPTH_BUFFER_BIT;
    if (clearStencil)
        // 模板缓冲区
        mask |= GL_STENCIL_BUFFER_BIT;

    if (mask != 0)
    {
        glClear(mask);
        CheckGLError("glClear");
    }
}

void CRenderer::AddFrameSample(FLOAT dt)
{
    m_FrameTimeHistory[m_NextIndex] = dt;

    // 关键逻辑：索引递增，到末尾自动归零
    m_NextIndex = (m_NextIndex + 1) % SAMPLE_COUNT;
}

FLOAT CRenderer::GetAverageFrameTime() const
{
    FLOAT sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; ++i)
    {
        sum += m_FrameTimeHistory[i];
    }
    return sum / SAMPLE_COUNT;
}

FLOAT CRenderer::GetSmoothFPS() const
{
    FLOAT avgTime = GetAverageFrameTime();
    return (avgTime > 0.0f) ? (1.0f / avgTime) : 0.0f;
}

void CRenderer::PushState()
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    CheckGLError("PushState");
}

void CRenderer::PopState()
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
    CheckGLError("PopState");
}

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

std::string CRenderer::GetGLInfo() const
{
    std::ostringstream oss;
    oss << "|====================== OpenGL 信息 ======================\n";
    oss << "|OpenGL版本: " << GetGLVersion() << "\n";
    oss << "|渲染器: " << GetGLRenderer() << "\n";
    oss << "|供应商: " << GetGLVendor() << "\n";
    oss << "|窗口尺寸: " << m_Width << "x" << m_Height << "\n";
    oss << "|宽高比: " << std::fixed << std::setprecision(2) << m_AspectRatio << "\n";
    oss << "|垂直同步: " << (m_VSyncEnabled ? "启用" : "禁用") << "\n";
    oss << "|=========================================================\n";

    return oss.str();
}

// ======================================================================
// ==================== 私有方法实现 =====================================

BOOL CRenderer::SetupPixelFormat()
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR), // 结构大小
        1,                             // 版本号
        PFD_DRAW_TO_WINDOW |           // 支持窗口
            PFD_SUPPORT_OPENGL |       // 支持OpenGL
            PFD_DOUBLEBUFFER,          // 双缓冲
        PFD_TYPE_RGBA,                 // RGBA类型
        32,                            // 32位颜色深度
        0, 0, 0, 0, 0, 0,              // 颜色位忽略
        0,                             // 无alpha缓冲区
        0,                             // 忽略移位
        0,                             // 无累积缓冲区
        0, 0, 0, 0,                    // 累积位忽略
        24,                            // 24位深度缓冲区
        8,                             // 8位模板缓冲区
        0,                             // 无辅助缓冲区
        PFD_MAIN_PLANE,                // 主平面
        0,                             // 保留
        0, 0, 0                        // 层掩码忽略
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

void CRenderer::CheckGLExtensions()
{
    // 检查WGL扩展支持
    const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
    // 检查是否支持垂直同步控制
    if (extensions && strstr(extensions, "WGL_EXT_swap_control"))
    {
        m_WGLSwapControlSupported = TRUE;
    }

    CheckGLError("CheckGLExtensions");
}

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

    // 启用多重采样抗锯齿
    // 如果硬件和像素格式支持，这将显著平滑远处的网格线
    glEnable(GL_MULTISAMPLE);

    // 设置混合函数
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    CheckGLError("SetupRenderState");
}

void CRenderer::UpdateFPS()
{
    if (!m_GLInitialized)
        return;

    // 1. 获取当前高精度时间
    static LARGE_INTEGER s_Frequency;
    if (s_Frequency.QuadPart == 0)
        QueryPerformanceFrequency(&s_Frequency);

    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);

    // 3. 计算自上一帧以来的时间增量 (DeltaTime)
    FLOAT rawDeltaTime = static_cast<FLOAT>(currentTime.QuadPart - m_LastCounter.QuadPart) / s_Frequency.QuadPart;
    m_LastCounter = currentTime;

    // 3. 异常帧剔除逻辑
    // 如果一帧耗时超过 0.1 秒 (10 FPS)，通常是拖动窗口或卡顿，不应让其破坏物理模拟
    if (rawDeltaTime > m_MaxDeltaTime || rawDeltaTime < 0.0f)
    {
        m_DeltaTime = 0.0166f; // 默认 60fps 的步长
    }
    else
    {
        m_DeltaTime = rawDeltaTime;
    }

    // 4. 存入环形缓冲区
    AddFrameSample(m_DeltaTime);
    m_FrameCount++;

    // 3. 每秒更新一次 FPS 显示
    DWORD now = GetTickCount();
    if (now - m_LastFPSUpdate >= 1000)
    {
        // 原始 FPS
        m_FPS = (FLOAT)m_FrameCount;

        // 平滑 FPS 计算 (指数移动平均 EMA 算法)
        // m_FPSSmooth = m_FPS * alpha + m_FPSSmooth * (1 - alpha)
        // 这里的 alpha 也就是你的 m_FPSSmoothingFactor (例如取 0.1f)
        FLOAT alpha = 0.2f;
        m_FPSSmooth = (m_FPS * alpha) + (m_FPSSmooth * (1.0f - alpha));

        // 重置计数
        m_FrameCount = 0;
        m_LastFPSUpdate = now;
    }
}

BOOL CRenderer::CheckGLError(const char *function)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::string errorMsg;
        switch (error)
        {
        case GL_INVALID_ENUM:
            errorMsg = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errorMsg = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errorMsg = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            errorMsg = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            errorMsg = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            errorMsg = "GL_OUT_OF_MEMORY";
            break;
        default:
            errorMsg = "Unknown error";
            break;
        }

        char buffer[256];
        sprintf_s(buffer, sizeof(buffer),
                  "OpenGL错误: %s\n函数: %s\n", errorMsg.c_str(), function);
        OutputDebugStringA(buffer);

        return TRUE;
    }
    return FALSE;
}

BOOL CRenderer::InternalSetVerticalSync(BOOL enable)
{
    m_VSyncEnabled = enable;

    if (m_WGLSwapControlSupported)
    {
        typedef BOOL(WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int);
        PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT =
            (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

        if (wglSwapIntervalEXT)
        {
            return wglSwapIntervalEXT(enable ? 1 : 0);
        }
    }

    return FALSE;
}

std::string CRenderer::GetGLVersion() const
{
    const GLubyte *version = glGetString(GL_VERSION);
    if (version)
    {
        return reinterpret_cast<const char *>(version);
    }
    return "Unknown";
}

std::string CRenderer::GetGLRenderer() const
{
    const GLubyte *renderer = glGetString(GL_RENDERER);
    if (renderer)
    {
        return reinterpret_cast<const char *>(renderer);
    }
    return "Unknown";
}

std::string CRenderer::GetGLVendor() const
{
    const GLubyte *vendor = glGetString(GL_VENDOR);
    if (vendor)
    {
        return reinterpret_cast<const char *>(vendor);
    }
    return "Unknown";
}