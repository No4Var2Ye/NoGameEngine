#include "stdafx.h"
#include "Graphics/UIManager.h"
#include "Core/Renderer.h"

CUIManager::CUIManager(CRenderer* pRenderer) : m_pRenderer(pRenderer) {}

CUIManager::~CUIManager() {}

void CUIManager::Begin2D()
{
    // 1. 保存当前的 3D 渲染属性和矩阵
    m_pRenderer->PushState();

    // 2. 切换到正交投影模式
    // 我们将坐标系设置为：左上角 (0,0)，右下角 (Width, Height)
    // 这样 UI 坐标就和像素一一对应了
    m_pRenderer->SetOrthoProjection(
        0.0f, 
        static_cast<float>(m_pRenderer->GetWidth()), 
        static_cast<float>(m_pRenderer->GetHeight()), 
        0.0f, 
        -1.0f, 1.0f
    );

    // 3. 准备 2D 渲染状态
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);   // 2. 禁用深度测试，确保 UI 在最前面
    glDisable(GL_LIGHTING);     // 3. 关闭光照，UI 使用纯色
    glEnable(GL_BLEND);         // 4. 开启混合以支持透明度
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void CUIManager::End2D()
{
    // 恢复之前 PushState 保存的所有属性和矩阵
    m_pRenderer->PopState();
}

void CUIManager::DrawRect(float x, float y, float w, float h, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
        glVertex2f(x,     y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x,     y + h);
    glEnd();
}

void CUIManager::DrawLine(float x1, float y1, float x2, float y2, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    glEnd();
}

void CUIManager::DrawFrame(float x, float y, float w, float h, float thickness, float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glLineWidth(thickness);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + w, y);
        glVertex2f(x + w, y + h);
        glVertex2f(x, y + h);
    glEnd();
    glLineWidth(1.0f); // 恢复默认线宽
}