
// ======================================================================
#include "stdafx.h"

#include "Core/GameEngine.h"
#include "Core/Window.h"
#include "Core/Renderer.h"
#include "Core/InputManager.h"
// #include "Resources/ResourceManager.h"
#include "Scene/SceneManager.h"
// ======================================================================

// 初始化静态实例
CGameEngine *CGameEngine::s_Instance = nullptr;

CGameEngine &CGameEngine::GetInstance()
{
    if (!s_Instance)
    {
        s_Instance = new CGameEngine();
    }
    return *s_Instance;
}

CGameEngine::CGameEngine()
{
    // 初始化引擎子系统
    // 成员变量初始化
    m_Window = std::make_unique<CWindow>(); // 智能指针, 自动删除
    m_Renderer = std::make_unique<CRenderer>();
    m_InputManager = std::make_unique<CInputManager>();
    // m_ResourceManager = std::make_unique<CResourceManager>();
    m_SceneManager = std::make_unique<CSceneManager>();
}

// TODO: 初始化引擎
BOOL CGameEngine::Initialize(HINSTANCE hInstance, const EngineConfig &config)
{
    // std::wcout << L"========= 引擎初始化开始 =========" << std::endl;

    // 判断是是否已经初始化
    if (m_Initialized)
        return TRUE;

    // 1. 创建窗口
    if (!m_Window->Create(hInstance, config))
    {
        return FALSE;
    }

    // 2. 初始化渲染器
    if (!m_Renderer->Initialize(m_Window->GetHWND()))
    {
        return FALSE;
    }

    m_Window->SetResizeCallback([this](int w, int h)
                                {
                                    // 当窗口大小改变，通知渲染器更新视口
                                    if (m_Renderer)
                                    {
                                        m_Renderer->Reset(w, h);
                                    }
                                    // TODO: UI系统 UI重排
                                });

    // 3. 初始化输入系统
    if (!m_InputManager->Initialize(m_Window->GetHWND(), hInstance))
    {
        return FALSE;
    }

    // 4. 初始化资源管理器
    // TODO: Shader 纹理 默认字体
    // if (!m_ResourceManager->Initialize())
    // {
    //     return FALSE;
    // }

    // 5. 初始化场景管理器
    if (!m_SceneManager->Initialize())
    {
        return FALSE;
    }
    // 6. 显示窗口
    m_Window->Show();
    // 立即重绘窗口, 不经过消息队列
    UpdateWindow(m_Window->GetHWND());

    m_Initialized = TRUE;
    return TRUE;
}

// TODO: 引擎运行
INT CGameEngine::Run()
{
    if (!m_Initialized)
        return -1;

    m_Running = TRUE;
    // MSG结构体是 Windows 消息系统的核心，用于在应用程序和操作系统之间传递消息。
    // 它是 Windows 事件驱动编程的基础。
    MSG msg = {};

    // 获取时间基准
    // deltaTime用于使游戏速度与帧率无关，确保在不同性能的电脑上游戏体验一致

    // TODO: 初始输入状态
    // if (m_Config.lockMouse) {
    //     m_InputManager->SetMouseLock(true);
    //     m_InputManager->HideCursor(true);
    // }

    // 主游戏循环
    while (m_Running)
    {
        // 1. 处理Windows消息
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                m_Running = FALSE;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!m_Running)
            break;

        // 2. 获取高精度 DeltaTime
        // 此时 CRenderer 已经在上一次 EndFrame 锁定了时间
        FLOAT deltaTime = m_Renderer->GetDeltaTime();

        // 3. 性能断点保护
        if (deltaTime > 0.1f)
            deltaTime = 0.0166f;

        // 4. 系统输入轮询
        m_InputManager->Update();

        // TODO: 5. 输入处理
        // InputProcess
        // 相机的处理
        // 在这里处理相机旋转、角色移动等

        // 6. 更新游戏逻辑
        m_SceneManager->Update(deltaTime);

        // 渲染判断
        if (m_Window->IsActive() && !m_Window->IsMinimized())
        {
            // 8. 渲染场景
            m_Renderer->BeginFrame();
            m_SceneManager->Render();

            // 9. TODO: 显示调试信息
            // DisplayDebugInfo();

            m_Renderer->EndFrame();
        }
        else
        {
            Sleep(10); // 窗口失去焦点或最小化时释放 CPU
        }
    }

    return static_cast<INT>(msg.wParam);
}

// TODO: 引擎停止
void CGameEngine::Shutdown()
{
    m_Running = FALSE;

    // 逆序关闭子系统
    m_SceneManager->Shutdown();
    // m_ResourceManager->Shutdown();
    m_InputManager->Shutdown();
    m_Renderer->Shutdown();
    m_Window->Destroy();

    m_Initialized = FALSE;
}

void CGameEngine::DisplayDebugInfo()
{
    // 如果没有文字渲染器，直接返回

    // 只在需要时显示

    // 计算程序运行时间

    // 创建调试信息字符串

    // 基本FPS信息

    // 帧时间信息

    // 运行时间

    // 相机位置和旋转（如果有相机）
}