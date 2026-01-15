
// ======================================================================
#ifndef __ENGINECONFIG_H__
#define __ENGINECONFIG_H__
// ======================================================================
#include <string>
// ======================================================================
struct ResourceConfig
{
    // 基础根路径 (相对于可执行文件或项目根目录)
    std::wstring rootPath = L"res/";

    // 子目录路径
    std::wstring modelDir   = L"Models/";
    std::wstring textureDir = L"Textures/";
    std::wstring shaderDir  = L"Shaders/";
    std::wstring fontDir    = L"Fonts/";
    std::wstring soundDir   = L"Sounds/";

    // 辅助方法：获取完整路径
    std::wstring GetRootPath() const { return rootPath; }
    std::wstring GetModelPath() const { return rootPath + modelDir; }
    std::wstring GetTexturePath() const { return rootPath + textureDir; }
};

// ======================================================================
// 引擎配置结构
struct EngineConfig
{
    INT windowWidth = 800;                  // 窗口宽度
    INT windowHeight = 600;                 // 窗口高度
    BOOL fullscreen = FALSE;                // 是否全屏
    const wchar_t *title = L"MyGameEngine"; // 窗口标题
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