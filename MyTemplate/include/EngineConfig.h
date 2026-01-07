
#ifndef __ENGINECONFIG_H__
#define __ENGINECONFIG_H__

struct EngineConfig
/**
 * @brief 引擎配置结构
 */
{
    INT windowWidth = 800;                 // 窗口宽度
    INT windowHeight = 600;                 // 窗口高度
    BOOL fullscreen = false;                // 是否全屏
    const wchar_t *title = L"MyGameEngine"; // 窗口标题
    INT colorBits = 32;                     // 颜色位数
    INT depthBits = 24;              // 深度缓冲位数
    INT stencilBits = 8;             // 模板缓冲位数
    BOOL vsync = true;               // 垂直同步
    BOOL showCursor = true;          // 显示光标
    BOOL resizable = true;           // 窗口可调整大小
    BOOL borderless = false;         // 无边框窗口
};

#endif // __ENGINECONFIG_H__