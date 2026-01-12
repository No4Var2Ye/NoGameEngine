
// ======================================================================
#include "stdafx.h"

#include <tchar.h>
#include <cassert>
#include <functional>
#include "Core/Window.h"
#include "Core/GameEngine.h"
#include "Core/InputManager.h"

// ======================================================================

// ======================================================================
// ====================== 公有方法 =======================================

CWindow::CWindow()
    : m_hWnd(nullptr),      // 窗口句柄
      m_hInstance(nullptr), // 应用程序实例句柄
      m_Fullscreen(FALSE),  // 是否全屏模式
      m_Minimized(FALSE),   // 窗口是否最小化
      m_Maximized(FALSE)    // 窗口是否最大化
{
    ZeroMemory(&m_WindowRect, sizeof(RECT));
}

CWindow::~CWindow()
{
    Destroy();
}

BOOL CWindow::Create(HINSTANCE hInstance, const EngineConfig &config)
{
    assert(hInstance != nullptr);

    m_hInstance = hInstance;
    m_Config = config;
    m_Fullscreen = FALSE;

    // 1. 注册窗口类
    if (!RegisterWindowClass())
    {
        MessageBoxW(NULL, L"注册窗口类失败!", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 2. 准备窗口样式
    // 永久保存窗口模式的样式备份
    m_WindowStyle = WS_OVERLAPPEDWINDOW;
    m_WindowExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

    DWORD currentStyle, currentExStyle;
    if (config.fullscreen)
    {
        currentStyle = WS_POPUP;
        currentExStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
    }
    else
    {
        currentStyle = m_WindowStyle;
        currentExStyle = m_WindowExStyle;
    }

    // 3. 计算窗口大小（包含边框）
    RECT windowRect = {0, 0, (LONG)config.windowWidth, (LONG)config.windowHeight};

    // 调整窗口矩形，使其包含边框和标题栏
    AdjustWindowRectEx(&windowRect, m_WindowStyle, FALSE, m_WindowExStyle);

    INT windowWidth = windowRect.right - windowRect.left;
    INT windowHeight = windowRect.bottom - windowRect.top;

    // 4. 计算窗口位置（居中显示）
    INT screenWidth = GetSystemMetrics(SM_CXSCREEN);
    INT screenHeight = GetSystemMetrics(SM_CYSCREEN);
    INT posX = (screenWidth - windowWidth) / 2;
    INT posY = (screenHeight - windowHeight) / 2;

    // 5. 创建窗口
    m_hWnd = CreateWindowExW(
        currentExStyle,        // 扩展窗口样式
        L"MyGameEngineWindow", // 窗口类名
        config.title,          // 窗口标题
        currentStyle,          // 窗口样式
        posX,                  // 窗口X位置
        posY,                  // 窗口Y位置
        windowWidth,           // 窗口宽度
        windowHeight,          // 窗口高度
        NULL,                  // 父窗口句柄
        NULL,                  // 菜单句柄
        hInstance,             // 应用程序实例句柄
        this                   // 创建参数，传递this指针
    );

    if (!m_hWnd)
    {
        MessageBoxW(NULL, L"创建窗口失败!", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 6. 保存窗口矩形（用于全屏切换）
    m_WindowRect.left = posX;
    m_WindowRect.top = posY;
    m_WindowRect.right = posX + windowWidth;
    m_WindowRect.bottom = posY + windowHeight;

    // 7. 全屏模式设置
    if (config.fullscreen)
    {

        if (!SetBorderlessFullscreen(TRUE))
        {
            // 全屏失败，切换到窗口模式
            m_Fullscreen = FALSE;
            MessageBoxW(NULL, L"全屏模式设置失败，将使用窗口模式。", L"警告", MB_OK | MB_ICONWARNING);
        }
    }
    else
    {
        m_Fullscreen = FALSE;
    }

    return TRUE;
}

BOOL CWindow::RegisterWindowClass()
{
    static bool isRegistered = false;
    if (isRegistered)
        return TRUE; // 如果已注册，直接跳过

    WNDCLASSEXW wc = {};

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;                      // 重绘时发送重绘消息，拥有设备上下文
    wc.lpfnWndProc = CWindow::WndProc;                                  // 窗口过程函数
    wc.cbClsExtra = 0;                                                  // 无额外类数据
    wc.cbWndExtra = 0;                                                  // 无额外窗口数据
    wc.hInstance = m_hInstance;                                         // 应用程序实例
    wc.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(IDI_APPLICATION));   // 默认应用程序图标
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW));     // 默认箭头光标
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);             // 黑色背景
    wc.lpszMenuName = nullptr;                                          // 无菜单
    wc.lpszClassName = L"MyGameEngineWindow";                           // 窗口类名
    wc.hIconSm = LoadIconW(nullptr, MAKEINTRESOURCEW(IDI_APPLICATION)); // 小图标

    if (RegisterClassExW(&wc))
    {
        isRegistered = true;
        return TRUE;
    }
    return FALSE;
}

void CWindow::Destroy()
{
    if (m_hWnd)
    {
        // 如果当前是全屏模式，先切换回窗口模式
        if (m_Fullscreen)
        {
            SetBorderlessFullscreen(FALSE);
            this->ShowMouseCursor(TRUE);
        }

        // 销毁窗口
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }
}

void CWindow::Show()
{
    if (m_hWnd)
    {
        // 显示窗口
        ShowWindow(m_hWnd, SW_SHOW);

        // 更新窗口
        UpdateWindow(m_hWnd);

        // 将窗口设为前台窗口
        SetForegroundWindow(m_hWnd);

        // 将输入焦点设到窗口
        SetFocus(m_hWnd);

        std::cout << "窗口已显示并设置焦点" << std::endl;
    }
}

void CWindow::Hide()
{
    if (m_hWnd)
    {
        ShowWindow(m_hWnd, SW_HIDE);
    }
}

void CWindow::ToggleVisibility()
{
    if (m_hWnd)
    {
        BOOL visible = IsWindowVisible(m_hWnd);
        ShowWindow(m_hWnd, visible ? SW_HIDE : SW_SHOW);
    }
}

void CWindow::ToggleFullscreen()
{
    if (m_hWnd)
    {
        LONG_PTR style = GetWindowLongPtr(m_hWnd, GWL_STYLE);

        // 判定逻辑：如果包含 WS_POPUP (0x80000000)，说明是全屏模式
        BOOL isFullscreen = (style & WS_POPUP) != 0;

        if (isFullscreen)
        {
            SetBorderlessFullscreen(FALSE); // 如果是全屏，则恢复窗口
        }
        else
        {
            SetBorderlessFullscreen(TRUE); // 如果是窗口，则进入全屏
        }
    }
}

void CWindow::SetTitle(const wchar_t *title)
{
    if (m_hWnd && title)
    {
        SetWindowTextW(m_hWnd, title);
    }
}

void CWindow::SetPosition(INT x, INT y, INT width, INT height)
{
    if (m_hWnd)
    {
        // 调整窗口矩形，使其包含边框
        RECT windowRect = {0, 0, width, height};
        AdjustWindowRectEx(&windowRect, m_WindowStyle, FALSE, m_WindowExStyle);

        INT windowWidth = windowRect.right - windowRect.left;
        INT windowHeight = windowRect.bottom - windowRect.top;

        SetWindowPos(m_hWnd, NULL, x, y, windowWidth, windowHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
    }
}

BOOL CWindow::SetBorderlessFullscreen(BOOL enable)
{
    // OutputDebugStringA("使用无边框窗口模拟全屏\n");

    // char debugBuf[256];
    // sprintf_s(debugBuf, "[Debug] Enter SetFS: Target=%s, CurrentVar=%s, WindowPtr=%p\n",
    //           enable ? "TRUE" : "FALSE",
    //           m_Fullscreen ? "TRUE" : "FALSE",
    //           this);
    // OutputDebugStringA(debugBuf);

    if (!m_hWnd)
        return FALSE;

    // 调试 2: 检查实际窗口样式（物理状态检查）
    LONG_PTR currentStyle = GetWindowLongPtr(m_hWnd, GWL_STYLE);

    // sprintf_s(debugBuf, "[Debug] Physical Style: 0x%IX, HasCaption: %s\n",
    //           currentStyle, (currentStyle & WS_CAPTION) ? "YES" : "NO");
    // OutputDebugStringA(debugBuf);

    // 1. 状态一致性检查
    // if (m_Fullscreen == enable && IsWindowVisible(m_hWnd))
    // {
    //     return TRUE;
    // }

    if (enable)
    {
        // OutputDebugStringA("切换到无边框全屏模式...\n");

        // 保存窗口位置和样式
        if (!m_Fullscreen)
        {
            GetWindowRect(m_hWnd, &m_WindowRect);
        }

        // 2. 获取当前窗口所在的显示器信息
        HMONITOR hMonitor = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = {sizeof(mi)};
        GetMonitorInfo(hMonitor, &mi);

        // 3. 修改样式：移除标题栏、边框等
        SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowLongPtr(m_hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW);

        // 4. 设置位置：覆盖整个屏幕
        SetWindowPos(m_hWnd, HWND_TOP,
                     mi.rcMonitor.left,
                     mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left,
                     mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

        m_Fullscreen = TRUE;
        // OutputDebugStringA("无边框全屏设置完成\n");
    }
    else
    {
        // OutputDebugStringA("切换到窗口模式...\n");

        DWORD standardStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        DWORD standardExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

        // 恢复窗口样式
        SetWindowLong(m_hWnd, GWL_STYLE, standardStyle);
        SetWindowLong(m_hWnd, GWL_EXSTYLE, standardExStyle);

        // 确保矩形有效（防止 m_WindowRect 也是空的）
        if (m_WindowRect.right == 0)
        {
            m_WindowRect = {100, 100, 1280, 800};
        }

        // 计算备份矩形的宽高
        INT width = m_WindowRect.right - m_WindowRect.left;
        INT height = m_WindowRect.bottom - m_WindowRect.top;

        // 恢复位置
        SetWindowPos(m_hWnd, HWND_NOTOPMOST,
                     m_WindowRect.left, m_WindowRect.top,
                     width, height,
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);

        m_Fullscreen = FALSE;
        // OutputDebugStringA("窗口模式恢复完成\n");
    }
    // 调试 3: 打印退出前的最终状态
    // sprintf_s(debugBuf, "[Debug] Exit SetFS: Var now is %s\n", m_Fullscreen ? "TRUE" : "FALSE");
    // OutputDebugStringA(debugBuf);

    // if (m_ResizeCallback)
    // {
    //     m_ResizeCallback(GetClientWidth(), GetClientHeight());
    // }

    // 【替换为】手动触发一次异步事件
    m_ResizeEvent.width = GetClientWidth();
    m_ResizeEvent.height = GetClientHeight();
    m_ResizeEvent.pending = TRUE;

    return TRUE;
}

BOOL CWindow::IsActive() const
{
    if (!m_hWnd)
        return FALSE;

    return GetActiveWindow() == m_hWnd;
}

INT CWindow::GetClientWidth() const
{
    if (!m_hWnd)
        return 0;

    RECT clientRect = {};
    GetClientRect(m_hWnd, &clientRect);
    return clientRect.right - clientRect.left;
}

INT CWindow::GetClientHeight() const
{
    if (!m_hWnd)
        return 0;

    RECT clientRect = {};
    GetClientRect(m_hWnd, &clientRect);
    return clientRect.bottom - clientRect.top;
}

void CWindow::ShowMouseCursor(BOOL show)
{
    CURSORINFO ci = {sizeof(CURSORINFO)};
    if (GetCursorInfo(&ci))
    {
        BOOL isVisible = (ci.flags & CURSOR_SHOWING);
        if (show && !isVisible)
        {
            ShowCursor(TRUE);
        }
        else if (!show && isVisible)
        {
            ShowCursor(FALSE);
        }
    }
}

BOOL CWindow::ProcessMessages()
{
    MSG msg = {};

    // 处理所有待处理的消息
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            return FALSE; // 收到退出消息
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return TRUE; // 继续运行
}

// ======================================================================
// ====================== 私有方法 =======================================

LRESULT CALLBACK CWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CWindow *pWindow = nullptr;

    if (msg == WM_NCCREATE)
    {
        // 在窗口创建时，获取传递的this指针
        CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
        pWindow = reinterpret_cast<CWindow *>(pCreate->lpCreateParams);

        // 将this指针存储在窗口的额外数据中
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWindow));
    }
    else
    {
        // 从窗口的额外数据中获取this指针
        pWindow = reinterpret_cast<CWindow *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (pWindow)
    {
        return pWindow->HandleMessage(hWnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}

LRESULT CWindow::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // 这样 InputManager 才能监听到 WM_KEYDOWN, WM_MOUSEMOVE 等消息
    auto pInput = CGameEngine::GetInstance().GetInputManager();
    if (pInput)
    {
        pInput->HandleMessage(hWnd, msg, wParam, lParam);
    }

    switch (msg)
    {
    case WM_CREATE:
        // 窗口创建
        return 0;

    case WM_SIZE:
        // 窗口大小改变
        // {
        //     m_Minimized = (wParam == SIZE_MINIMIZED);
        //     m_Maximized = (wParam == SIZE_MAXIMIZED);

        //     // 1. 获取新的客户区尺寸
        //     int newWidth = LOWORD(lParam);
        //     int newHeight = HIWORD(lParam);

        //     // 2. 只有在非最小化且尺寸有效时才通知渲染器
        //     // 通知渲染器窗口大小改变
        //     if (!m_Minimized && newWidth > 0 && newHeight > 0)
        //     {
        //         if (m_ResizeCallback)
        //         {
        //             try
        //             {
        //                 m_ResizeCallback(newWidth, newHeight);
        //             }
        //             catch (...)
        //             {
        //                 OutputDebugStringA("Critical: Resize callback failed!\n");
        //             }
        //         }
        //         else
        //         {
        //             // 如果回调由于某种原因丢失，使用单例兜底
        //             CGameEngine::GetInstance().OnWindowResize(newWidth, newHeight);
        //         }
        //     }
        // }
        {
            if (wParam != SIZE_MINIMIZED) // 忽略最小化，避免宽高温标为0
            {
                m_ResizeEvent.width = LOWORD(lParam);
                m_ResizeEvent.height = HIWORD(lParam);
                m_ResizeEvent.pending = true;
            }
        }
        return 0;

    case WM_EXITSIZEMOVE:
    {
        // 用户松开鼠标边缘时，最后确认一次尺寸同步
        // RECT rect;
        // GetClientRect(hWnd, &rect);
        // if (m_ResizeCallback)
        // {
        //     m_ResizeCallback(rect.right - rect.left, rect.bottom - rect.top);
        // }
        RECT rect;
        GetClientRect(hWnd, &rect);
        m_ResizeEvent.width = rect.right - rect.left;
        m_ResizeEvent.height = rect.bottom - rect.top;
        m_ResizeEvent.pending = true;
    }
        return 0;

    case WM_ACTIVATE:
    {
        // 窗口激活状态改变
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            // 窗口失去焦点
        }
        else
        {
            // 窗口获得焦点
        }
        return 0;
    }

    case WM_SETFOCUS:
        // 窗口获得焦点
        return 0;

    case WM_KILLFOCUS:
        // 窗口失去焦点
        return 0;

    case WM_GETMINMAXINFO:
        // 设置窗口最小最大尺寸
        {
            LPMINMAXINFO pMMI = (LPMINMAXINFO)lParam;

            // 设置最小尺寸
            pMMI->ptMinTrackSize.x = 800;
            pMMI->ptMinTrackSize.y = 600;

            // 设置最大尺寸
            pMMI->ptMaxTrackSize.x = GetSystemMetrics(SM_CXSCREEN);
            pMMI->ptMaxTrackSize.y = GetSystemMetrics(SM_CYSCREEN);
        }
        return 0;

    // TODO: 按键设置
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
        // char buffer[64];
        // sprintf_s(buffer, sizeof(buffer), "收到按键: 0x%X (虚拟键码)\n", wParam);
        // OutputDebugStringA(buffer);

        // 按键按下
        if (wParam == VK_ESCAPE)
        {
            // ESC键 - 退出程序
            // OutputDebugStringA("ESC 键被按下，退出程序\n");
            PostMessage(hWnd, WM_CLOSE, 0, 0);
            Sleep(1000);
        }
        else if (wParam == VK_F11)
        // else if (wParam == VK_F11 || (wParam == VK_RETURN && (HIWORD(lParam) & KF_ALTDOWN)))
        {
            // F11键 - 切换全屏
            // OutputDebugStringA("F11 键被按下!\n");
            ToggleFullscreen();
        }
        return 0;

    case WM_KEYUP:
        // 按键释放
        return 0;

    case WM_SYSCOMMAND:
        // 系统命令
        switch (wParam & 0xFFF0) // 必须使用 0xFFF0 过滤低位
        {
        case SC_SCREENSAVE:   // 屏保
        case SC_MONITORPOWER: // 显示器节能
            if (m_Fullscreen)
                return 0; // 全屏模式下防止屏保和节能模式
            break;
        case SC_KEYMENU: // 系统菜单
            if (m_Fullscreen)
                return 0; // 全屏模式下禁用系统菜单
            break;
        }
        break;

    case WM_ERASEBKGND:
        // 阻止擦除背景，减少闪烁
        return 1;

    case WM_CLOSE:
        // 关闭窗口
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        // 窗口销毁
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}