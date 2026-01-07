
// ======================================================================
#ifndef __RENDERER_H__
#define __RENDERER_H__
// ======================================================================

#include <string>
#include <chrono> // 现代时间库. 实现权衡的帧率计算
#include <Windows.h>
#include <GL/gl.h>
// ======================================================================

/**
 * @brief OpenGL渲染器类
 * @details 负责OpenGL初始化、渲染状态管理和帧缓冲交换
 */
class CRenderer
{
private:
    HWND m_hWnd; // 窗口句柄
    HDC m_hDC;   // 设备上下文
    HGLRC m_hRC; // OpenGL渲染上下文

    DWORD m_Width;       // 渲染区宽度
    DWORD m_Height;      // 渲染区高度
    FLOAT m_AspectRatio; // 宽高比

    FLOAT m_ClearColor[4]; // 清除颜色
    BOOL m_VSyncEnabled;   // 垂直同步是否启用

    // ======================================================================
    // 渲染统计数据
    LARGE_INTEGER m_LastCounter;                                  // 计时器
    DWORD m_FrameCount;                                           // 帧计数器
    DWORD m_LastFPSUpdate;                                        // 上次FPS更新时间
    FLOAT m_FPS;                                                  // 当前帧率
    FLOAT m_FPSSmooth;                                            // 平滑后的FPS
    std::chrono::high_resolution_clock::time_point m_LastFPSTime; // 上次计算FPS的时间
    FLOAT m_FPSSmoothingFactor;                                   // 平滑因子

    static const int SAMPLE_COUNT = 60;     // 记录最近 60 帧
    FLOAT m_FrameTimeHistory[SAMPLE_COUNT]; // 缓冲区数组
    int m_NextIndex;                        // 下一个要写入的位置

    FLOAT m_DeltaTime;    // 帧间隔时间
    FLOAT m_MinDeltaTime; // 最小帧时间
    FLOAT m_MaxDeltaTime; // 最大帧时间, 超过100ms 就剔除

    // ======================================================================
    // OpenGL扩展功能支持标志
    BOOL m_GLInitialized;           // OpenGL是否已初始化
    BOOL m_WGLSwapControlSupported; // 交换控制是否支持
    BOOL SetupPixelFormat();        // 初始化OpenGL像素格式
    void CheckGLExtensions();       // 检查OpenGL扩展功能
    void SetupRenderState();        // 初始化OpenGL渲染状态
    /**
     * @brief 检查OpenGL错误
     * @param function 发生错误的函数名
     * @return 有错误返回TRUE，无错误返回FALSE
     */
    BOOL CheckGLError(const char *function);
    std::string GetGLVersion() const;  // 获取OpenGL版本信息, 返回OpenGL版本字符串
    std::string GetGLRenderer() const; // 获取OpenGL渲染器信息
    std::string GetGLVendor() const;   // 获取OpenGL供应商信息

    /**
     * @brief 更新帧率统计
     * @note 在图形渲染中，FPS如果波动剧烈，会导致视觉上的卡顿（Stuttering）。
     *       一个设计良好的 UpdateFPS 不仅是给用户看一个数字，
     *       更重要的是为垂直同步（VSync）调节和动态画质调整提供数据支持。
     */
    void UpdateFPS();

    BOOL InternalSetVerticalSync(BOOL enable); // 内部设置垂直同步

public:
    CRenderer();
    ~CRenderer();

    // 禁止拷贝构造和赋值
    CRenderer(const CRenderer &) = delete;
    CRenderer &operator=(const CRenderer &) = delete;

    BOOL Initialize(HWND hWnd); // 初始化渲染器
    void Shutdown();            // 关闭渲染器

    void BeginFrame(); // 开始渲染一帧
    void EndFrame();   // 结束渲染一帧

    /**
     * @brief 重置渲染器
     * @note（在窗口大小改变时调用）
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

    void SetVerticalSync(BOOL enable); // 设置垂直同步
    void ToggleVerticalSync();         // 切换垂直同步状态

    void SetWireframeMode(BOOL enable);   // 设置线框模式
    void SetBackfaceCulling(BOOL enable); // 设置背面剔除
    void SetDepthTest(BOOL enable);       // 设置深度测试
    void SetBlending(BOOL enable);        // 设置混合模式

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
     * @param fovY 垂直视角（传入角度）
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

    FLOAT GetFPS() const { return m_FPS; } // 获取当前帧率
    void AddFrameSample(FLOAT dt);
    FLOAT GetAverageFrameTime() const; // 获取平均耗时（用于计算平滑 FPS）
    FLOAT GetSmoothFPS() const;        // 获取平滑帧率

    INT GetWidth() const { return m_Width; }               // 获取渲染区宽度
    INT GetHeight() const { return m_Height; }             // 获取渲染区高度
    FLOAT GetAspectRatio() const { return m_AspectRatio; } // 获取宽高比

    /**
     * @brief 获取帧间隔时间
     * @note 供外部逻辑（如物理引擎）调用
     * @return 帧间隔时间
     */
    FLOAT GetDeltaTime() const { return m_DeltaTime; }

    BOOL IsInitialized() const { return m_GLInitialized; } // 检查渲染器是否已初始化

    HGLRC GetGLContext() const { return m_hRC; }   // 获取OpenGL上下文
    HDC GetDeviceContext() const { return m_hDC; } // 获取设备上下文

    /**
     * @brief 保存当前OpenGL状态
     * @note 保存当前矩阵、属性和启用状态
     */
    void PushState();
    void PopState();   // 恢复保存的OpenGL状态
    void ResetState(); // 重置渲染器到默认状态

    std::string GetGLInfo() const; // 获取OpenGL信息
};

#endif // __RENDERER_H__