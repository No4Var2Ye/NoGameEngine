
// ======================================================================
#include "stdafx.h"
#include "Entities/SkyboxEntity.h"
#include "Core/GameEngine.h"
// ======================================================================

unsigned int CSkyboxEntity::s_nextID = 0;

CSkyboxEntity::CSkyboxEntity(GLuint textureID)
    : m_uCubemapID(textureID),
      m_fSize(100.0f),          // 默认大小
      m_bEnableRotation(FALSE), // 默认不旋转
      m_fRotationSpeed(2.0f),   // 默认旋转速度
      m_fCurrentRotation(0.0f)  // 初始旋转角度
{
    SetName(L"Skybox_Day");
}

void CSkyboxEntity::Update(float deltaTime)
{
    // 调用基类更新
    CEntity::Update(deltaTime);

    // 更新天空盒旋转
    if (m_bEnableRotation)
    {
        m_fCurrentRotation += m_fRotationSpeed * deltaTime;
        if (m_fCurrentRotation >= 360.0f)
        {
            m_fCurrentRotation -= 360.0f;
        }
    }
}

void CSkyboxEntity::Render()
{
    if (m_uCubemapID == 0 || !IsVisible())
        return;

    // 保存当前 OpenGL 状态
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);

    // 1. 状态设置
    glDisable(GL_DEPTH_TEST); // 禁用深度测试
    glDepthMask(GL_FALSE);    // 禁用深度写入
    glDisable(GL_LIGHTING);   // 禁用光照
    glDisable(GL_FOG);        // 禁用雾化
    glDisable(GL_CULL_FACE);  // 禁用背面剔除

    // 启用纹理
    glEnable(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_uCubemapID);

    // 设置纹理参数
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glPushMatrix();

    // 2. 核心逻辑：跟随相机
    auto pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (pCamera)
    {
        Vector3 camPos = pCamera->GetPosition();
        // 将天空盒平移到相机位置
        glTranslatef(camPos.x, camPos.y, camPos.z);
    }

    // 3. 应用旋转
    if (m_bEnableRotation)
    {
        glRotatef(m_fCurrentRotation, 0.0f, 1.0f, 0.0f); // 绕Y轴旋转
    }

    // 4. 应用缩放
    glScalef(m_fSize, m_fSize, m_fSize);

    // 5. 渲染立方体
    DrawCube();

    glPopMatrix();

    // 6. 恢复 OpenGL 状态
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glDisable(GL_TEXTURE_CUBE_MAP);

    glPopAttrib();
}

void CSkyboxEntity::DrawCube()
{
    GLfloat half = 0.5f; // 1x1x1 立方体的一半尺寸

    // 设置颜色为白色，确保纹理正常显示
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    {
        // 前面 (+Z)
        glTexCoord3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(-half, -half, half);
        glTexCoord3f(1.0f, -1.0f, 1.0f);
        glVertex3f(half, -half, half);
        glTexCoord3f(1.0f, 1.0f, 1.0f);
        glVertex3f(half, half, half);
        glTexCoord3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-half, half, half);

        // 后面 (-Z)
        glTexCoord3f(1.0f, -1.0f, -1.0f);
        glVertex3f(-half, -half, -half);
        glTexCoord3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(half, -half, -half);
        glTexCoord3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(half, half, -half);
        glTexCoord3f(1.0f, 1.0f, -1.0f);
        glVertex3f(-half, half, -half);

        // 上面 (+Y)
        glTexCoord3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(-half, half, -half);
        glTexCoord3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-half, half, half);
        glTexCoord3f(1.0f, 1.0f, 1.0f);
        glVertex3f(half, half, half);
        glTexCoord3f(1.0f, 1.0f, -1.0f);
        glVertex3f(half, half, -half);

        // 下面 (-Y)
        glTexCoord3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-half, -half, -half);
        glTexCoord3f(1.0f, -1.0f, -1.0f);
        glVertex3f(half, -half, -half);
        glTexCoord3f(1.0f, -1.0f, 1.0f);
        glVertex3f(half, -half, half);
        glTexCoord3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(-half, -half, half);

        // 右面 (+X)
        glTexCoord3f(1.0f, -1.0f, -1.0f);
        glVertex3f(half, -half, -half);
        glTexCoord3f(1.0f, 1.0f, -1.0f);
        glVertex3f(half, half, -half);
        glTexCoord3f(1.0f, 1.0f, 1.0f);
        glVertex3f(half, half, half);
        glTexCoord3f(1.0f, -1.0f, 1.0f);
        glVertex3f(half, -half, half);

        // 左面 (-X)
        glTexCoord3f(-1.0f, -1.0f, -1.0f);
        glVertex3f(-half, -half, -half);
        glTexCoord3f(-1.0f, -1.0f, 1.0f);
        glVertex3f(-half, -half, half);
        glTexCoord3f(-1.0f, 1.0f, 1.0f);
        glVertex3f(-half, half, half);
        glTexCoord3f(-1.0f, 1.0f, -1.0f);
        glVertex3f(-half, half, -half);
    }
    glEnd();
}