// ======================================================================
#ifndef __WINDOW_H__
#define __WINDOW_H__
// ======================================================================

#include <Windows.h>
#include <functional>     // 引入回调函数
#include "EngineConfig.h" // 包含配置结构
// ======================================================================

// ======================================================================
/**
 * @brief 窗口类，负责创建和管理Windows窗口
 * @details 封装了窗口创建、消息处理和显示功能
 */
class CWindow
{
private:
    HWND m_hWnd;           // 窗口句柄
    HINSTANCE m_hInstance; // 应用程序实例句柄

    // FIXME: 内存对齐
    char padding1[16];
    std::function<void(int, int)> m_ResizeCallback; // 接收(width, height)的回调函数
    char padding2[16];

    BOOL m_Fullscreen;          // 是否全屏模式
    BOOL m_IsSwitching = FALSE; // 是否正在切换全屏
    BOOL m_Minimized;           // 窗口是否最小化
    BOOL m_Maximized;           // 窗口是否最大化

    RECT m_WindowRect;         // 保存窗口位置和大小（用于全屏切换）
    DWORD m_WindowStyle = 0;   // 窗口样式
    DWORD m_WindowExStyle = 0; // 窗口扩展样式
    DWORD m_OriginalStyle;     // 保存原始窗口样式
    DWORD m_OriginalExStyle;   // 保存原始扩展样式
    DWORD m_CurrentStyle;      // 当前窗口样式
    DWORD m_CurrentExStyle;    // 当前扩展样式
    BOOL m_InSizing = FALSE;   // 重入锁

    EngineConfig m_Config; // 引擎配置

    /**
     * @brief 窗口过程函数（静态）
     * @param hWnd 窗口句柄
     * @param msg 消息ID
     * @param wParam 消息参数1
     * @param lParam 消息参数2
     * @return 消息处理结果
     */
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 处理窗口消息
     * @param hWnd 窗口句柄
     * @param msg 消息ID
     * @param wParam 消息参数1
     * @param lParam 消息参数2
     * @return 消息处理结果
     */
    LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    BOOL RegisterWindowClass(); // 注册窗口类

    BOOL SetFullscreen(BOOL enable);           // 切换全屏模式
    BOOL SetBorderlessFullscreen(BOOL enable); // 切换全屏模式 无边框模拟

    ResizeData m_ResizeEvent; // 重置窗口大小事件

public:
    CWindow();
    ~CWindow();

    // 删除拷贝构造和赋值
    CWindow(const CWindow &) = delete;
    CWindow &operator=(const CWindow &) = delete;

    struct ResizeData
    {
        BOOL pending = false;
        INT width = 0;
        INT height = 0;
    };
    ResizeData ConsumeResizeEvent() {
        ResizeData data = m_ResizeEvent;
        m_ResizeEvent.pending = false; // 消费后重置
        return data;
    } // 获取并重置状态

    /**
     * @brief 创建窗口
     * @param hInstance 应用程序实例句柄
     * @param config 引擎配置
     * @return 创建成功返回TRUE，失败返回FALSE
     */
    BOOL Create(HINSTANCE hInstance, const EngineConfig &config);

    void SetTitle(const wchar_t *title);         // 设置窗口标题
    void Destroy();                              // 销毁窗口
    void Show();                                 // 显示窗口
    void Hide();                                 // 隐藏窗口
    void ToggleVisibility();                     // 切换显示/隐藏窗口
    void ToggleFullscreen();                     // 切换全屏/窗口模式
    BOOL IsSwitching() { return m_IsSwitching; } // 获取切换状态

    /**
     * @brief 设置窗口位置和大小
     * @param x 左上角X坐标
     * @param y 左上角Y坐标
     * @param width 宽度
     * @param height 高度
     */
    void SetPosition(INT x, INT y, INT width, INT height);

    HWND GetHWND() const { return m_hWnd; }               // 获取窗口句柄
    HINSTANCE GetInstance() const { return m_hInstance; } // 获取应用程序实例句柄

    BOOL IsFullscreen() const { return m_Fullscreen; } // 判断是否全屏
    BOOL IsActive() const;                             // 判断窗口是否激活
    BOOL IsMinimized() const { return m_Minimized; }   // 判断窗口是否最小化
    BOOL IsMaximized() const { return m_Maximized; }   // 判断窗口是否最大化
    INT GetClientWidth() const;                        // 获取窗口客户区宽度
    INT GetClientHeight() const;                       // 获取窗口客户区高度

    void ShowMouseCursor(BOOL show); // 安全的光标机制
    /**
     * @brief 处理窗口消息循环
     * @return 如果有WM_QUIT消息返回FALSE，否则返回TRUE
     */
    static BOOL ProcessMessages();

    // 提供一个接口来绑定渲染器的 Reset 函数
    // cb = callback
    void SetResizeCallback(std::function<void(INT, INT)> cb) { this->m_ResizeCallback = cb; }
};

#endif // __WINDOW_H__