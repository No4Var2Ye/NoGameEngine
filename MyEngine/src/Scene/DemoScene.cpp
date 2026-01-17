
// ======================================================================
#include "stdafx.h"
#include <functional>
#include "EngineConfig.h"
#include "Scene/DemoScene.h"
#include "Core/GameEngine.h"
#include "Core/InputManager.h"
#include "Core/Entity.h"
#include "Graphics/Camera/Camera.h"
#include "Resources/ResourceManager.h"
#include "Entities/ModelEntity.h"
#include "Entities/SkyboxEntity.h"
#include "Entities/GridEntity.h"
#include "Entities/TerrainEntity.h"
// ======================================================================
// 测试
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

CDemoScene::CDemoScene()
    : CScene("DemoScene"),       //
      m_PossessedEntityYaw(0.0f) //
{
}

// ======================================================================
// 1. 初始化阶段
// ======================================================================
BOOL CDemoScene::Initialize()
{

    // ======================================================================
    // 0. 获取资源管理器实例
    auto resMgr = CGameEngine::GetInstance().GetResourceManager();

    // ======================================================================
    // 1. 创建根实体
    m_pRootEntity = CEntity::Create();
    m_pRootEntity->SetName(L"DemoSceneRoot");

    // ======================================================================
    // 2. 添加天空盒
    GLuint skyboxTexture = LoadSkybox();
    if (skyboxTexture != 0)
    {
        m_pSkybox = CSkyboxEntity::Create(skyboxTexture);
        m_pSkybox->SetName(L"WorldSkybox");
        m_pSkybox->SetSize(500.0f);        // 设置天空盒大小
        m_pSkybox->EnableRotation(TRUE);   // 启用旋转
        m_pSkybox->SetRotationSpeed(2.0f); // 设置旋转速度

        m_pRootEntity->AddChild(m_pSkybox);
        LogInfo(L"天空盒创建成功\n");
    }

    // ======================================================================
    // 3. 添加地形
    m_pTerrain = CTerrainEntity::Create(L"Terrain/terrain_heightmap4.png",
                                        L"Terrain/grass.jpg",
                                        300.0f, 15.0f);
    if (m_pTerrain)
    {
        m_pTerrain->SetName(L"WorldTerrain");
        m_pTerrain->SetColor(Vector4(0.6f, 0.8f, 0.9f, 1.0f));

        m_pTerrain->SetNormalScale(1.0f);
        m_pTerrain->SetNormalStep(100);
        m_pTerrain->SetDrawNormals(TRUE);

        m_pTerrain->SetPosition(Vector3(0, 0, 0));

        m_pRootEntity->AddChild(m_pTerrain);
        LogInfo(L"地形创建成功\n");
    }

    // ======================================================================
    // 4. 创建网格坐标实体
    m_pGrid = CGridEntity::Create(500.0f, 1.0f);
    if (m_pGrid)
    {
        m_pGrid->SetPosition(Vector3(0, -0.01f, 0));
        m_pGrid->SetFadeDist(50.0f, 200.0f);
        m_pGrid->SetShowAxes(TRUE);

        m_pRootEntity->AddChild(m_pGrid);
        LogInfo(L"网格创建成功\n");
    }

    // ======================================================================
    // 5. 加载鸭子模型资源
    auto pDuckModel = resMgr->GetModel(L"Duck/glTF/Duck.gltf");
    if (pDuckModel)
    {
        auto pDuckEntity = CModelEntity::Create(pDuckModel);
        pDuckEntity->SetName(L"MainDuck");
        pDuckEntity->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        pDuckEntity->SetScale(Vector3(0.01f, 0.01f, 0.01f));
        // pDuckEntity->SetRotation(Vector3(0.0f, -90.0f, 0.0f));

        // 调试：输出模型信息
        // LogInfo(L"鸭子模型位置: (0, 1, 0), 缩放: 0.01.\n");

        pDuckEntity->SetDrawBoundingBox(TRUE);
        pDuckEntity->SetNormalScale(10.0f);
        pDuckEntity->SetNormalStep(10);
        pDuckEntity->SetDrawNormals(TRUE);

        pDuckEntity->SetSnapToTerrain(TRUE, 0.0f);

        m_pRootEntity->AddChild(pDuckEntity);

        RegisterEntityForSnapping(pDuckEntity, TRUE);
        m_pPossessedEntity = pDuckEntity;
    }

    // ======================================================================
    // 场景配置
    SetupFog(); // 启用雾化

    m_bInitialized = TRUE;
    return TRUE;
}

// ======================================================================
// 2. 更新阶段
// ======================================================================
void CDemoScene::Update(float deltaTime)
{
    if (!m_pRootEntity || m_bIsPaused)
        return;

    // 1. 处理输入
    ProcessInput(deltaTime);

    // 2. 驱动层更新
    m_pRootEntity->Update(deltaTime);

    // 3.实体更新逻辑
    UpdateLogic(deltaTime);
}

// ======================================================================
// 3. 渲染阶段
// ======================================================================
void CDemoScene::Render()
{
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    static CCamera *s_pLastCamera = nullptr;
    CCamera *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (pCamera != s_pLastCamera)
    {
        s_pLastCamera = pCamera;
        if (pCamera)
        {
            pCamera->ApplyViewMatrix();
        }
    }

    // 3. 优化光照设置 - 只在变化时更新
    static Vector3 s_lastCamPos;
    Vector3 camPos = pCamera ? pCamera->GetPosition() : Vector3::Zero();
    if ((camPos - s_lastCamPos).LengthSquared() > 1.0f) // 相机移动超过1单位才更新
    {
        SetupGlobalLighting();
        s_lastCamPos = camPos;
    }

    // 驱动层级系统渲染
    if (m_pRootEntity)
    {
        // 建议在 CSkyboxEntity::Render 内部手动关闭和开启雾
        m_pRootEntity->Render();
    }

    // DrawColorCube();    // 测试渲染
    // DrawTexturedCube(); // 测试贴图
}

void CDemoScene::Shutdown()
{
    if (m_pRootEntity)
    {
        // 递归清理实体持有的资源或断开连接
        m_pRootEntity = nullptr;
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

    LogWarning(L"ResourceManager 不可用，无法加载天空盒\n");
    return 0;
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

void CDemoScene::SetupGlobalLighting()
{
    static BOOL s_bInitialized = FALSE;
    static GLfloat s_lightPosition[4];

    if (!s_bInitialized)
    {
        // 一次性设置不变的光照属性
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);

        GLfloat lightAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f};
        GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
        GLfloat lightSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};

        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

        s_bInitialized = TRUE;
    }

    // 只更新变化的光源位置
    CCamera *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (pCamera)
    {
        Vector3 camPos = pCamera->GetPosition();
        s_lightPosition[0] = camPos.x;
        s_lightPosition[1] = camPos.y + 10.0f;
        s_lightPosition[2] = camPos.z;
        s_lightPosition[3] = 1.0f;

        glLightfv(GL_LIGHT0, GL_POSITION, s_lightPosition);
    }
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

void CDemoScene::ProcessInput(float deltaTime)
{
    auto inputMgr = CGameEngine::GetInstance().GetInputManager();
    auto pCamera = CGameEngine::GetInstance().GetMainCamera();

    CameraMode mode = pCamera->GetMode();

    // CAUTION: 只有在第三人称模式下，WASD 才控制模型实体
    if (m_pPossessedEntity && mode == CameraMode::ThirdPerson)
    {
        UpdateEntities(deltaTime);
    }

    // if (inputMgr->IsKeyPressed('M'))
    // {
    //     // 示例：按一下 M 键，生成一个新的鸭子
    //     // auto pNewModel = CreateExtraDuck();
    //     // m_pRootEntity->AddChild(pNewModel);
    //     LogInfo(L"按下 M 键：成功添加一个新实体，且不会由于按住而导致重复创建。\n");
    // }

    // 示例：按 'L' 键切换地形线框模式
    // if (inputMgr->IsKeyPressed('L'))
    // {
    //     if (m_pTerrain)
    //     {
    //         m_pTerrain->SetWireframe(!m_pTerrain->IsWireframe());
    //     }
    // }

    // 示例：按 'B' 键切换包围盒显示
    // if (inputMgr->IsKeyPressed('B'))
    // {
    //     if (m_pPossessedEntity)
    //     {
    //         m_pPossessedEntity->SetDrawBoundingBox(!m_pPossessedEntity->IsDrawBoundingBox());
    //     }
    // }
}

void CDemoScene::UpdateLogic(float deltaTime)
{
    UpdateAutoSnapping();
}

void CDemoScene::UpdateEntities(float deltaTime)
{
    // 获取引擎子系统
    auto inputMgr = CGameEngine::GetInstance().GetInputManager();
    auto pCamera = CGameEngine::GetInstance().GetMainCamera();

    if (!m_pPossessedEntity || !pCamera)
        return;

    Vector3 inputDir(0, 0, 0);

    if (inputMgr->IsKeyDown('W'))
        inputDir.z += 1.0f;
    if (inputMgr->IsKeyDown('S'))
        inputDir.z -= 1.0f;
    if (inputMgr->IsKeyDown('A'))
        inputDir.x += 1.0f;
    if (inputMgr->IsKeyDown('D'))
        inputDir.x -= 1.0f;

    if (inputDir.Length() > 0.01f)
    {
        inputDir.Normalize();

        // 1. 核心：计算相机参考系下的世界向量
        Vector3 camForward = pCamera->GetForward();
        camForward.y = 0; // 抹平高度差，确保只在地面移动
        camForward.Normalize();

        Vector3 camRight = pCamera->GetRight();
        camRight.y = 0;
        camRight.Normalize();

        // 这里的 moveVec 就是鸭子在世界坐标系中该走的方向
        Vector3 moveVec = (camForward * inputDir.z) + (camRight * inputDir.x);
        moveVec.Normalize();

        // 2. 旋转逻辑：计算目标偏航角 (Yaw)
        // atan2f(x, z) 得到的弧度：0是正Z, PI/2是正X
        float targetRad = atan2f(moveVec.x, moveVec.z);
        float targetYaw = targetRad * 57.29578f; // Rad to Deg

        // 模型偏置修正：如果鸭子“横着走”，改这个值 (-90, 0, 90, 180)
        float finalTargetYaw = targetYaw - 90.0f;

        // 3. 角度插值（处理 180 度跳转问题）
        float angleDiff = finalTargetYaw - m_PossessedEntityYaw;
        while (angleDiff > 180.0f)
            angleDiff -= 360.0f;
        while (angleDiff < -180.0f)
            angleDiff += 360.0f;

        // 平滑旋转：lerpFactor 越大，转向越灵敏
        float lerpFactor = 12.0f;
        m_PossessedEntityYaw += angleDiff * lerpFactor * deltaTime;

        // 4. 应用变换：强制 X=0, Z=0
        // 这行代码解决了“鸭子乱偏”到地下的问题
        m_pPossessedEntity->SetRotation(Vector3(0.0f, m_PossessedEntityYaw, 0.0f));

        // 5. 移动位置
        float moveSpeed = 2.0f;
        Vector3 currentPos = m_pPossessedEntity->GetPosition();
        m_pPossessedEntity->SetPosition(currentPos + (moveVec * moveSpeed * deltaTime));
    }
}

void CDemoScene::RegisterEntityForSnapping(std::shared_ptr<CEntity> pEntity, BOOL isDynamic)
{
    if (!pEntity || !m_pTerrain)
        return;

    if (isDynamic)
    {
        auto it = std::find(m_DynamicSnapEntities.begin(), m_DynamicSnapEntities.end(), pEntity);
        if (it == m_DynamicSnapEntities.end())
        {
            m_DynamicSnapEntities.push_back(pEntity);
            // LogInfo(L"已注册动态贴地实体: %s\n", pEntity->GetName().c_str());
        }
    }
    else
    {
        // 静态物体：直接执行一次贴地，后续不再计算
        Vector3 pos = pEntity->GetPosition();
        float h = m_pTerrain->GetGroundHeight(pos);
        pEntity->SetPosition(Vector3(pos.x, h + pEntity->GetGroundOffset(), pos.z));
    }
}

void CDemoScene::UpdateAutoSnapping()
{
    if (!m_pTerrain || m_DynamicSnapEntities.empty())
        return;

    const float moveThresholdSq = 0.1f * 0.1f;

    for (auto &pEntity : m_DynamicSnapEntities)
    {
        if (pEntity && pEntity->IsAutoSnapEnabled())
        {
            Vector3 currentPos = pEntity->GetPosition();
            Vector3 lastPos = pEntity->GetLastSnapPos();

            // 计算水平面(X,Z)上的位移平方
            float dx = currentPos.x - lastPos.x;
            float dz = currentPos.z - lastPos.z;
            float distSq = dx * dx + dz * dz;

            // 位移阈值判断
            if (distSq > moveThresholdSq)
            {
                float h = m_pTerrain->GetGroundHeight(currentPos);

                // 更新位置（Y轴为高度 + 偏移）
                pEntity->SetPosition(Vector3(currentPos.x, h + pEntity->GetGroundOffset(), currentPos.z));

                // 【关键修复】：更新最后记录的位置，防止下一帧重复进入
                pEntity->SetLastSnapPos(currentPos);
            }
        }
    }
}