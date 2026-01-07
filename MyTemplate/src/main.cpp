/**
 * @file main.cpp
 * @brief OpenGL 3D游戏主程序
 * @details 基于Win32 API和OpenGL的3D游戏框架
 */

// ======================================================================
// 课程设计需求:
// - 模型管理类
// - 摄像机类（摄像机可采用第一人称或者第三人称）
// - 交互管理类

// 构建初步三维场景(30%)
// - 天空盒和地形
// - 静态模型和动态模型

// 互动（20%）
// - 实现场景漫游，人机交互等功能
// - 可添加其他交互
// - 用DirectInput实现 5 %

// 加分功能（10%）
// - 光照效果
// - 光源类型
// - 光照模型
// - 渲染效果等

// ======================================================================
#include "stdafx.h"
#define TESTENGINE
// #define TESTWINDOW1
// #define TESTWINDOW2

#include "EngineConfig.h"
#include "Core/GameEngine.h"
#include "Core/Window.h"
// ======================================================================

/**
 * @brief Windows程序(宽字节)入口点
 */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
/**
 * @brief 程序入口点（Windows应用程序）
 *
 * @param hInstance 当前应用程序实例的句柄
 * @param hPrevInstance 上一个实例的句柄，Win32中通常为NULL
 * @param lpCmdLine 命令行参数字符串（不含程序名）
 * @param nCmdShow 窗口显示方式（最小化/最大化/正常等）
 *
 * @return int 程序退出代码
 */
{
    // 分配控制台
    AllocConsole();
    // 重定向标准输出到控制台
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
    std::cout << "=== 程序开始 ===" << std::endl;
    // FreeConsole();

#ifdef TESTENGINE
    // 初始化游戏引擎
    CGameEngine &engine = CGameEngine::GetInstance();

    // 配置引擎参数
    EngineConfig config;
    config.windowWidth = 1280;
    config.windowHeight = 800;
    config.fullscreen = FALSE;
    config.title = L"我的游戏引擎";

    // 初始化引擎
    if (!engine.Initialize(hInstance, config))
    {
        MessageBoxW(NULL, L"游戏引擎初始化失败!", L"错误", MB_OK | MB_ICONERROR);
        return -1;
    }

    // 运行主循环
    INT exitCode = engine.Run();

    // 清理资源
    engine.Shutdown();

    FreeConsole();

    return exitCode;
#endif // TESTENGINE

#ifdef TESTWINDOW1
    // 测试1: 普通窗口
    printf("测试1: 创建普通窗口\n");
    {
        CWindow win1;

        EngineConfig config1;
        config1.windowWidth = 1200;
        config1.windowHeight = 900;
        config1.fullscreen = FALSE;
        config1.title = L"我的GameEngine";

        if (win1.Create(GetModuleHandle(NULL), config1))
        {
            printf("成功\n");

            ShowWindow(win1.GetHWND(), SW_SHOW);
            UpdateWindow(win1.GetHWND());

            MessageBoxW(NULL, L"窗口已创建，点击确定关闭", L"测试", MB_OK);

            DestroyWindow(win1.GetHWND());
        }
        else
        {
            printf("失败\n");
        }
    }
    return 0;
#endif // TESTWINDOW1

#ifdef TESTWINDOW2
    // 测试2: 全屏窗口
    printf("测试2: 创建全屏窗口\n");
    {
        // 获取屏幕分辨率
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        printf("屏幕分辨率: %dx%d\n", screenWidth, screenHeight);

        CWindow win2;

        EngineConfig config2;
        config2.windowWidth = screenWidth;
        config2.windowHeight = screenHeight;
        config2.fullscreen = TRUE;
        config2.title = L"全屏测试窗口";

        if (win2.Create(GetModuleHandle(NULL), config2))
        {
            printf("成功\n");

            ShowWindow(win2.GetHWND(), SW_SHOW);
            UpdateWindow(win2.GetHWND());

            // MessageBoxW(NULL, L"窗口已创建，点击确定关闭", L"测试", MB_OK);

            MSG msg = {};
            while (GetMessage(&msg, NULL, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);

                // 按ESC退出
                if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE)
                {
                    DestroyWindow(win2.GetHWND());
                }
            }

            DestroyWindow(win2.GetHWND());
        }
        else
        {
            printf("失败\n");
        }
    }
    return 0;
#endif // TESTWINDOW1
}