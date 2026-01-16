
// ======================================================================
#include "stdafx.h"
#include "Scene/DemoScene.h"
#include "Core/GameEngine.h"
#include "Graphics/Camera/Camera.h"
#include "Resources/ResourceManager.h"
#include "Entities/ModelEntity.h"
#include "Entities/SkyboxEntity.h"
// ======================================================================
#include "Resources/Texture.h"
#include "Resources/Model.h"
// ======================================================================

// TODO: 测试
namespace
{
    // 测试贴图加载
    std::shared_ptr<CTexture> g_pTexture = nullptr;
    BOOL g_bTextureLoaded = FALSE;
}

void DrawTexturedCube()
{
    glDisable(GL_LIGHTING);            // 暂时关掉光照，排除光照干扰
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // 强制设为纯白，确保纹理 1:1 输出

    static bool bAttempted = false;

    if (!bAttempted)
    {
        bAttempted = true; // 无论成功失败，只试一次
        g_pTexture = std::make_shared<CTexture>();
        if (!g_pTexture->LoadFromFile(L"assets/Textures/test-texture.png"))
        {
            // 如果加载失败，使用资源管理器里的兜底纹理
            g_pTexture = CGameEngine::GetInstance().GetResourceManager()->GetDefaultTexture();
        }
    }

    glPushMatrix();
    glTranslatef(2.0f, 0.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    g_pTexture->Bind(GL_TEXTURE0);

    // 设置纹理环境
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // 绘制立方体（固定管线方式）
    glBegin(GL_QUADS);
    {
        // 前面
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);

        // 后面
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);

        // 顶面
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);

        // 底面
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);

        // 右面
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(1.0f, -1.0f, 1.0f);

        // 左面
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-1.0f, -1.0f, 1.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, 1.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-1.0f, 1.0f, -1.0f);
    }
    glEnd();

    g_pTexture->Unbind(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void DrawColorCube()
{
    glPushMatrix();
    {
        // 将立方体稍微抬高一点，放在坐标原点上方
        glTranslatef(-2.0f, 0.0f, 0.0f);

        // 让立方体自己旋转，方便观察 3D 效果
        // static float rotation = 0.0f;
        // rotation += 0.5f;
        // glRotatef(rotation, 0.0f, 1.0f, 0.0f);
        glBegin(GL_QUADS);
        {
            // 前面 (Z+)
            glColor3f(1.0f, 0.0f, 0.0f); // 红色
            glVertex3f(-1.0f, -1.0f, 1.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);

            // 后面 (Z-)
            glColor3f(0.0f, 1.0f, 0.0f); // 绿色
            glVertex3f(-1.0f, -1.0f, -1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glVertex3f(1.0f, -1.0f, -1.0f);

            // 顶面 (Y+)
            glColor3f(0.0f, 0.0f, 1.0f); // 蓝色
            glVertex3f(-1.0f, 1.0f, -1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);

            // 底面 (Y-)
            glColor3f(1.0f, 1.0f, 0.0f); // 黄色
            glVertex3f(-1.0f, -1.0f, -1.0f);
            glVertex3f(1.0f, -1.0f, -1.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f);

            // 右面 (X+)
            glColor3f(1.0f, 0.0f, 1.0f); // 紫色
            glVertex3f(1.0f, -1.0f, -1.0f);
            glVertex3f(1.0f, 1.0f, -1.0f);
            glVertex3f(1.0f, 1.0f, 1.0f);
            glVertex3f(1.0f, -1.0f, 1.0f);

            // 左面 (X-)
            glColor3f(0.0f, 1.0f, 1.0f); // 青色
            glVertex3f(-1.0f, -1.0f, -1.0f);
            glVertex3f(-1.0f, -1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, -1.0f);
        }
        glEnd();
    }
    glPopMatrix();
}

BOOL CDemoScene::Initialize()
{
    // 1. 创建根实体 (使用之前设计的工厂方法)
    m_pRootEntity = CEntity::Create();
    m_pRootEntity->SetName(L"SceneRoot");

    // 2. 添加天空盒
    GLuint skyboxTexture = LoadSkybox(); // 需要实现这个函数
    if (skyboxTexture != 0)
    {
        auto pSkybox = CSkyboxEntity::Create(skyboxTexture);
        pSkybox->SetSize(500.0f);        // 设置天空盒大小
        pSkybox->EnableRotation(TRUE);   // 启用旋转
        pSkybox->SetRotationSpeed(2.0f); // 设置旋转速度

        m_pRootEntity->AddChild(pSkybox);
        LogInfo(L"天空盒创建成功\n");
    }
    else
    {
        LogWarning(L"天空盒纹理加载失败\n");
    }

    // 3. 添加地形
    // auto terrain = CModelEntity::Create(pTerrainModel);
    // m_pRootEntity->AddChild(terrain);

    // 2. 加载鸭子模型资源
    auto resMgr = CGameEngine::GetInstance().GetResourceManager();
    auto pDuckModel = resMgr->GetModel(L"Duck/glTF/Duck.gltf");

    if (pDuckModel)
    {
        auto pDuckEntity = CModelEntity::Create(pDuckModel);
        pDuckEntity->SetName(L"MainDuck");
        pDuckEntity->SetPosition(Vector3(0.0f, 0.0f, 0.0f)); // 确保在视野内
        pDuckEntity->SetScale(Vector3(0.01f, 0.01f, 0.01f));

        // 调试：输出模型信息
        LogInfo(L"鸭子模型位置: (0, 1, 0), 缩放: 0.01.\n");

        m_pRootEntity->AddChild(pDuckEntity);
    }

    m_bInitialized = TRUE;
    return TRUE;
}

void CDemoScene::Update(float deltaTime)
{
    // 驱动层级系统更新（计算矩阵等）
    if (m_pRootEntity && !m_bIsPaused)
    {
        auto children = m_pRootEntity->GetChildren(); // 你需要公开这个接口或存下指针
        if (!children.empty())
        {
            static float rotationY = 0.0f;
            rotationY += deltaTime * 10.0f;
            children[0]->SetRotation(Vector3(0, rotationY, 0));
        }

        m_pRootEntity->Update(deltaTime);
    }
}

void CDemoScene::Render()
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    CCamera *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (pCamera)
    {
        pCamera->ApplyViewMatrix(); // 这里内部通常执行 glLoadMatrix 或 gluLookAt
    }

    DrawGrid();         // 场景底部的网格
    DrawColorCube();    // 测试渲染
    DrawTexturedCube(); // 测试贴图

    // 驱动层级系统渲染
    if (m_pRootEntity)
    {
        m_pRootEntity->Render();
    }
}

GLuint CDemoScene::LoadSkybox()
{
    std::wstring skyboxName = L"day";

    auto pResMgr = CGameEngine::GetInstance().GetResourceManager();
    if (pResMgr)
    {
        return pResMgr->LoadSkybox(skyboxName);
    }
}

void CDemoScene::DrawGrid()
{
    glPushMatrix();

    // ========================================================
    // 1. 绘制参考地平面 (Grid)
    // ========================================================

    glDisable(GL_LIGHTING); // 调试阶段关闭光照，确保颜色准确

    float size = 100.0f; // 地面大小
    float step = 1.0f;   // 网格间距

    glBegin(GL_LINES);
    for (float i = -size; i <= size; i += step)
    {
        // 颜色：深灰色，中心轴可以用深白色区分
        if (i == 0)
            glColor3f(0.8f, 0.8f, 0.8f);
        else
            glColor3f(0.4f, 0.4f, 0.4f);

        // 绘制横线 (平行于 Z 轴)
        glVertex3f(i, 0.0f, -size);
        glVertex3f(i, 0.0f, size);

        // 绘制纵线 (平行于 X 轴)
        glVertex3f(-size, 0.0f, i);
        glVertex3f(size, 0.0f, i);
    }
    glEnd();

    // 绘制世界坐标轴 (X-红, Y-绿, Z-蓝)
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0, 0.1f, 0);
    glVertex3f(5, 0.1f, 0); // X轴
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0, 0.1f, 0);
    glVertex3f(0, 5.1f, 0); // Y轴
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0, 0.1f, 0);
    glVertex3f(0, 0.1f, 5); // Z轴
    glEnd();
    glLineWidth(1.0f);

    glPopMatrix();
}

void CDemoScene::Shutdown()
{
    if (m_pRootEntity)
    {
        // 递归清理实体持有的资源或断开连接
        m_pRootEntity = nullptr;
    }
}