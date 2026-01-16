// ======================================================================
#include "stdafx.h"
#include "Entities/GridEntity.h"
#include "Core/GameEngine.h"
#include "Graphics/Camera/Camera.h"
// ======================================================================

unsigned int CGridEntity::s_nextID = 2000; // 网格实体起始ID段

CGridEntity::CGridEntity(FLOAT size, FLOAT step)
    : m_fSize(size),                         //
      m_fStep(step),                         //
      m_SubColor(Vector3(0.6f, 0.6f, 0.6f)), // 坐标轴颜色
      m_MainColor(Vector3(0.2f, 0.2f, 0.2f)) // 细网格颜色
{
    SetName(L"WorldGrid");
}

void CGridEntity::Update(float deltaTime)
{
    // 网格通常是静态的，直接调用基类更新矩阵计算
    CEntity::Update(deltaTime);
}

void CGridEntity::Render()
{
    if (!m_bVisible)
        return;

    CCamera *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (!pCamera)
        return;

    Vector3 camPos = pCamera->GetPosition();

    // 1. 保护 OpenGL 状态
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);

    glDisable(GL_LIGHTING);   // 网格不参与光照计算
    glDisable(GL_TEXTURE_2D); // 关闭纹理

    // 网格必须受雾化影响，这样远处的线条才会自然隐入雾中
    glEnable(GL_FOG);

    // 开启混合，用于实现淡出效果
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 2. 应用实体变换
    glPushMatrix();
    glMultMatrixf(GetWorldMatrix().m);

    // 3. 绘制逻辑
    glBegin(GL_LINES);
    {
        // --- 第一层：细网格 (Minor Grid) ---
        // 步长为 m_fStep (如 1.0)，淡出距离设为较短，例如 30 单位
        float minorFadeEnd = 30.0f;

        for (float i = -m_fSize; i <= m_fSize; i += m_fStep)
        {
            // 如果是粗网格的位置，跳过不画（由第二层画），避免颜色叠加变亮
            if (fmod(abs(i), m_fStep * 10.0f) < 0.001f)
                continue;

            DrawFadingLine(i, TRUE, camPos, m_SubColor, minorFadeEnd);
            DrawFadingLine(i, FALSE, camPos, m_SubColor, minorFadeEnd);
        }

        // --- 第二层：粗网格 (Major Grid) ---
        // 步长为 m_fStep * 10 (如 10.0)，淡出距离设为较长，如 m_fSize 的 80%
        float majorStep = m_fStep * 10.0f;
        float majorFadeEnd = m_fSize * 0.8f;
        Vector3 majorColor(0.5f, 0.5f, 0.5f); // 粗网格比细网格亮一点

        for (float i = -m_fSize; i <= m_fSize; i += majorStep)
        {
            // 坐标轴使用 MainColor
            Vector3 color = (abs(i) < 0.001f) ? m_MainColor : majorColor;
            DrawFadingLine(i, TRUE, camPos, color, majorFadeEnd);
            DrawFadingLine(i, FALSE, camPos, color, majorFadeEnd);
        }
    }
    glEnd();

    glPopMatrix();
    glPopAttrib();

    // 4. 递归渲染子节点
    CEntity::Render();
}
void CGridEntity::DrawFadingLine(float coord, BOOL isParallelToZ, const Vector3 &camPos, const Vector3 &color, float maxDist)
{
    float fadeStart = maxDist * 0.5f; // 从一半距离开始淡出
    float fadeEnd = maxDist;

    // 我们直接计算线段两个端点的 Alpha，OpenGL 会自动在顶点间做插值
    auto DrawSegmentWithAlpha = [&](float start, float end)
    {
        auto GetAlpha = [&](float pX, float pZ) -> float
        {
            float d = sqrt(pow(pX - camPos.x, 2) + pow(pZ - camPos.z, 2));
            float alpha = 1.0f - (d - fadeStart) / (fadeEnd - fadeStart);
            return (alpha < 0) ? 0.0f : (alpha > 1.0f ? 1.0f : alpha);
        };

        float alphaStart, alphaEnd;
        if (isParallelToZ)
        {
            alphaStart = GetAlpha(coord, start);
            alphaEnd = GetAlpha(coord, end);
            if (alphaStart <= 0 && alphaEnd <= 0)
                return; // 全透明不画

            glColor4f(color.x, color.y, color.z, alphaStart);
            glVertex3f(coord, 0.0f, start);
            glColor4f(color.x, color.y, color.z, alphaEnd);
            glVertex3f(coord, 0.0f, end);
        }
        else
        {
            alphaStart = GetAlpha(start, coord);
            alphaEnd = GetAlpha(end, coord);
            if (alphaStart <= 0 && alphaEnd <= 0)
                return;

            glColor4f(color.x, color.y, color.z, alphaStart);
            glVertex3f(start, 0.0f, coord);
            glColor4f(color.x, color.y, color.z, alphaEnd);
            glVertex3f(end, 0.0f, coord);
        }
    };

    // 考虑到相机可能在中间，将长线段切成几段，增加插值的精度
    float step = (fadeEnd / 2.0f);
    for (float t = -m_fSize; t < m_fSize; t += step)
    {
        DrawSegmentWithAlpha(t, Math::Min(t + step, m_fSize));
    }
}