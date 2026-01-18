// ======================================================================
#include "stdafx.h"
#include "Entities/GridEntity.h"
#include "Core/GameEngine.h"
// #include "Graphics/Camera/Camera.h"
#include "Entities/CameraEntity.h"
// ======================================================================

unsigned int CGridEntity::s_nextID = 2000; // 网格实体起始ID段

CGridEntity::CGridEntity(FLOAT size, FLOAT step)
    : m_fSize(size),                          //
      m_fStep(step),                          //
      m_MainColor(Vector3(0.2f, 0.2f, 0.2f)), // 粗网格颜色
      m_SubColor(Vector3(0.6f, 0.6f, 0.6f)),  // 细网格颜色
      m_bShowAxes(TRUE),                      // 世界坐标
      m_bEnableFade(TRUE),                    // 是否启用距离淡出
      m_fMinorFadeDist(30.0f),                // 细网格消失距离
      m_fMajorFadeDist(150.0f)                // 粗网格消失距离

{
    SetName(L"WorldGrid");
    BuildAdaptiveGeometry(); // 预计算顶点
}

void CGridEntity::InitGrid()
{
    m_gridVertices.clear();
    // 预先循环计算所有线条坐标
    for (float i = -m_fSize; i <= m_fSize; i += m_fStep)
    {
        // X方向线段的两个端点
        m_gridVertices.push_back({-m_fSize, 0, i, 255, 255, 255, 255});
        m_gridVertices.push_back({m_fSize, 0, i, 255, 255, 255, 255});
        // Z方向线段的两个端点
        m_gridVertices.push_back({i, 0, -m_fSize, 255, 255, 255, 255});
        m_gridVertices.push_back({i, 0, m_fSize, 255, 255, 255, 255});
    }
}

void CGridEntity::BuildGeometry()
{
    m_MainGridVertices.clear();
    m_SubGridVertices.clear();

    const float majorStep = m_fStep * 10.0f;
    const float segmentStep = 10.0f;

    for (float i = -m_fSize; i <= m_fSize; i += m_fStep)
    {
        // 核心修复 2: 提高浮点数判定鲁棒性
        BOOL isMajor = (fmod(fabs(i) + 0.001f, majorStep) < m_fStep);
        const Vector3 &color = isMajor ? m_MainColor : m_SubColor;
        auto &targetVec = isMajor ? m_MainGridVertices : m_SubGridVertices;

        unsigned char r = static_cast<unsigned char>(color.x * 255);
        unsigned char g = static_cast<unsigned char>(color.y * 255);
        unsigned char b = static_cast<unsigned char>(color.z * 255);
        unsigned char a = 255;

        // X方向线段：分段绘制
        for (float x = -m_fSize; x < m_fSize; x += segmentStep)
        {
            float nextX = Math::Min(x + segmentStep, m_fSize);
            targetVec.push_back({x, 0.0f, i, r, g, b, a});
            targetVec.push_back({nextX, 0.0f, i, r, g, b, a});
        }

        // Z方向线段：分段绘制
        for (float z = -m_fSize; z < m_fSize; z += segmentStep)
        {
            float nextZ = Math::Min(z + segmentStep, m_fSize);
            targetVec.push_back({i, 0.0f, z, r, g, b, a});
            targetVec.push_back({i, 0.0f, nextZ, r, g, b, a});
        }
    }

    // LogDebug(L"网格几何构建完成: 粗网格顶点=%d, 细网格顶点=%d.\n",
    //          m_MainGridVertices.size(), m_SubGridVertices.size());
}

void CGridEntity::BuildGeometryWithLOD()
{
    m_MainGridVertices.clear();
    m_SubGridVertices.clear();

    const float majorStep = m_fStep * 10.0f;

    // 根据网格大小动态调整细节级别
    int lodLevel = CalculateLODLevel();
    float effectiveStep = m_fStep * lodLevel;

    // 确保步长合理
    effectiveStep = Math::Max(effectiveStep, m_fStep);
    effectiveStep = Math::Min(effectiveStep, m_fStep * 5.0f); // 最大5倍步长

    for (float i = -m_fSize; i <= m_fSize; i += effectiveStep)
    {
        // if (fabs(i) < effectiveStep * 0.5f)
        //     continue;

        bool isMajor = (fmod(fabs(i), majorStep) < effectiveStep * 0.5f);
        const Vector3 &color = isMajor ? m_MainColor : m_SubColor;
        auto &targetVec = isMajor ? m_MainGridVertices : m_SubGridVertices;

        unsigned char r = static_cast<unsigned char>(color.x * 255);
        unsigned char g = static_cast<unsigned char>(color.y * 255);
        unsigned char b = static_cast<unsigned char>(color.z * 255);

        // 只在LOD级别为1时绘制所有网格线，高级别LOD只绘制主要线
        if (lodLevel == 1 || isMajor)
        {
            targetVec.push_back({-m_fSize, 0.0f, i, r, g, b, 255});
            targetVec.push_back({m_fSize, 0.0f, i, r, g, b, 255});
            targetVec.push_back({i, 0.0f, -m_fSize, r, g, b, 255});
            targetVec.push_back({i, 0.0f, m_fSize, r, g, b, 255});
        }
    }
}

int CGridEntity::CalculateLODLevel() const
{
    CCameraEntity *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (!pCamera)
        return 1;

    float distance = (GetPosition() - pCamera->GetPosition()).Length();

    if (distance < 50.0f)
        return 1; // 近距离：全细节
    if (distance < 100.0f)
        return 2; // 中距离：减半细节
    if (distance < 200.0f)
        return 3; // 远距离：1/3细节
    return 5;     // 超远距离：1/5细节
}

BOOL CGridEntity::ShouldRebuildLOD() const
{
    static int s_lastLODLevel = 1;
    int currentLOD = CalculateLODLevel();

    if (currentLOD != s_lastLODLevel)
    {
        s_lastLODLevel = currentLOD;
        return true;
    }
    return false;
}

void CGridEntity::BuildAdaptiveGeometry()
{
    m_MainGridVertices.clear();
    m_SubGridVertices.clear();

    CCameraEntity *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (!pCamera)
        return;

    Vector3 camPos = pCamera->GetPosition();
    float distanceToGrid = fabs(camPos.y - GetPosition().y); // 相机到网格平面的垂直距离

    // 动态调整网格密度：距离越远，网格越稀疏
    float adaptiveStep = CalculateAdaptiveStep(distanceToGrid);
    const float majorStep = adaptiveStep * 10.0f;
    float visibleRange = CalculateVisibleRange(distanceToGrid, pCamera->GetFOV());

    float startX = floor((camPos.x - visibleRange) / adaptiveStep) * adaptiveStep;
    float endX = ceil((camPos.x + visibleRange) / adaptiveStep) * adaptiveStep;
    float startZ = floor((camPos.z - visibleRange) / adaptiveStep) * adaptiveStep;
    float endZ = ceil((camPos.z + visibleRange) / adaptiveStep) * adaptiveStep;

    // 限制在总大小范围内（可选，如果你希望网格无限大则不限）
    startX = Math::Max(startX, -m_fSize);
    endX = Math::Min(endX, m_fSize);
    startZ = Math::Max(startZ, -m_fSize);
    endZ = Math::Min(endZ, m_fSize);

    // 绘制平行于 Z 轴的线 (X 坐标变化)
    for (float x = startX; x <= endX; x += adaptiveStep)
    {
        // 使用精确的 epsilon 检查，确保主网格位置固定
        bool isMajor = (fmod(fabs(x) + 0.001f, majorStep) < adaptiveStep * 0.5f);
        const Vector3 &color = isMajor ? m_MainColor : m_SubColor;
        auto &targetVec = isMajor ? m_MainGridVertices : m_SubGridVertices;

        unsigned char r = static_cast<unsigned char>(color.x * 255);
        unsigned char g = static_cast<unsigned char>(color.y * 255);
        unsigned char b = static_cast<unsigned char>(color.z * 255);

        targetVec.push_back({x, 0.0f, startZ, r, g, b, 255});
        targetVec.push_back({x, 0.0f, endZ, r, g, b, 255});
    }

    // 绘制平行于 X 轴的线 (Z 坐标变化)
    for (float z = startZ; z <= endZ; z += adaptiveStep)
    {
        bool isMajor = (fmod(fabs(z) + 0.001f, majorStep) < adaptiveStep * 0.5f);
        const Vector3 &color = isMajor ? m_MainColor : m_SubColor;
        auto &targetVec = isMajor ? m_MainGridVertices : m_SubGridVertices;

        unsigned char r = static_cast<unsigned char>(color.x * 255);
        unsigned char g = static_cast<unsigned char>(color.y * 255);
        unsigned char b = static_cast<unsigned char>(color.z * 255);

        targetVec.push_back({startX, 0.0f, z, r, g, b, 255});
        targetVec.push_back({endX, 0.0f, z, r, g, b, 255});
    }
}

float CGridEntity::CalculateAdaptiveStep(float distance)
{
    // 距离越远，网格步长越大
    if (distance < 20.0f)
        return 1.0f; // 近距离：细网格
    else if (distance < 50.0f)
        return 2.0f; // 中距离
    else if (distance < 100.0f)
        return 5.0f; // 中远距离
    else if (distance < 200.0f)
        return 10.0f; // 远距离
    else
        return 20.0f; // 超远距离：粗网格
}

float CGridEntity::CalculateVisibleRange(float distance, float fov)
{
    // 根据距离和视野计算可见范围
    float tangent = tan(fov * 0.5f * 3.14159f / 180.0f);
    float visibleRange = distance * tangent * 1.5f; // 1.5倍安全系数

    // 限制最大最小范围
    return Math::Clamp(visibleRange, 50.0f, 500.0f);
}

void CGridEntity::Update(float deltaTime)
{
    // 网格通常是静态的，直接调用基类更新矩阵计算
    CEntity::Update(deltaTime);

    // 静态变量保存上一次的相机位置和网格构建状态
    static Vector3 s_lastCamPos;
    static float s_lastBuildTime = 0.0f;
    static BOOL s_firstUpdate = TRUE;

    CCameraEntity *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (!pCamera)
        return;

    Vector3 camPos = pCamera->GetPosition();

    // 首次更新或相机移动显著时重建网格
    BOOL shouldRebuild = s_firstUpdate;

    if (!shouldRebuild)
    {
        // 检查相机是否移动了足够远的距离
        float moveDistance = (camPos - s_lastCamPos).Length();
        shouldRebuild = (moveDistance > 10.0f); // 移动超过10个单位就重建

        // 或者每隔一段时间重建一次（避免频繁重建）
        s_lastBuildTime += deltaTime;
        if (s_lastBuildTime > 2.0f) // 每2秒强制重建一次
        {
            shouldRebuild = TRUE;
            s_lastBuildTime = 0.0f;
        }
    }

    if (shouldRebuild)
    {
        BuildAdaptiveGeometry();
        s_lastCamPos = camPos;
        s_firstUpdate = FALSE;

        // LogDebug(L"网格动态重建: 相机位置(%.1f, %.1f, %.1f)\n", camPos.x, camPos.y, camPos.z);
    }
}

void CGridEntity::Render()
{
    if (!m_bVisible)
        return;

    // 检查网格数据是否有效
    if (m_MainGridVertices.empty() && m_SubGridVertices.empty())
    {
        // 如果网格数据为空，尝试重建
        BuildAdaptiveGeometry();
        if (m_MainGridVertices.empty() && m_SubGridVertices.empty())
            return; // 重建失败则不渲染
    }

    // 1. 距离裁剪 - 如果网格太远就不渲染
    CCameraEntity *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (pCamera)
    {
        Vector3 camPos = pCamera->GetPosition();
        float distance = (GetPosition() - camPos).Length();

        // 如果相机距离超过粗网格淡出距离的2倍，完全跳过渲染
        if (distance > m_fMajorFadeDist * 2.0f)
            return;
    }

    // 2. 保存OpenGL状态
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_FOG_BIT);

    // 3. 设置渲染状态
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    // 4. 使用雾效实现距离淡出
    if (m_bEnableFade)
    {
        SetupDistanceFog();
    }
    else
    {
        glDisable(GL_FOG);
    }

    // 5. 应用变换
    glPushMatrix();
    glMultMatrixf(GetWorldMatrix().m);

    // 6. 使用顶点数组渲染（高性能）
    RenderGridWithVertexArrays();

    // 7. 绘制坐标轴
    if (m_bShowAxes)
    {
        RenderCoordinateAxes();
    }

    // 8. 恢复状态
    glPopMatrix();
    glPopAttrib();

    // 9. 渲染子实体
    CEntity::Render();
}

void CGridEntity::SetupDistanceFog()
{
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);

    // 设置雾效参数 - 使用线性淡出
    glFogf(GL_FOG_START, m_fMinorFadeDist * 0.3f); // 开始淡出的距离
    glFogf(GL_FOG_END, m_fMajorFadeDist);          // 完全消失的距离

    // 雾颜色设置为背景色（黑色）
    GLfloat fogColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    glFogfv(GL_FOG_COLOR, fogColor);

    glHint(GL_FOG_HINT, GL_FASTEST); // 为了性能，使用最快计算
}

void CGridEntity::RenderGridWithVertexArrays()
{
    if (m_SubGridVertices.empty() && m_MainGridVertices.empty())
        return;

    // 启用顶点数组
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // 渲染细网格
    if (!m_SubGridVertices.empty())
    {
        // 设置细网格的雾效范围
        if (m_bEnableFade)
        {
            glFogf(GL_FOG_START, m_fMinorFadeDist * 0.3f);
            glFogf(GL_FOG_END, m_fMinorFadeDist);
        }

        glVertexPointer(3, GL_FLOAT, sizeof(GridVertex), &m_SubGridVertices[0].x);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(GridVertex), &m_SubGridVertices[0].r);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_SubGridVertices.size()));
    }

    // 渲染粗网格
    if (!m_MainGridVertices.empty())
    {
        // 设置粗网格的雾效范围（看得更远）
        if (m_bEnableFade)
        {
            glFogf(GL_FOG_START, m_fMajorFadeDist * 0.3f);
            glFogf(GL_FOG_END, m_fMajorFadeDist);
        }

        glVertexPointer(3, GL_FLOAT, sizeof(GridVertex), &m_MainGridVertices[0].x);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(GridVertex), &m_MainGridVertices[0].r);
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_MainGridVertices.size()));
    }

    // 禁用顶点数组
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void CGridEntity::RenderCoordinateAxes()
{
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    {
        float bias = 0.01f;

        // X轴 - 红色
        glColor3f(m_AxisColorX.x, m_AxisColorX.y, m_AxisColorX.z);
        glVertex3f(-m_fSize, bias, 0.0f);
        glVertex3f(m_fSize, bias, 0.0f);

        // Z轴 - 蓝色
        glColor3f(m_AxisColorZ.x, m_AxisColorZ.y, m_AxisColorZ.z);
        glVertex3f(0.0f, bias, -m_fSize);
        glVertex3f(0.0f, bias, m_fSize);

        // Y轴 - 绿色（垂直向上，短一些）
        glColor3f(m_AxisColorY.x, m_AxisColorY.y, m_AxisColorY.z);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, m_fSize * 0.1f, 0.0f); // Y轴短一些
    }
    glEnd();

    glLineWidth(1.0f);
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
            if (!m_bEnableFade)
                return 1.0f;

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

    // 动态调整步长：距离越远，线段分段越少以节省性能
    float renderStep = m_bEnableFade ? (maxDist * 0.5f) : m_fSize * 2.0f;
    for (float t = -m_fSize; t < m_fSize; t += renderStep)
    {
        DrawSegmentWithAlpha(t, Math::Min(t + renderStep, m_fSize));
    }
}