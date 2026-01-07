#ifndef __WINDOW_H__
#define __WINDOW_H__

#include "EngineConfig.h" // 包含配置结构
#include <Windows.h>

/**
 * @brief 窗口类，负责创建和管理Windows窗口
 * @details 封装了窗口创建、消息处理和显示功能
 */
class CWindow
{
private:
    HWND m_hWnd = nullptr;           // 窗口句柄
    HINSTANCE m_hInstance = nullptr; // 应用程序实例句柄
    BOOL m_Fullscreen = false;       // 是否全屏模式
    BOOL m_Minimized = false;        // 窗口是否最小化
    BOOL m_Maximized = false;        // 窗口是否最大化

    RECT m_WindowRect;         // 保存窗口位置和大小（用于全屏切换）
    DWORD m_WindowStyle = 0;   // 窗口样式
    DWORD m_WindowExStyle = 0; // 窗口扩展样式

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

    /**
     * @brief 注册窗口类
     * @return 注册成功返回TRUE，失败返回FALSE
     */
    BOOL RegisterWindowClass();

    /**
     * @brief 切换全屏模式
     * @param enable 是否启用全屏
     * @return 切换成功返回TRUE，失败返回FALSE
     */
    BOOL SetFullscreen(BOOL enable);

public:
    CWindow();
    ~CWindow();

    // 删除拷贝构造和赋值
    CWindow(const CWindow &) = delete;
    CWindow &operator=(const CWindow &) = delete;

    /**
     * @brief 创建窗口
     * @param hInstance 应用程序实例句柄
     * @param config 引擎配置
     * @return 创建成功返回TRUE，失败返回FALSE
     */
    BOOL Create(HINSTANCE hInstance, const EngineConfig &config);

    void Destroy();          // 销毁窗口
    void Show();             // 显示窗口
    void Hide();             // 隐藏窗口
    void ToggleVisibility(); // 切换显示/隐藏窗口
    void ToggleFullscreen(); // 切换全屏/窗口模式

    /**
     * @brief 设置窗口标题
     * @param title 新标题
     */
    void SetTitle(const wchar_t *title);

    /**
     * @brief 设置窗口位置和大小
     * @param x 左上角X坐标
     * @param y 左上角Y坐标
     * @param width 宽度
     * @param height 高度
     */
    void SetPosition(INT x, INT y, INT width, INT height);

    /**
     * @brief 获取窗口句柄
     * @return 窗口句柄
     */
    HWND GetHWND() const { return m_hWnd; }

    /**
     * @brief 获取应用程序实例句柄
     * @return 实例句柄
     */
    HINSTANCE GetInstance() const { return m_hInstance; }

    /**
     * @brief 判断是否全屏
     * @return 全屏返回TRUE，窗口模式返回FALSE
     */
    BOOL IsFullscreen() const { return m_Fullscreen; }

    /**
     * @brief 判断窗口是否激活
     * @return 激活返回TRUE，否则返回FALSE
     */
    BOOL IsActive() const;

    /**
     * @brief 获取窗口客户区宽度
     * @return 宽度
     */
    INT GetClientWidth() const;

    /**
     * @brief 获取窗口客户区高度
     * @return 高度
     */
    INT GetClientHeight() const;

    /**
     * @brief 处理窗口消息循环
     * @return 如果有WM_QUIT消息返回FALSE，否则返回TRUE
     */
    static BOOL ProcessMessages();
};

#endif // __WINDOW_H__