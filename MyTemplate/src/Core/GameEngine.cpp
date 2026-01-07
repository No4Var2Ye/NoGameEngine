
#include "stdafx.h"
#include "Core/GameEngine.h"
#include "Core/Window.h"
#include "Core/Renderer.h"
#include "Core/InputManager.h"
// #include "Resources/ResourceManager.h"
#include "Scene/SceneManager.h"

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
    std::cout << "=== 引擎初始化开始 ===" << std::endl;

    if (m_Initialized) // 判断是是否已经初始化
        return true;

    // TODO: 创建窗口
    if (!m_Window->Create(hInstance, config))
    {
        return false;
    }

    // 显示窗口
    m_Window->Show();
    // 立即重绘窗口, 不经过消息队列
    UpdateWindow(m_Window->GetHWND());
    
    // 2. 初始化渲染器
    if (!m_Renderer->Initialize(m_Window->GetHWND()))
    {
        return false;
    }
    UpdateWindow(m_Window->GetHWND());

    // 3. 初始化输入系统
    if (!m_InputManager->Initialize(m_Window->GetHWND(), hInstance))
    {
        return false;
    }

    // 4. 初始化资源管理器
    // if (!m_ResourceManager->Initialize())
    // {
    //     return false;
    // }

    // 5. 初始化场景管理器
    if (!m_SceneManager->Initialize())
    {
        return false;
    }

    m_Initialized = true;
    return true;
}

// TODO: 引擎运行
INT CGameEngine::Run()
{
    if (!m_Initialized)
    {
        return -1;
    }

    m_Running = true;
    // MSG结构体是 Windows 消息系统的核心，用于在应用程序和操作系统之间传递消息。
    // 它是 Windows 事件驱动编程的基础。
    MSG msg = {};

    // 获取时间基准
    DWORD lastTime = GetTickCount();
    DWORD currentTime = 0;
    float deltaTime = 0.0f;

    // TODO: 初始化状态
    // m_InputManager->HideCursor();
    // m_InputManager->LockMouse();

    // 主游戏循环
    while (m_Running)
    {
        // 1. 处理Windows消息
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                m_Running = false;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        // 2. 计算帧时间
        currentTime = GetTickCount();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // 限制帧率
        if (deltaTime < 0.016f) // 约60FPS
        {
            Sleep(16 - static_cast<DWORD>(deltaTime * 1000));
            continue;
        }

        // 3. 更新输入状态
        m_InputManager->Update();
        
        // TODO: 输入处理
        // InputProcess
        // 相机的处理

        // 4. 更新游戏逻辑
        m_SceneManager->Update(deltaTime);

        // 5. 渲染场景
        m_Renderer->BeginFrame();
        m_SceneManager->Render();
        m_Renderer->EndFrame();
    }

    return static_cast<INT>(msg.wParam);
}

// TODO: 引擎停止
void CGameEngine::Shutdown()
{
    m_Running = false;

    // 逆序关闭子系统
    m_SceneManager->Shutdown();
    // m_ResourceManager->Shutdown();
    m_InputManager->Shutdown();
    m_Renderer->Shutdown();
    m_Window->Destroy();

    m_Initialized = false;
}