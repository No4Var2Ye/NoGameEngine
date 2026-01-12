
// ======================================================================
#include "stdafx.h"

#include "Core/GameEngine.h"
#include "Core/Window.h"
#include "Core/Renderer.h"
#include "Core/InputManager.h"
#include "Graphics/Camera.h"
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
    : m_ShowDebugInfo(TRUE)
{
    // 初始化引擎子系统
    // 成员变量初始化
    m_Window = std::make_unique<CWindow>(); // 智能指针, 自动删除
    m_Renderer = std::make_unique<CRenderer>();
    m_InputManager = std::make_unique<CInputManager>();
    m_pMainCamera = std::make_unique<CCamera>();
    // m_ResourceManager = std::make_unique<CResourceManager>();
    m_SceneManager = std::make_unique<CSceneManager>();
}

// TODO: 初始化引擎
BOOL CGameEngine::Initialize(HINSTANCE hInstance, const EngineConfig &config)
{
    std::wcout << L"========= 引擎初始化开始 =========" << std::endl;

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

    // OutputDebugStringA("--- 开始设置回调 ---\n");

    m_Window->SetResizeCallback([this](INT w, INT h)
                                {
                                    OutputDebugStringA("！！！回调函数内部终于跑到了！！！\n");

                                    // 1. 通知渲染器更新视口 (glViewport)
                                    // 当窗口大小改变，通知渲染器更新视口
                                    if (m_Renderer)
                                        m_Renderer->Reset(w, h);

                                    // 2. 更新投影矩阵 (核心步骤)
                                    if (m_pMainCamera && h > 0) // 确保高度不为0防止除零异常
                                    {
                                        // 动态计算当前的 Aspect Ratio
                                        FLOAT aspect = (FLOAT)w / (FLOAT)h;

                                        // 更新相机内部的投影矩阵参数
                                        // 建议从 config 或相机现有值中获取 FOV, Near, Far
                                        FLOAT fov = m_pMainCamera->GetFOV();
                                        FLOAT nearP = m_pMainCamera->GetNear();
                                        FLOAT farP = m_pMainCamera->GetFar();

                                        m_pMainCamera->SetProjection(fov, aspect, nearP, farP);

                                        std::cout << "[Resize] 投影矩阵已更新, Aspect: " << aspect << std::endl;
                                    }

                                    // TODO: UI系统 UI重排
                                });

    INT startW = m_Window->GetClientWidth();
    INT startH = m_Window->GetClientHeight();

    m_Renderer->Reset(startW, startH);

    // 3. 初始化输入系统
    if (!m_InputManager->Initialize(m_Window->GetHWND(), hInstance))
    {
        return FALSE;
    }

    // 4. 初始化相机
    m_pMainCamera->SetMode(CameraMode::FreeLook);
    // 设置初始位置
    Vector3 position(0.0f, 0.0f, 10.0f);
    Vector3 target(0.0f, 0.0f, 0.0f); // 看向原点
    Vector3 up(0.0f, 1.0f, 0.0f);
    m_pMainCamera->Initialize(position, target, up);

    // 获取当前窗口宽高比
    FLOAT aspect = (FLOAT)m_Renderer->GetWidth() / m_Renderer->GetHeight();
    // std::cout << "ASPECT" << aspect << std::endl; // 1.6
    m_pMainCamera->SetProjection(45.0f, aspect, 0.1f, 1000.0f);
    // m_pMainCamera->EnableMouseLook(TRUE);

    // 5. 初始化资源管理器
    // TODO: Shader 纹理 默认字体
    // if (!m_ResourceManager->Initialize())
    // {
    //     return FALSE;
    // }

    // 6. 初始化场景管理器
    if (!m_SceneManager->Initialize())
    {
        return FALSE;
    }

    // 7. 显示窗口
    m_Window->Show();
    // 立即重绘窗口, 不经过消息队列
    // UpdateWindow(m_Window->GetHWND());

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
    // if (m_Config.lockMouse)
    // {
    //         m_InputManager->LockMouse();
    //         m_InputManager->HideCursor();
    // }

    // m_InputManager->LockMouse();
    // m_InputManager->HideCursor();

    m_pMainCamera->EnableMouseLook(TRUE);
    m_pMainCamera->SetMoveSpeed(2.0f);

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

        // 5. 输入处理
        this->ProcessInput(deltaTime);

        // 6. 更新游戏逻辑
        m_pMainCamera->Update(deltaTime);
        m_SceneManager->Update(deltaTime);

        // 渲染判断
        if (m_Window->IsActive() && !m_Window->IsMinimized())
        {
            // ================================================
            // 8. 渲染场景
            m_Renderer->BeginFrame();
            // ================================================

            m_pMainCamera->ApplyProjectionMatrix();
            m_pMainCamera->ApplyViewMatrix();

            m_SceneManager->Render();

            // 渲染 UI
            // m_Renderer->PushState();
            //         m_Renderer->SetOrthoProjection(...); // 切换为 2D 矩阵
            //         m_UIManager->Render();
            // m_Renderer->PopState();

            // 9. TODO: 显示调试信息 UI
            // DisplayDebugInfo();

            // ================================================
            m_Renderer->EndFrame();
            m_InputManager->ResetMouseWheel();
            m_InputManager->ClearDelta();
            // ================================================
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

    m_pMainCamera->Reset();
    m_InputManager->Shutdown();

    m_Renderer->Shutdown();
    m_Window->Destroy();

    m_Initialized = FALSE;

#ifdef MYDEBUG
    OutputDebugStringA("游戏引擎已完全关闭\n");
#endif
}

void CGameEngine::ProcessInput(FLOAT delatTime)
{
    // 1. 全局快捷键处理
    if (m_InputManager->IsKeyPressed(VK_F1))
    {
        m_ShowDebugInfo = !m_ShowDebugInfo;
    }

    // Vector3 pos = m_pMainCamera->GetPosition();
    // Vector3 tar = m_pMainCamera->GetTarget();
    // Vector3 fwd = m_pMainCamera->GetForward();

    // 获取摄像机的位置
    // if (m_InputManager->IsMouseButtonDown(MouseButton::Right))
    // {
    //     printf("\n--------- Camera Debug Info ---------\n");
    //     printf("Pos   : [%8.2f, %8.2f, %8.2f]\n", pos.x, pos.y, pos.z);
    //     printf("Target: [%8.2f, %8.2f, %8.2f]\n", tar.x, tar.y, tar.z);
    //     printf("Dir(F): [%8.2f, %8.2f, %8.2f]\n", fwd.x, fwd.y, fwd.z);
    //     printf("---------------------------------------\n");
    // }

    // 2. 相机模式快速切换 (用于测试)
    if (m_InputManager->IsKeyPressed('1'))
        GetMainCamera()->SetMode(CameraMode::FirstPerson);
    if (m_InputManager->IsKeyPressed('2'))
        GetMainCamera()->SetMode(CameraMode::ThirdPerson);
    if (m_InputManager->IsKeyPressed('3'))
        GetMainCamera()->SetMode(CameraMode::FreeLook);
    if (m_InputManager->IsKeyPressed('4'))
        GetMainCamera()->SetMode(CameraMode::Orbital);

    // 3. 触发震动测试
    if (m_InputManager->IsKeyPressed('G'))
    {
        GetMainCamera()->StartShake(0.2f, 0.1f);
    }

    // 4. 分发输入给子系统
    ProcessCameraInput(delatTime);
    // ProcessUIInput(deltaTime);
}

void CGameEngine::ProcessCameraInput(FLOAT deltaTime)
{
    // 调试输出
    static int cameraInputCount = 0;
    cameraInputCount++;

    // 0. 基础重置
    if (m_InputManager->IsKeyPressed('0') || m_InputManager->IsKeyPressed(VK_NUMPAD0))
    {
        m_pMainCamera->Reset();
    }

    CameraMode mode = m_pMainCamera->GetMode();

    // 1. 鼠标状态切换
    if (m_InputManager->IsMouseButtonPressed(MouseButton::Left))
    {
        m_pMainCamera->StartMouseLook();
        m_InputManager->HideCursor();
        m_InputManager->LockMouse(); // 限制光标在窗口内
    }

    if (m_InputManager->IsMouseButtonReleased(MouseButton::Left))
    {
        m_pMainCamera->StopMouseLook();
        m_InputManager->ShowCursor(); // 恢复鼠标显示
        m_InputManager->UnlockMouse();
    }

    // 2. 旋转与观察
    // 通常点击左键时才允许相机旋转，这样左键可以留给 UI 或 游戏内交互
    // 无论是否按下，都先获取 Delta，防止数据积压
    long dx = m_InputManager->GetMouseDeltaX();
    long dy = m_InputManager->GetMouseDeltaY();

    if (m_pMainCamera->IsMouseLookActive())
    {
        POINT delta = m_InputManager->GetMouseDelta();

        if (delta.x != 0 || delta.y != 0)
        {
            // char buffer[256];
            // sprintf_s(buffer, "[ProcessCameraInput] Processing mouse: dx=%d, dy=%d\n",
            //           delta.x, delta.y);
            // OutputDebugStringA(buffer);

            // 注意：dy 取反，因为鼠标向上移动应该是抬头
            m_pMainCamera->ProcessMouseMovement(delta.x, -delta.y);
        }
    }

    // 3. 缩放处理
    INT wheelDelta = m_InputManager->GetMouseWheelDelta();
    if (wheelDelta != 0)
    {
        // std::cout << "[鼠标控制] 滚轮滚动: delta=" << wheelDelta;
        // if (wheelDelta > 0)
        // {
        //     std::cout << " (向上滚动/放大)" << std::endl;
        // }
        // else
        // {
        //     std::cout << " (向下滚动/缩小)" << std::endl;
        // }

        m_pMainCamera->ProcessMouseWheel(wheelDelta);
        m_InputManager->ResetMouseWheel();
    }

    // 4. 移动处理 (键盘)
    // 获取相机当前模式，决定 WASD 是移动相机还是移动目标

    if (mode == CameraMode::FirstPerson || mode == CameraMode::FreeLook)
    {
        FLOAT fwd = 0.0f, right = 0.0f, up = 0.0f;

        if (m_InputManager->IsKeyDown('W'))
            fwd += 1.0f;
        if (m_InputManager->IsKeyDown('S'))
            fwd -= 1.0f;
        if (m_InputManager->IsKeyDown('D'))
            right += 1.0f;
        if (m_InputManager->IsKeyDown('A'))
            right -= 1.0f;
        if (m_InputManager->IsKeyDown('E') || m_InputManager->IsKeyDown(VK_SPACE))
            up += 1.0f; // 上升
        if (m_InputManager->IsKeyDown('Q'))
            up -= 1.0f; // 下降

        // 应用移动 (乘以 deltaTime 保证速度恒定)
        if (fwd != 0 || right != 0 || up != 0)
        {
            FLOAT length = sqrtf(fwd * fwd + right * right + up * up);

            if (length > 0.0f)
            {
                fwd /= length;
                right /= length;
                up /= length;
            }

            // std::cout << deltaTime << std::endl;
            // std::cout << "移动向量: fwd=" << fwd << ", right=" << right << ", up=" << up << std::endl;

            m_pMainCamera->Move(fwd * deltaTime, right * deltaTime, up * deltaTime);
        }
    }
    else if (mode == CameraMode::ThirdPerson)
    {
        // 在第三人称下，键盘通常用来移动“玩家对象”或“目标点”
        // 这里我们直接控制相机的 Target 偏移，或者你可以直接控制你的角色类
        Vector3 targetMove(0, 0, 0);
        if (m_InputManager->IsKeyDown('W'))
            targetMove.z -= 1.0f;
        if (m_InputManager->IsKeyDown('S'))
            targetMove.z += 1.0f;
        // ... 应用到你的角色或 camera->SetThirdPersonTarget()
    }

    m_InputManager->ClearDelta();
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

    // auto* renderer = GetRenderer();
    // auto* camera = GetMainCamera();

    // renderer->PushState();
    //     // 切换到正交投影（UI 模式）
    //     renderer->SetOrthoProjection(0, renderer->GetWidth(), renderer->GetHeight(), 0, -1, 1);

    //     // 示例：获取平滑 FPS 和 相机位置
    //     FLOAT fps = renderer->GetSmoothFPS();
    //     Vector3 pos = camera->GetPosition();

    //     // 这里调用你的字体渲染函数，例如：
    //     // m_pFont->DrawText(10, 10, "FPS: %.2f", fps);
    //     // m_pFont->DrawText(10, 30, "Cam: %.1f, %.1f, %.1f", pos.x, pos.y, pos.z);

    // renderer->PopState();
}

void CGameEngine::OnWindowResize(INT w, INT h) {
    if (m_Renderer) m_Renderer->Reset(w, h);
    if (m_pMainCamera) {
        FLOAT aspect = (h > 0) ? (FLOAT)w / h : 1.0f;
        m_pMainCamera->SetProjection(m_pMainCamera->GetFOV(), aspect, 0.1f, 1000.0f);
    }
}