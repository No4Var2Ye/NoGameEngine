
// ======================================================================
#ifndef __ENGINECONFIG_H__
#define __ENGINECONFIG_H__
// ======================================================================

// ======================================================================
#include <string>
// ======================================================================

namespace Hotkeys
{
    // 系统级
    const UINT ToggleDebugInfo = VK_F1;   // 切换左侧调试文字显示
    const UINT ToggleWireframe = VK_F2;   // 切换线框模式
    const UINT ToggleNormals = VK_F3;     // 切换法线显示
    const UINT ToggleBoundingBox = VK_F4; // 切换包围盒显示
    const UINT ToggleGrid = VK_F5;        // 全屏切换网格系统
    const UINT ToggleFullScreen = VK_F11; // 全屏切换预留

    // 相机与控制
    const UINT ResetCamera = '0';     // 重置相机位置
    const UINT CameraMode1 = '1';     // 第一人称
    const UINT CameraMode2 = '2';     // 第三人称
    const UINT CameraMode3 = '3';     // 自由视角
    const UINT CameraMode4 = '4';     // 轨道视角
    const UINT CameraShakeTest = 'G'; // 震动测试

    // 常用移动
    const UINT MoveForward = 'W';
    const UINT MoveBackward = 'S';
    const UINT MoveLeft = 'A';
    const UINT MoveRight = 'D';
    const UINT MoveUp = 'E';
    const UINT MoveUpAlt = VK_SPACE;
    const UINT MoveDown = 'Q';
}

struct ResourceConfig
{
    std::wstring rootPath = L"assets/";

    // 子目录路径
    std::wstring modelDir = L"Models/";
    std::wstring textureDir = L"Textures/";
    std::wstring shaderDir = L"Shaders/";
    std::wstring fontDir = L"Fonts/";
    std::wstring soundDir = L"Sounds/";
    std::wstring skyboxPath = L"Textures/Skybox/";

    // 辅助方法：获取完整路径
    std::wstring GetRootPath() const { return rootPath; }
    std::wstring GetModelPath() const { return rootPath + modelDir; }
    std::wstring GetTexturePath() const { return rootPath + textureDir; }
    std::wstring GetSoundPath() const { return rootPath + soundDir; }
    std::wstring GetSkyboxPath() const { return rootPath + skyboxPath; }
};

// ======================================================================
// 引擎配置结构
struct EngineConfig
{
    INT windowWidth = 800;                  // 窗口宽度
    INT windowHeight = 600;                 // 窗口高度
    BOOL fullscreen = FALSE;                // 是否全屏
    const wchar_t *title = L"QMT's GameEngine"; // 窗口标题
    // TODO: 配置窗口参数
    INT colorBits = 32;      // 颜色位数
    INT depthBits = 24;      // 深度缓冲位数
    INT stencilBits = 8;     // 模板缓冲位数
    BOOL vsync = TRUE;       // 垂直同步
    BOOL showCursor = TRUE;  // 显示光标
    BOOL resizable = TRUE;   // 窗口可调整大小
    BOOL borderless = FALSE; // 无边框窗口
    // 资源路径配置
    ResourceConfig resConfig;
};

#endif // __ENGINECONFIG_H__