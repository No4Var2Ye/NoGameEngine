
#include "stdafx.h"
#include "Core/Window.h"
#include <tchar.h>
#include <cassert>

CWindow::CWindow()
    : m_hWnd(nullptr), m_hInstance(nullptr), m_Fullscreen(false), m_Minimized(false), m_Maximized(false)
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
    m_Fullscreen = config.fullscreen;

    // 1. 注册窗口类
    if (!RegisterWindowClass())
    {
        MessageBoxW(NULL, L"注册窗口类失败!", L"错误", MB_OK | MB_ICONERROR);
        return FALSE;
    }

    // 2. 准备窗口样式
    DWORD style = WS_OVERLAPPEDWINDOW; // 默认窗口样式
    DWORD exStyle = WS_EX_APPWINDOW;   // 默认扩展样式

    if (m_Fullscreen)
    {
        // 全屏模式
        style = WS_POPUP; // 无边框窗口
        exStyle = WS_EX_APPWINDOW | WS_EX_TOPMOST;
    }
    else
    {
        // 窗口模式
        style = WS_OVERLAPPEDWINDOW;
        exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
    }

    m_WindowStyle = style;
    m_WindowExStyle = exStyle;

    // 3. 计算窗口大小（包含边框）
    RECT windowRect = {0, 0, config.windowWidth, config.windowHeight};

    // 调整窗口矩形，使其包含边框和标题栏
    AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);

    INT windowWidth = windowRect.right - windowRect.left;
    INT windowHeight = windowRect.bottom - windowRect.top;

    // 4. 计算窗口位置（居中显示）
    INT screenWidth = GetSystemMetrics(SM_CXSCREEN);
    INT screenHeight = GetSystemMetrics(SM_CYSCREEN);
    INT posX = (screenWidth - windowWidth) / 2;
    INT posY = (screenHeight - windowHeight) / 2;

    // 5. 创建窗口
    m_hWnd = CreateWindowExW(
        exStyle,               // 扩展窗口样式
        L"MyGameEngineWindow", // 窗口类名
        config.title,          // 窗口标题
        style,                 // 窗口样式
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
    GetWindowRect(m_hWnd, &m_WindowRect);

    // 7. 全屏模式设置
    if (m_Fullscreen)
    {
        if (!SetFullscreen(TRUE))
        {
            // 全屏失败，切换到窗口模式
            m_Fullscreen = false;
            MessageBoxW(NULL, L"全屏模式设置失败，将使用窗口模式。", L"警告", MB_OK | MB_ICONWARNING);
        }
    }

    return TRUE;
}

BOOL CWindow::RegisterWindowClass()
{
    WNDCLASSEXW wc = {};

    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;          // 重绘时发送重绘消息，拥有设备上下文
    wc.lpfnWndProc = CWindow::WndProc;                      // 窗口过程函数
    wc.cbClsExtra = 0;                                      // 无额外类数据
    wc.cbWndExtra = 0;                                      // 无额外窗口数据
    wc.hInstance = m_hInstance;                             // 应用程序实例
    wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);            // 默认应用程序图标
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);              // 默认箭头光标
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // 黑色背景
    wc.lpszMenuName = NULL;                                 // 无菜单
    wc.lpszClassName = L"MyGameEngineWindow";               // 窗口类名
    wc.hIconSm = LoadIconW(NULL, IDI_APPLICATION);          // 小图标

    return RegisterClassExW(&wc) != 0;
}

void CWindow::Destroy()
{
    if (m_hWnd)
    {
        // 如果当前是全屏模式，先切换回窗口模式
        if (m_Fullscreen)
        {
            SetFullscreen(FALSE);
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
    std::cout << "ToggleFullscreen() 被调用" << std::endl;
    std::cout << "当前全屏状态: " << (m_Fullscreen ? "是" : "否") << std::endl;
    
    if (m_hWnd)
    {
        SetFullscreen(!m_Fullscreen);
    }
        else
    {
        std::cout << "错误: 窗口句柄无效!" << std::endl;
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

BOOL CWindow::SetFullscreen(BOOL enable)
{
    char buffer[256];
    sprintf_s(buffer, sizeof(buffer), 
             "SetFullscreen(%s) 调用\n", enable ? "TRUE" : "FALSE");
    OutputDebugStringA(buffer);

    if (!m_hWnd)
    {
        OutputDebugStringA("错误: 窗口句柄为空\n");
        return FALSE;
    }

    if (!m_hWnd || m_Fullscreen == enable)
    {
        sprintf_s(buffer, sizeof(buffer), "已经是%s模式，无需切换\n", enable ? "全屏" : "窗口");
        OutputDebugStringA(buffer);
        return TRUE;
    }

    if (enable)
    {
        OutputDebugStringA("切换到全屏模式...\n");
        // 切换到全屏模式

        // 1. 保存当前窗口位置和大小
        GetWindowRect(m_hWnd, &m_WindowRect);
        sprintf_s(buffer, sizeof(buffer), 
                 "保存窗口矩形: (%d,%d)-(%d,%d)\n",
                 m_WindowRect.left, m_WindowRect.top,
                 m_WindowRect.right, m_WindowRect.bottom);
        OutputDebugStringA(buffer);

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        m_Config.windowWidth = screenWidth;
        m_Config.windowHeight = screenHeight;

        // 2. 获取当前显示设置
        DEVMODEW devMode = {};
        devMode.dmSize = sizeof(DEVMODEW);

        sprintf_s(buffer, sizeof(buffer), 
                 "请求分辨率: %dx%dx%dbpp\n",
                 m_Config.windowWidth, m_Config.windowHeight, m_Config.colorBits);
        OutputDebugStringA(buffer);

        if (EnumDisplaySettingsW(NULL, ENUM_CURRENT_SETTINGS, &devMode))
        {
            sprintf_s(buffer, sizeof(buffer), 
                     "当前显示设置: %dx%dx%dbpp\n",
                     devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmBitsPerPel);
            OutputDebugStringA(buffer);

        }
        
        // 3. 设置全屏显示模式
        // TODO: FIXME: 多设备支持
        // devMode.dmPelsWidth = m_Config.windowWidth;
        // devMode.dmPelsHeight = m_Config.windowHeight;
        devMode.dmPelsWidth = 2560;
        devMode.dmPelsHeight = 1600;
        devMode.dmBitsPerPel = m_Config.colorBits;
        devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        LONG result = ChangeDisplaySettingsW(&devMode, CDS_FULLSCREEN);
        
        sprintf_s(buffer, sizeof(buffer), 
                 "ChangeDisplaySettings 返回: %d\n", result);
        OutputDebugStringA(buffer);

        if (result != DISP_CHANGE_SUCCESSFUL)
        {
            // 全屏模式失败
            OutputDebugStringA("全屏模式设置失败!\n");
            return FALSE;
        }
        
        // 4. 设置窗口为全屏样式
        SetWindowLongW(m_hWnd, GWL_STYLE, WS_POPUP);
        SetWindowLongW(m_hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW | WS_EX_TOPMOST);

        // 5. 设置窗口位置和大小
        SetWindowPos(m_hWnd, HWND_TOP, 0, 0,
                     m_Config.windowWidth, m_Config.windowHeight,
                     SWP_SHOWWINDOW | SWP_FRAMECHANGED);

        // 6. 隐藏光标
        ShowCursor(FALSE);

        m_Fullscreen = TRUE;
        OutputDebugStringA("切换到全屏模式成功\n");
    }
    else
    {
        // 切换回窗口模式
        OutputDebugStringA("切换到窗口模式...\n");

        // 1. 恢复显示设置
        ChangeDisplaySettingsW(NULL, 0);
        OutputDebugStringA("显示设置已恢复\n");

        // 2. 恢复窗口样式
        SetWindowLongW(m_hWnd, GWL_STYLE, m_WindowStyle);
        SetWindowLongW(m_hWnd, GWL_EXSTYLE, m_WindowExStyle);
        OutputDebugStringA("窗口样式已恢复\n");

        // 3. 恢复窗口位置和大小
        INT width = m_WindowRect.right - m_WindowRect.left;
        INT height = m_WindowRect.bottom - m_WindowRect.top;

        sprintf_s(buffer, sizeof(buffer), 
                 "恢复窗口位置: (%d,%d) 大小: %dx%d\n",
                 m_WindowRect.left, m_WindowRect.top, width, height);
        OutputDebugStringA(buffer);


        SetWindowPos(m_hWnd, HWND_TOP,
                     m_WindowRect.left, m_WindowRect.top,
                     width, height,
                     SWP_SHOWWINDOW | SWP_FRAMECHANGED);

        // 4. 显示光标
        ShowCursor(TRUE);

        m_Fullscreen = FALSE;
        OutputDebugStringA("切换到窗口模式成功\n");
    }

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
    char buffer[256];

    switch (msg)
    {
    case WM_CREATE:
        // 窗口创建
        return 0;

    case WM_DESTROY:
        // 窗口销毁
        PostQuitMessage(0);
        return 0;

    case WM_CLOSE:
        // 关闭窗口
        DestroyWindow(hWnd);
        return 0;

    case WM_SIZE:
        // 窗口大小改变
        {
            m_Minimized = false;
            m_Maximized = false;

            if (wParam == SIZE_MINIMIZED)
            {
                m_Minimized = true;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_Maximized = true;
            }
            else if (wParam == SIZE_RESTORED)
            {
                // 恢复正常大小
            }

            // TODO: 这里可以通知渲染器窗口大小改变
        }
        return 0;

    case WM_ACTIVATE:
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
            pMMI->ptMinTrackSize.x = 400;
            pMMI->ptMinTrackSize.y = 300;

            // 设置最大尺寸
            pMMI->ptMaxTrackSize.x = GetSystemMetrics(SM_CXSCREEN);
            pMMI->ptMaxTrackSize.y = GetSystemMetrics(SM_CYSCREEN);
        }
        return 0;

    // TODO: 按键设置
    case WM_KEYDOWN:
        sprintf_s(buffer, sizeof(buffer), "收到按键: 0x%X (虚拟键码)\n", wParam);
        OutputDebugStringA(buffer);

        // 按键按下
        if (wParam == VK_ESCAPE)
        {
            // ESC键 - 退出程序
            OutputDebugStringA("ESC 键被按下，退出程序\n");
            PostMessage(hWnd, WM_CLOSE, 0, 0);
        }
        else if (wParam == VK_F1)
        {
            // F1键 - 切换全屏
            OutputDebugStringA("F1 键被按下!\n");
            ToggleFullscreen();
        }
        return 0;

    case WM_KEYUP:
        // 按键释放
        return 0;

    case WM_SYSCOMMAND:
        // 系统命令
        switch (wParam)
        {
        case SC_SCREENSAVE:   // 屏保
        case SC_MONITORPOWER: // 显示器节能
            if (m_Fullscreen)
            {
                // 全屏模式下防止屏保和节能模式
                return 0;
            }
            break;

        case SC_KEYMENU: // 系统菜单
            if (m_Fullscreen)
            {
                // 全屏模式下禁用系统菜单
                return 0;
            }
            break;
        }
        break;

    case WM_ERASEBKGND:
        // 阻止擦除背景，减少闪烁
        return 1;
    }

    return DefWindowProcW(hWnd, msg, wParam, lParam);
}