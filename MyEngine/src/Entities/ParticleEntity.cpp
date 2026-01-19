
#include "stdafx.h"

#include "Entities/ParticleEntity.h"

CParticleEntity::CParticleEntity(int maxParticles)
{
    m_name = L"RainSystem";
    m_particles.resize(maxParticles);
    for (auto &p : m_particles)
    {
        // 随机分布在初始空间内
        p.position = Vector3(
            (float)(rand() % 200 - 100) * 0.1f * m_Range,
            (float)(rand() % 100) * 0.1f * m_Height,
            (float)(rand() % 200 - 100) * 0.1f * m_Range);
        p.velocity = Vector3(0, -m_FallSpeed - (rand() % 10), 0);
        p.length = 0.8f + (rand() % 10) * 0.1f;
    }
}

void CParticleEntity::Update(float deltaTime)
{
    // 获取相机位置，让雨始终在玩家周围下
    // 注意：这里可以从单例 Engine 获取 MainCamera
    // Vector3 camPos = CGameEngine::GetInstance().GetMainCamera()->GetPosition();

    for (auto &p : m_particles)
    {
        p.position = p.position + p.velocity * deltaTime;

        // 如果雨滴掉到地面以下（假设地面在 Y=0 附近或根据地形高度）
        // 这里简单处理：掉落超过一定高度就重置到顶部
        if (p.position.y < -5.0f)
        {
            p.position.y = m_Height;
            // 可以在相机水平位置随机偏移，产生“雨随人走”的效果
        }
    }
}

void CParticleEntity::Render()
{
    glDisable(GL_LIGHTING); // 雨滴不需要受光
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLineWidth(2.5f);

    glColor4f(0.7f, 0.7f, 1.0f, 0.6f); // 浅蓝色半透明
    glBegin(GL_LINES);
    for (const auto &p : m_particles)
    {
        glVertex3f(p.position.x, p.position.y, p.position.z);
        glVertex3f(p.position.x, p.position.y - p.length, p.position.z);
    }
    glEnd();

    glLineWidth(1.0f);

    glEnable(GL_LIGHTING);
    glDisable(GL_BLEND);
}