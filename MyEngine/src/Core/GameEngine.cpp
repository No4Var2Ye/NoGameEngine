
// ======================================================================
#include "stdafx.h"
#include "EngineConfig.h"
#include "Core/GameEngine.h"
#include "Utils/DebugUtils.h"
#include "Core/Window.h"
#include "Core/Renderer.h"
#include "Core/InputManager.h"
// #include "Graphics/Camera/Camera.h"
#include "Entities/CameraEntity.h"
#include "Graphics/UI/UIManager.h"
#include "Resources/ResourceManager.h"
#include "Scene/SceneManager.h"
#include "Scene/DemoScene.h"
// ======================================================================

CGameEngine *CGameEngine::s_Instance = nullptr; // 初始化静态实例

CGameEngine &CGameEngine::GetInstance()
{
    if (!s_Instance)
    {
        s_Instance = new CGameEngine();
    }
    return *s_Instance;
}

// ======================================================================
// TODO: 引擎构造函数
// ======================================================================
CGameEngine::CGameEngine()
    : m_ShowDebugInfo(TRUE)
{
    // 初始化引擎子系统
    // 成员变量初始化
    m_Window = std::make_unique<CWindow>(); // 智能指针, 自动删除
    m_Renderer = std::make_unique<CRenderer>();
    m_InputManager = std::make_unique<CInputManager>();
    m_pMainCamera = nullptr;
    m_ResourceManager = std::make_unique<CResourceManager>();
    m_SceneManager = std::make_unique<CSceneManager>();
    m_UIManager = std::make_unique<CUIManager>();
}

// ======================================================================
// TODO: 引擎初始化
// ======================================================================
BOOL CGameEngine::Initialize(HINSTANCE hInstance, const EngineConfig &config)
{
    LogInfo(L"=--=--=--=--=--=--= 引擎初始化开始 =--=--=--=--=--=--=--=--=--=\n");

    // 判断是是否已经初始化
    if (m_Initialized)
        return TRUE;

    // 1. 创建窗口
    if (!m_Window->Create(hInstance, config))
    {
        return FALSE;
    }

    // 窗口创建后立即禁用输入法
    m_Window->DisableIME();
    m_Window->ForceEnglishKeyboardLayout();

    // 2. 初始化渲染器
    if (!m_Renderer->Initialize(m_Window->GetHWND()))
    {
        return FALSE;
    }

    INT startW = m_Window->GetClientWidth();
    INT startH = m_Window->GetClientHeight();

    m_Renderer->Reset(startW, startH);

    // 3. 初始化输入系统
    if (!m_InputManager->Initialize(m_Window->GetHWND(), hInstance))
    {
        return FALSE;
    }

    // 4. 初始化相机
    m_pMainCamera = CCameraEntity::Create();
    if (m_pMainCamera)
    {
        m_pMainCamera->SetPosition(Vector3(0.0f, 5.0f, 10.0f));
        FLOAT aspect = (startH > 0) ? (FLOAT)startW / (FLOAT)startH : 1.0f;
        m_pMainCamera->SetProjection(45.0f, aspect, 0.1f, 1000.0f);
        m_pMainCamera->SetMode(CameraMode::FreeLook);
    }

    // 5. 初始化资源管理器
    // TODO: Shader 纹理 默认字体
    if (!m_ResourceManager->Initialize(config.resConfig))
    {
        return FALSE;
    }
    if (!m_Renderer->InitializeFontSystem())
    {
        LogWarning(L"字体系统初始化失败，将继续运行\n");
    }

    // 6. 初始化场景管理器
    if (m_SceneManager)
    {
        // 1. 初始化场景管理器
        m_SceneManager->Initialize();

        // 2. 创建并注册 DemoScene
        auto pDemoScene = std::make_shared<CDemoScene>();
        m_SceneManager->RegisterScene(pDemoScene);

        // 3. 立即切换到该场景 (不带过渡效果，作为首个场景)
        m_SceneManager->ChangeSceneImmediate("DemoScene");
    }

    // 7. 初始化UI系统
    if (!m_UIManager->Initialize(m_Window->GetHWND(), L"Microsoft YaHei", 18))
    {
        return FALSE;
    }

    // 8. 显示窗口
    m_Window->Show();

    m_Initialized = TRUE;
    return TRUE;
}

// ======================================================================
// TODO: 引擎运行
// ======================================================================
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

    // m_pMainCamera->EnableMouseLook(TRUE);
    // m_pMainCamera->SetMoveSpeed(2.0f);

    // 跟踪上一次的窗口尺寸，用于检测变化
    INT lastWindowWidth = m_Window->GetClientWidth();
    INT lastWindowHeight = m_Window->GetClientHeight();

    // 主游戏循环
    while (m_Running)
    {
        // 1. 处理Windows消息
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                if (m_State == EngineState::Running)
                {
                    m_State = EngineState::FadeOut;
                    m_SplashTimer = 0.0f; // 重置计时器
                    continue;             // 不设置 m_Running = FALSE，继续跑循环
                }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!m_Running)
            break;

        // 在安全区检测并处理缩放
        INT currentWidth = m_Window->GetClientWidth();
        INT currentHeight = m_Window->GetClientHeight();

        // 检测窗口大小是否真的发生了变化
        if (currentWidth != lastWindowWidth || currentHeight != lastWindowHeight)
        {
            // 确保尺寸有效
            INT safeW = (currentWidth < 1) ? 1 : currentWidth;
            INT safeH = (currentHeight < 1) ? 1 : currentHeight;

            // 更新渲染器视口
            if (m_Renderer)
            {
                m_Renderer->Reset(safeW, safeH);

                // sprintf_s(debugMsg, "渲染器视口已重置\n");
                // OutputDebugStringA(debugMsg);
            }

            // 更新相机投影矩阵
            if (m_pMainCamera)
            {
                FLOAT newAspect = (FLOAT)safeW / (FLOAT)safeH;
                m_pMainCamera->SetProjection(m_pMainCamera->GetFOV(), newAspect, m_pMainCamera->GetNear(), m_pMainCamera->GetFar());
            }

            // TODO: 更新UI系统

            // 更新最后记录的尺寸
            lastWindowWidth = safeW;
            lastWindowHeight = safeH;

            // OutputDebugStringA(">>> 窗口大小更新完成\n");
        }

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
            switch (m_State)
            {
            case EngineState::FadeIn:
            {
                RenderSplashScreen(deltaTime, false);
                break;
            }
            case EngineState::Running:
            {
                // 渲染主场景
                m_SceneManager->Render();

                // 渲染UI
                m_Renderer->PushState();
                {
                    if (m_ShowDebugInfo)
                        DisplayDebugInfo();
                }
                m_Renderer->PopState();

                // 检测退出按键切换到 FadeOut
                if (m_InputManager->IsKeyPressed(VK_ESCAPE))
                    m_State = EngineState::FadeOut;
            }
            break;

            case EngineState::FadeOut:
            {
                RenderSplashScreen(deltaTime, true);
                break;
            }
            case EngineState::Finished:
            {
                m_Running = FALSE;
                break;
            }
            }

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

// ======================================================================
// TODO: 引擎停止
void CGameEngine::Shutdown()
{
    m_Running = FALSE;

    // 恢复输入法状态
    if (m_Window)
    {
        m_Window->EnableIME();
    }

    // 逆序关闭子系统
    m_UIManager->Shutdown();

    m_SceneManager->Shutdown();
    // m_ResourceManager->Shutdown();

    // m_pMainCamera->Reset();
    m_InputManager->Shutdown();

    m_Renderer->Shutdown();
    m_Window->Destroy();

    m_Initialized = FALSE;

    LogInfo(L"=--=--=--=--=--=--= 引擎已完全关闭 =--=--=--=--=--=--=--=--=---=\n");
    Sleep(1000);
}
// ======================================================================

void CGameEngine::ProcessInput(FLOAT deltaTime)
{
    // 1. 全局快捷键处理
    if (m_InputManager->IsKeyPressed(VK_ESCAPE))
    {
        // 如果当前是运行状态，开启淡出
        if (m_State == EngineState::Running)
        {
            m_State = EngineState::FadeOut;
            m_SplashTimer = 0.0f;
        }
    }

    if (m_InputManager->IsKeyPressed(Hotkeys::ToggleDebugInfo))
    {
        m_ShowDebugInfo = !m_ShowDebugInfo;
    }

    // 2. ESC键退出时恢复输入法
    if (m_InputManager->IsKeyPressed(VK_ESCAPE))
    {
        m_Window->EnableIME(); // 退出前恢复输入法
    }

    // 3. 分发输入给子系统
    ProcessCameraInput(deltaTime);
    ProcessUIInput(deltaTime);
}

void CGameEngine::ProcessCameraInput(FLOAT deltaTime)
{
    if (!m_pMainCamera)
        return;

    // ======================================================================
    // 0. 基础重置
    // 1. 旋转与观察
    // 2. 缩放处理
    // 3. 移动处理
    //  -> 已经迁移到Scene中
    // ======================================================================
}

void CGameEngine::ProcessUIInput(FLOAT deltaTime)
{
}

void CGameEngine::DisplayDebugInfo()
{
    if (!m_ShowDebugInfo)
        return;

    // 1. 定义起始位置和行间距
    INT startX = 30;
    INT startY = 40;
    INT lineHeight = 22; // 每一行的高度差
    INT row = 0;         // 使用行倍数，方便排列

    // 右侧：操作指南
    // 定义右侧边距和行高
    INT windowWidth = m_Renderer->GetWidth();
    INT rightMargin = 250; // 预留给右侧文字的宽度
    INT rightX = windowWidth - rightMargin;
    INT rightY = 40; // 与左侧对齐或自定义
    INT rRow = 0;

    // 设置颜色
    FLOAT black[] = {0.0f, 0.0f, 0.0f, 1.0f};
    FLOAT white[] = {1.0f, 1.0f, 1.0f, 1.0f};
    FLOAT gray[] = {0.7f, 0.7f, 0.7f, 1.0f};
    FLOAT green[] = {0.0f, 1.0f, 0.0f, 1.0f};
    FLOAT yellow[] = {1.0f, 1.0f, 0.0f, 1.0f};
    FLOAT cyan[] = {0.0f, 1.0f, 1.0f, 1.0f};
    FLOAT orange[] = {1.0f, 0.5f, 0.0f, 1.0f};

    // ======================================================================
    // 1. 性能与内存模块 (Performance & Memory)
    // FPS 和 帧时间 (ms)
    float frameTime = 1000.0f / (m_Renderer->GetFPS() + 0.001f); // 避免除零
    std::string perfText = "FPS: " + std::to_string(static_cast<INT>(m_Renderer->GetFPS())) +
                           " (" + std::to_string(frameTime).substr(0, 4) + " ms)";
    m_Renderer->RenderText2D(perfText, startX, startY + (lineHeight * row++), green, 1.0f);

    size_t usedMem = DebugUtils::GetUsedMemoryMB();
    std::string memText = "Memory: " + std::to_string(usedMem) + " MB / " +
                          std::to_string(DebugUtils::GetTotalMemoryMB()) + " MB";
    m_Renderer->RenderText2D(memText, startX, startY + (lineHeight * row++), green, 1.0f);

    // ======================================================================
    // 2. 引擎状态
    std::string stateText = "VSync: " + std::string(m_Renderer->IsVSyncEnabled() ? "ON" : "OFF") +
                            " | Window: " + std::to_string(m_Renderer->GetWidth()) + "x" + std::to_string(m_Renderer->GetHeight());
    m_Renderer->RenderText2D(stateText, startX, startY + (lineHeight * row++), orange, 1.0f);

    row++;

    // ======================================================================
    // 3. 相机数据
    std::string camParaText = "[主相机的参数]";
    m_Renderer->RenderText2D(camParaText, startX, startY + (lineHeight * row++), cyan, 1.0f);
    
    Vector3 camPos = m_pMainCamera->GetPosition();
    std::string camText = "Pos: (" + std::to_string(camPos.x).substr(0, 5) + ", " +
                          std::to_string(camPos.y).substr(0, 5) + ", " +
                          std::to_string(camPos.z).substr(0, 5) + ")";
    m_Renderer->RenderText2D(camText, startX, startY + (lineHeight * row++), cyan, 1.0f);

    Vector3 forward = m_pMainCamera->GetForward();
    std::string dirText = "Dir: (" + std::to_string(forward.x).substr(0, 5) + ", " +
                          std::to_string(forward.y).substr(0, 5) + ", " +
                          std::to_string(forward.z).substr(0, 5) + ")";
    m_Renderer->RenderText2D(dirText, startX, startY + (lineHeight * row++), cyan, 1.0f);

    std::string tmpText = "[后续修补场景相机参数读取]";
    m_Renderer->RenderText2D(tmpText, startX, startY + (lineHeight * row++), cyan, 1.0f);
    

    // 相机模式处理
    CameraMode mode = m_pMainCamera->GetMode();
    std::string modeStr = "Mode: ";
    if (mode == CameraMode::FirstPerson)
        modeStr += "First Person";
    else if (mode == CameraMode::ThirdPerson)
        modeStr += "Third Person";
    else if (mode == CameraMode::FreeLook)
        modeStr += "Free Look";
    else
        modeStr += "Orbital";
    m_Renderer->RenderText2D(modeStr, startX, startY + (lineHeight * row++), cyan, 1.0f);

    row++;

    // ======================================================================
    // 4. 输入状态模块 (Input Interaction)
    // 获取实时按键和鼠标位置
    std::string inputState = "Mouse: (" + std::to_string(m_InputManager->GetMouseX()) + ", " +
                             std::to_string(m_InputManager->GetMouseY()) + ") | Keys: ";
    if (m_InputManager->IsKeyDown(Hotkeys::MoveForward))
        inputState += "Forward ";
    if (m_InputManager->IsKeyDown(Hotkeys::MoveBackward))
        inputState += "Backward ";
    if (m_InputManager->IsKeyDown(Hotkeys::MoveLeft))
        inputState += "Left ";
    if (m_InputManager->IsKeyDown(Hotkeys::MoveRight))
        inputState += "Right ";
    if (m_InputManager->IsKeyDown(Hotkeys::MoveUp))
        inputState += "Up ";
    if (m_InputManager->IsKeyDown(Hotkeys::MoveDown))
        inputState += "Down ";

    m_Renderer->RenderText2D(inputState, startX, startY + (lineHeight * row++), black, 0.9f);

    row++;

    // ======================================================================
    // TODO: 4. 操作提示
    m_Renderer->RenderText2D("[ 快捷键 ]", rightX, rightY + (lineHeight * rRow++), black, 0.8f);
    m_Renderer->RenderText2D("ESC: 退出系统", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("F1 : 切换系统信息显示", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("F2 : 切换模型线框显示", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("F3 : 切换模型法线显示", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("F4 : 切换包围盒子显示", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("F5 : 切换网格系统显示", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("F11: 切换系统全屏显示", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("鼠标移动: 移动相机", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("鼠标滚动: 缩放视野", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("0: 重置相机位置", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("1/2/3/4: 切换相机模式", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("W/A/S/D: 相机移动", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("Q/E: 相机垂直升降", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("G: 摄像机抖动", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);

    rRow++;
    m_Renderer->RenderText2D("[ 相机模式 ]", rightX, rightY + (lineHeight * rRow++), black, 0.8f);
    m_Renderer->RenderText2D("1: 第一人称视角", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("2: 第三人称视角", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("3: 自由视角", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("4: 轨道视角", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
}

void CGameEngine::SetState(EngineState newState)
{
    if (newState == EngineState::FadeOut && m_State == EngineState::Running)
    {
        m_SplashTimer = 0.0f;
    }
    m_State = newState;
}

void CGameEngine::RenderSplashScreen(FLOAT deltaTime, BOOL isFadeOut)
{
    FLOAT currentDuration = isFadeOut ? FadeOutDuration : FadeInDuration;

    INT lineHeight = 22; // 每一行的高度差
    INT row = 0;         // 使用行倍数，方便排列

    m_SplashTimer += deltaTime;

    // 计算 Alpha 值 (0.0 到 1.0)
    FLOAT alpha = m_SplashTimer / currentDuration;
    if (alpha > 1.0f)
        alpha = 1.0f;

    // 如果是退出动画，Alpha 取反
    FLOAT drawAlpha = isFadeOut ? (1.0f - alpha) : alpha;

    // 清屏为纯黑 (这是关键，动画期间不显示场景)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 设置文字颜色（白色，带动态透明度）
    FLOAT textColor[] = {1.0f, 1.0f, 1.0f, drawAlpha};

    // 计算屏幕中心
    INT centerX = m_Renderer->GetWidth() / 2 - 40; // 粗略偏移
    INT centerY = m_Renderer->GetHeight() / 2;

    // 渲染 QMT
    m_Renderer->RenderText2D("Q M T", centerX, centerY + (lineHeight * row++), textColor, 1.0f);
    m_Renderer->RenderText2D("- - - - - - - - -", centerX - 30, centerY + (lineHeight * row++), textColor, 1.0f);
    m_Renderer->RenderText2D("3D程序设计", centerX - 20, centerY + (lineHeight * row++), textColor, 1.0f);

    // 状态切换逻辑
    if (m_SplashTimer >= currentDuration)
    {
        m_SplashTimer = 0.0f;
        if (!isFadeOut)
        {
            m_State = EngineState::Running;
            m_Renderer->SetClearColor(m_Renderer->GetClearColor());
        }
        else
        {
            m_State = EngineState::Finished;
        }
    }
}