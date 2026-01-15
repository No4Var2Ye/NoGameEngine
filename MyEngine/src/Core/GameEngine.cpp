
// ======================================================================
#include "stdafx.h"
#include "Core/GameEngine.h"
#include "Utils/DebugUtils.h"
#include "Core/Window.h"
#include "Core/Renderer.h"
#include "Core/InputManager.h"
#include "Graphics/Camera.h"
#include "Graphics/UIManager.h"
#include "Resources/ResourceManager.h"
#include "Scene/SceneManager.h"
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
    m_pMainCamera = std::make_unique<CCamera>();
    m_ResourceManager = std::make_unique<CResourceManager>();
    m_SceneManager = std::make_unique<CSceneManager>();
    m_UIManager = std::make_unique<CUIManager>();
}

// ======================================================================
// TODO: 引擎初始化
// ======================================================================
BOOL CGameEngine::Initialize(HINSTANCE hInstance, const EngineConfig &config)
{
    LogInfo(L"=--=--=--=--=--=--=--= 引擎初始化开始 =--=--=--=--=--=--=--=\n");

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
    m_pMainCamera->SetMode(CameraMode::FreeLook);
    // 设置初始位置
    Vector3 position(0.0f, 0.0f, 10.0f);
    Vector3 target(0.0f, 0.0f, 0.0f); // 看向原点
    Vector3 up(0.0f, 1.0f, 0.0f);
    m_pMainCamera->Initialize(position, target, up);

    // 获取当前窗口宽高比
    FLOAT aspect = (startH > 0) ? (FLOAT)startW / (FLOAT)startH : 1.0f;
    m_pMainCamera->SetProjection(45.0f, aspect, 0.1f, 1000.0f);

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
    if (!m_SceneManager->Initialize())
    {
        return FALSE;
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

    m_pMainCamera->EnableMouseLook(TRUE);
    m_pMainCamera->SetMoveSpeed(2.0f);

    // 跟踪上一次的窗口尺寸，用于检测变化
    INT lastWindowWidth = m_Window->GetClientWidth();
    INT lastWindowHeight = m_Window->GetClientHeight();

    // 主游戏循环
    while (m_Running)
    {
        // debugFrame++;

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
                // 重新计算宽高比
                FLOAT newAspect = (FLOAT)safeW / (FLOAT)safeH;

                // 调用 SetProjection 更新相机内部的 m_AspectRatio
                m_pMainCamera->SetProjection(
                    m_pMainCamera->GetFOV(),
                    newAspect,
                    m_pMainCamera->GetNear(),
                    m_pMainCamera->GetFar());

                // 强制重新计算投影矩阵
                m_pMainCamera->ApplyProjectionMatrix();

                // 打印调试信息
                // sprintf_s(debugMsg,
                //           "相机投影矩阵更新: 宽高比=%.4f, FOV=%.1f\n",
                //           newAspect, m_pMainCamera->GetFOV());
                // OutputDebugStringA(debugMsg);
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

                m_pMainCamera->ApplyProjectionMatrix();
                m_pMainCamera->ApplyViewMatrix();
                m_SceneManager->Render();
                

                m_Renderer->PushState();
                if (m_ShowDebugInfo)
                    DisplayDebugInfo();
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

    m_pMainCamera->Reset();
    m_InputManager->Shutdown();

    m_Renderer->Shutdown();
    m_Window->Destroy();

    m_Initialized = FALSE;

    LogInfo(L"=--=--=--=--=--=--=--= 引擎已完全关闭 =--=--=--=--=--=--=--=\n");
    Sleep(1000);
}

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

    if (m_InputManager->IsKeyPressed(VK_F1))
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
    // 0. 基础重置
    if (m_InputManager->IsKeyPressed('0') || m_InputManager->IsKeyDown(VK_NUMPAD0))
    {
        // OutputDebugStringA("DEBUG: Reset Key Pressed!\n");
        m_pMainCamera->Reset();
    }

    // 相机模式快速切换
    if (m_InputManager->IsKeyPressed('1'))
        GetMainCamera()->SetMode(CameraMode::FirstPerson);
    if (m_InputManager->IsKeyPressed('2'))
        GetMainCamera()->SetMode(CameraMode::ThirdPerson);
    if (m_InputManager->IsKeyPressed('3'))
        GetMainCamera()->SetMode(CameraMode::FreeLook);
    if (m_InputManager->IsKeyPressed('4'))
        GetMainCamera()->SetMode(CameraMode::Orbital);

    // 触发震动测试
    if (m_InputManager->IsKeyPressed('G'))
    {
        GetMainCamera()->StartShake(0.2f, 0.1f);
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
    if (m_InputManager->IsKeyDown('W'))
        inputState += "W ";
    if (m_InputManager->IsKeyDown('A'))
        inputState += "A ";
    if (m_InputManager->IsKeyDown('S'))
        inputState += "S ";
    if (m_InputManager->IsKeyDown('D'))
        inputState += "D ";
    if (m_InputManager->IsKeyDown('E'))
        inputState += "E ";
    if (m_InputManager->IsKeyDown('Q'))
        inputState += "Q ";

    m_Renderer->RenderText2D(inputState, startX, startY + (lineHeight * row++), black, 0.9f);

    row++;

    // ======================================================================
    // TODO: 4. 操作提示
    m_Renderer->RenderText2D("[ 快捷键 ]", rightX, rightY + (lineHeight * rRow++), black, 0.8f);
    m_Renderer->RenderText2D("ESC: 退出系统", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("F1 : 切换信息显示", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
    m_Renderer->RenderText2D("F11: 切换全屏显示", rightX, rightY + (lineHeight * rRow++), gray, 0.75f);
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

void CGameEngine::TestFontRendering()
{
    // OutputDebugStringA("开始字体渲染测试...\n");

    FLOAT colors[][4] = {
        {1.0f, 0.0f, 0.0f, 1.0f}, // 红
        {0.0f, 1.0f, 0.0f, 1.0f}, // 绿
        {0.0f, 0.0f, 1.0f, 1.0f}, // 蓝
    };

    // 测试1：简单文字
    m_Renderer->RenderText2D("字体测试 - 红色", 50, 50, colors[0], 1.0f);
    m_Renderer->RenderText2D("字体测试 - 绿色", 50, 70, colors[1], 1.0f);
    m_Renderer->RenderText2D("字体测试 - 蓝色", 50, 90, colors[2], 1.0f);

    // 测试2：不同大小
    m_Renderer->RenderText2D("大号字体", 50, 120, colors[0], 1.5f);
    m_Renderer->RenderText2D("中号字体", 50, 150, colors[1], 1.0f);
    m_Renderer->RenderText2D("小号字体", 50, 170, colors[2], 0.7f);

    // OutputDebugStringA("字体渲染测试完成\n");
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