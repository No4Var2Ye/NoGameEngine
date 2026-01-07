#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <Windows.h>
#include <gl/GL.h>
#include <string>

/**
 * @brief OpenGL渲染器类
 * @details 负责OpenGL初始化、渲染状态管理和帧缓冲交换
 */
class CRenderer
{
private:
    HWND m_hWnd = nullptr;        // 窗口句柄
    HDC m_hDC = nullptr;          // 设备上下文
    HGLRC m_hRC = nullptr;        // OpenGL渲染上下文
    
    INT m_Width = 0;              // 渲染区宽度
    INT m_Height = 0;             // 渲染区高度
    FLOAT m_AspectRatio = 0.0f;   // 宽高比
    
    FLOAT m_ClearColor[4];  // 清除颜色
    BOOL m_VSyncEnabled = TRUE;   // 垂直同步是否启用
    
    // 渲染统计
    DWORD m_FrameCount = 0;       // 帧计数器
    DWORD m_LastFPSUpdate = 0;    // 上次FPS更新时间
    DWORD m_FPS = 0;              // 当前帧率
    
    // OpenGL扩展功能支持标志
    BOOL m_GLInitialized = FALSE; // OpenGL是否已初始化
    BOOL m_WGLSwapControlSupported = FALSE;  // 交换控制是否支持
    
    /**
     * @brief 初始化OpenGL像素格式
     * @return 初始化成功返回TRUE，失败返回FALSE
     */
    BOOL SetupPixelFormat();
    
    /**
     * @brief 检查OpenGL扩展功能
     */
    void CheckGLExtensions();
    
    /**
     * @brief 初始化OpenGL渲染状态
     */
    void SetupRenderState();
    
    /**
     * @brief 更新帧率统计
     */
    void UpdateFPS();
    
    /**
     * @brief 检查OpenGL错误
     * @param function 发生错误的函数名
     * @return 有错误返回TRUE，无错误返回FALSE
     */
    BOOL CheckGLError(const char* function);
    
    /**
     * @brief 内部设置垂直同步
     * @param enable 是否启用垂直同步
     * @return 设置成功返回TRUE，失败返回FALSE
     */
    BOOL InternalSetVerticalSync(BOOL enable);
    
    /**
     * @brief 获取OpenGL版本信息
     * @return OpenGL版本字符串
     */
    std::string GetGLVersion() const;
    
    /**
     * @brief 获取OpenGL渲染器信息
     * @return 渲染器信息字符串
     */
    std::string GetGLRenderer() const;
    
    /**
     * @brief 获取OpenGL供应商信息
     * @return 供应商信息字符串
     */
    std::string GetGLVendor() const;
    
public:
    CRenderer();
    ~CRenderer();
    
    // 禁止拷贝构造和赋值
    CRenderer(const CRenderer&) = delete;
    CRenderer& operator=(const CRenderer&) = delete;
    
    /**
     * @brief 初始化渲染器
     * @param hWnd 窗口句柄
     * @return 初始化成功返回TRUE，失败返回FALSE
     */
    BOOL Initialize(HWND hWnd);
    
    /**
     * @brief 关闭渲染器
     */
    void Shutdown();
    
    /**
     * @brief 开始渲染一帧
     */
    void BeginFrame();
    
    /**
     * @brief 结束渲染一帧
     */
    void EndFrame();
    
    /**
     * @brief 重置渲染器（在窗口大小改变时调用）
     * @param width 新宽度
     * @param height 新高度
     */
    void Reset(INT width, INT height);
    
    /**
     * @brief 设置清除颜色
     * @param r 红色分量 (0.0-1.0)
     * @param g 绿色分量 (0.0-1.0)
     * @param b 蓝色分量 (0.0-1.0)
     * @param a 透明度分量 (0.0-1.0)，默认为1.0
     */
    void SetClearColor(FLOAT r, FLOAT g, FLOAT b, FLOAT a = 1.0f);
    
    /**
     * @brief 设置清除颜色（数组形式）
     * @param color RGBA颜色数组
     */
    void SetClearColor(const FLOAT color[4]);
    
    /**
     * @brief 设置垂直同步
     * @param enable 是否启用垂直同步
     */
    void SetVerticalSync(BOOL enable);
    
    /**
     * @brief 切换垂直同步状态
     */
    void ToggleVerticalSync();
    
    /**
     * @brief 设置线框模式
     * @param enable 是否启用线框模式
     */
    void SetWireframeMode(BOOL enable);
    
    /**
     * @brief 设置背面剔除
     * @param enable 是否启用背面剔除
     */
    void SetBackfaceCulling(BOOL enable);
    
    /**
     * @brief 设置深度测试
     * @param enable 是否启用深度测试
     */
    void SetDepthTest(BOOL enable);
    
    /**
     * @brief 设置混合模式
     * @param enable 是否启用混合模式
     */
    void SetBlending(BOOL enable);
    
    /**
     * @brief 设置混合函数
     * @param srcFactor 源因子
     * @param dstFactor 目标因子
     */
    void SetBlendFunc(GLenum srcFactor, GLenum dstFactor);
    
    /**
     * @brief 设置视口
     * @param x 视口左下角X坐标
     * @param y 视口左下角Y坐标
     * @param width 视口宽度
     * @param height 视口高度
     */
    void SetViewport(INT x, INT y, INT width, INT height);
    
    /**
     * @brief 设置投影矩阵（透视投影）
     * @param fovY 垂直视角（度）
     * @param aspect 宽高比
     * @param zNear 近裁剪面
     * @param zFar 远裁剪面
     */
    void SetPerspectiveProjection(FLOAT fovY, FLOAT aspect, FLOAT zNear, FLOAT zFar);
    
    /**
     * @brief 设置投影矩阵（正交投影）
     * @param left 左裁剪面
     * @param right 右裁剪面
     * @param bottom 下裁剪面
     * @param top 上裁剪面
     * @param zNear 近裁剪面
     * @param zFar 远裁剪面
     */
    void SetOrthoProjection(FLOAT left, FLOAT right, FLOAT bottom, FLOAT top, FLOAT zNear, FLOAT zFar);
    
    /**
     * @brief 清除缓冲区
     * @param clearColor 是否清除颜色缓冲区
     * @param clearDepth 是否清除深度缓冲区
     * @param clearStencil 是否清除模板缓冲区
     */
    void Clear(BOOL clearColor = TRUE, BOOL clearDepth = TRUE, BOOL clearStencil = FALSE);
    
    /**
     * @brief 获取当前帧率
     * @return 帧率
     */
    DWORD GetFPS() const { return m_FPS; }
    
    /**
     * @brief 获取渲染区宽度
     * @return 宽度
     */
    INT GetWidth() const { return m_Width; }
    
    /**
     * @brief 获取渲染区高度
     * @return 高度
     */
    INT GetHeight() const { return m_Height; }
    
    /**
     * @brief 获取宽高比
     * @return 宽高比
     */
    FLOAT GetAspectRatio() const { return m_AspectRatio; }
    
    /**
     * @brief 检查渲染器是否已初始化
     * @return 已初始化返回TRUE，否则返回FALSE
     */
    BOOL IsInitialized() const { return m_GLInitialized; }
    
    /**
     * @brief 获取OpenGL上下文
     * @return OpenGL渲染上下文
     */
    HGLRC GetGLContext() const { return m_hRC; }
    
    /**
     * @brief 获取设备上下文
     * @return 设备上下文
     */
    HDC GetDeviceContext() const { return m_hDC; }
    
    /**
     * @brief 保存当前OpenGL状态
     * @note 保存当前矩阵、属性和启用状态
     */
    void PushState();
    
    /**
     * @brief 恢复保存的OpenGL状态
     */
    void PopState();
    
    /**
     * @brief 重置渲染器到默认状态
     */
    void ResetState();
    
    /**
     * @brief 获取OpenGL信息
     * @return OpenGL信息字符串
     */
    std::string GetGLInfo() const;
};
#endif // __RENDERER_H__