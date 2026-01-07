/**
 * @file MyEngine/main.cpp
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
// - 静态模型
// - 动态模型

// 互动（20%）
// - 实现场景漫游，人机交互等功能
// - 可添加其他交互
// TODO: 有空再做
// - 用DirectInput实现 5 %

// 加分功能（10%）
// - 光照效果
// - 光源类型
// - 光照模型
// - 渲染效果等

// ======================================================================
#include "stdafx.h"

// ======================================================================
#include "EngineConfig.h"
#include "Core/GameEngine.h"
// ======================================================================


// Windows程序(宽字节)入口点
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
/**
 * @brief Windows应用程序入口点
 *
 * @param hInstance 当前应用程序实例的句柄
 * @param hPrevInstance 上一个实例的句柄，Win32中通常为NULL
 * @param lpCmdLine 命令行参数字符串
 * @param nCmdShow 窗口显示方式
 *
 * @return int 程序退出代码
 */
{
    
#ifdef MYDEDUG
    // 分配控制台
    AllocConsole();

    // 重定向标准输出到控制台
    FILE *fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
#endif // MYDEDUG

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

#ifdef MYDEDUG
    // 释放控制台
    FreeConsole();
#endif // MYDEDUG

    return exitCode;
} // wWinMain