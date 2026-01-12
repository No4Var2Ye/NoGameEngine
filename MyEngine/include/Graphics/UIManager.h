
// ======================================================================
#ifndef __UI_MANAGER_H__
#define __UI_MANAGER_H__

// ======================================================================
#include <windows.h>
#include <GL/gl.h>
// ======================================================================

class CRenderer;

class CUIManager
{
public:
    CUIManager(CRenderer* pRenderer);
    ~CUIManager();

    // 1. 状态切换：进入/退出 2D 模式
    void Begin2D();
    void End2D();

    // 2. 基础 2D 绘图接口
    void DrawRect(float x, float y, float width, float height, float r, float g, float b, float a);
    void DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a);
    void DrawFrame(float x, float y, float width, float height, float thickness, float r, float g, float b, float a);

private:
    CRenderer* m_pRenderer;
};

#endif // __UI_MANAGER_H__