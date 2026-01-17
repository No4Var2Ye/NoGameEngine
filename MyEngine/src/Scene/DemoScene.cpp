
// ======================================================================
#include "stdafx.h"
#include "Scene/DemoScene.h"
#include "Core/GameEngine.h"
#include "Graphics/Camera/Camera.h"
#include "Resources/ResourceManager.h"
#include "Entities/ModelEntity.h"
#include "Entities/SkyboxEntity.h"
#include "Entities/GridEntity.h"
#include "Entities/TerrainEntity.h"
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
    glTranslatef(2.0f, 1.0f, 0.0f);

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
        glTranslatef(-2.0f, 1.0f, 0.0f);

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
    // ======================================================================
    // 0. 获取资源管理器实例
    auto resMgr = CGameEngine::GetInstance().GetResourceManager();

    // ======================================================================
    // 1. 创建根实体
    m_pRootEntity = CEntity::Create();
    m_pRootEntity->SetName(L"SceneRoot");
    
    // ======================================================================
    // 2. 添加天空盒
    GLuint skyboxTexture = LoadSkybox();
    if (skyboxTexture != 0)
    {
        auto pSkybox = CSkyboxEntity::Create(skyboxTexture);
        pSkybox->SetName(L"WorldSkybox");
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

    // ======================================================================
    // 3. 添加地形
    auto pTerrain = CTerrainEntity::Create(L"assets/Textures/Terrain/terrain_heightmap4.png",
                                           L"assets/Textures/Terrain/grass.jpg",
                                           300.0f, 15.0f);
    if (pTerrain)
    {
        pTerrain->SetName(L"WorldTerrain");
        pTerrain->SetColor(Vector4(0.6f, 0.8f, 0.9f, 1.0f));

        pTerrain->SetNormalScale(1.0f);
        pTerrain->SetNormalStep(100);
        pTerrain->SetDrawNormals(TRUE);

        pTerrain->SetPosition(Vector3(0, 0, 0));

        m_pRootEntity->AddChild(pTerrain);
    }
    else
    {
        LogError(L"地形创建失败.\n");
    }

    // ======================================================================
    // 4. 创建网格实体
    auto pGrid = CGridEntity::Create(100.0f, 1.0f);
    if (pGrid)
    {
        pGrid->SetPosition(Vector3(0, -0.01f, 0));
        m_pRootEntity->AddChild(pGrid);
        LogInfo(L"网格创建成功\n");
    }

    // ======================================================================
    // 5. 加载鸭子模型资源
    auto pDuckModel = resMgr->GetModel(L"Duck/glTF/Duck.gltf");
    if (pDuckModel)
    {
        auto pDuckEntity = CModelEntity::Create(pDuckModel);
        pDuckEntity->SetName(L"MainDuck");
        pDuckEntity->SetPosition(Vector3(0.0f, 0.0f, 0.0f)); // 确保在视野内
        pDuckEntity->SetScale(Vector3(0.01f, 0.01f, 0.01f));

        // 调试：输出模型信息
        // LogInfo(L"鸭子模型位置: (0, 1, 0), 缩放: 0.01.\n");

        pDuckEntity->SetDrawBoundingBox(TRUE);
        pDuckEntity->SetNormalScale(10.0f);
        pDuckEntity->SetNormalStep(10);
        pDuckEntity->SetDrawNormals(TRUE);

        m_pRootEntity->AddChild(pDuckEntity);
    }

    // 启用雾化
    SetupFog();

    m_bInitialized = TRUE;
    return TRUE;
}

void CDemoScene::Update(float deltaTime)
{
    // 驱动层级系统更新（计算矩阵等）
    if (m_pRootEntity && !m_bIsPaused)
    {
        m_pRootEntity->Update(deltaTime);

        static std::weak_ptr<CTerrainEntity> pTerrainCache;
        auto pTerrain = pTerrainCache.lock();

        if (!pTerrain)
        {
            // 通过名字查找实体
            auto pEntity = m_pRootEntity->FindChildByName(L"WorldTerrain");
            // 将基类指针安全转换为地形类指针
            pTerrain = std::dynamic_pointer_cast<CTerrainEntity>(pEntity);
            pTerrainCache = pTerrain;
        }

        static std::weak_ptr<CModelEntity> pDuckCache;
        auto pDuck = pDuckCache.lock();

        if (!pDuck)
        {
            // 通过名字查找实体
            auto pEntity = m_pRootEntity->FindChildByName(L"MainDuck");
            // 将基类指针安全转换为地形类指针
            pDuck = std::dynamic_pointer_cast<CModelEntity>(pEntity);
            pDuckCache = pDuck;
        }

        if (pTerrain && pDuck)
        {
            Vector3 duckPos = pDuck->GetPosition();

            // 1. 根据当前 XZ 坐标获取高度
            float groundHeight = pTerrain->GetHeightAt(duckPos.x, duckPos.z);

            // 2. 将鸭子设置到该高度
            // 注意：如果鸭子的原点在中心而非脚底，你可能需要加上偏移：groundHeight + offset
            duckPos.y = groundHeight;
            pDuck->SetPosition(duckPos);
        }
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

    // 在渲染前清理纹理状态
    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    CleanupTextureState();

    SetupGlobalLighting();

    // 驱动层级系统渲染
    if (m_pRootEntity)
    {
        // 建议在 CSkyboxEntity::Render 内部手动关闭和开启雾
        m_pRootEntity->Render();
    }

    CleanupTextureState();

    // DrawColorCube();    // 测试渲染
    // DrawTexturedCube(); // 测试贴图
}

GLuint CDemoScene::LoadSkybox()
{
    std::wstring skyboxName = L"day";

    auto pResMgr = CGameEngine::GetInstance().GetResourceManager();
    if (pResMgr)
    {
        return pResMgr->LoadSkybox(skyboxName);
    }

    LogWarning(L"ResourceManager 不可用，无法加载天空盒\n");
    return 0;
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

void CDemoScene::SetupFog()
{
    glEnable(GL_FOG); // 1. 开启雾化

    // 2. 设置雾的颜色。建议与天空盒底部的颜色（或者背景清除色）完全一致
    // 这样物体在远方会逐渐消失在背景中，产生无限深度的错觉
    GLfloat fogColor[4] = {0.5f, 0.6f, 0.7f, 1.0f}; // 这是一个偏浅蓝的天空色
    glFogfv(GL_FOG_COLOR, fogColor);

    // 3. 设置雾的模式
    // GL_LINEAR: 线性雾，需要设置开始和结束距离
    // GL_EXP 或 GL_EXP2: 指数雾，更真实，只需设置密度 (Density)
    glFogi(GL_FOG_MODE, GL_EXP2);

    // 4. 设置密度。这个值通常很小，建议在 0.001 到 0.05 之间尝试
    glFogf(GL_FOG_DENSITY, 0.01f);

    // 5. 设置计算质量（可选）
    glHint(GL_FOG_HINT, GL_NICEST); // 基于像素计算，效果最好
}

void CDemoScene::Shutdown()
{
    if (m_pRootEntity)
    {
        // 递归清理实体持有的资源或断开连接
        m_pRootEntity = nullptr;
    }
}

void CDemoScene::SetupGlobalLighting()
{
    // 全局光照设置
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // 光源位置（跟随相机）
    Vector3 camPos = CGameEngine::GetInstance().GetMainCamera()->GetPosition();
    GLfloat lightPosition[] = {camPos.x, camPos.y + 10.0f, camPos.z, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    // 光源属性
    GLfloat lightAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // 全局材质
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

void CDemoScene::CleanupTextureState()
{
    // 清理所有纹理单元
    GLint maxTextureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);

    for (int i = 0; i < maxTextureUnits; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

    // 激活默认纹理单元
    glActiveTexture(GL_TEXTURE0);

    // 检查清理结果
    GLint currentTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
    if (currentTexture != 0)
    {
        LogWarning(L"纹理清理失败，当前绑定纹理: %d", currentTexture);
    }
}