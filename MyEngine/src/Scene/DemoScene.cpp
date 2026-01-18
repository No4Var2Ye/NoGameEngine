
// ======================================================================
#include "stdafx.h"
#include <functional>
#include "EngineConfig.h"
#include "Scene/DemoScene.h"
#include "Core/GameEngine.h"
#include "Core/InputManager.h"
#include "Core/Entity.h"
#include "Resources/ResourceManager.h"
#include "Entities/CameraEntity.h"
#include "Entities/ModelEntity.h"
#include "Entities/SkyboxEntity.h"
#include "Entities/GridEntity.h"
#include "Entities/TerrainEntity.h"
// ======================================================================

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

        m_pTerrain->SetNormalScale(1.5f);
        m_pTerrain->SetNormalStep(10);
        m_pTerrain->SetDrawNormals(FALSE);

        m_pTerrain->SetPosition(Vector3(0, 0, 0));

        m_pRootEntity->AddChild(m_pTerrain);
        LogInfo(L"地形创建成功\n");
    }

    // ======================================================================
    // 4. 创建网格坐标实体
    m_pGrid = CGridEntity::Create(500.0f, 2.0f);
    if (m_pGrid)
    {
        m_pGrid->SetPosition(Vector3(0, 3.3f, 0));
        // m_pGrid->SetPosition(Vector3(0, -0.01f, 0));
        m_pGrid->SetFadeDist(100.0f, 500.0f);
        m_pGrid->SetShowAxes(TRUE);
        m_pGrid->SetVisible(TRUE);

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
        pDuckEntity->SetPosition(Vector3(0.0f, 0.0f, -5.0f));
        pDuckEntity->SetScale(Vector3(0.01f, 0.01f, 0.01f));
        // pDuckEntity->SetRotation(Vector3(0.0f, -90.0f, 0.0f));

        // 调试：输出模型信息
        // LogInfo(L"鸭子模型位置: (0, 1, 0), 缩放: 0.01.\n");

        pDuckEntity->SetDrawBoundingBox(FALSE);
        pDuckEntity->SetNormalScale(10.0f);
        pDuckEntity->SetNormalStep(10);
        pDuckEntity->SetDrawNormals(FALSE);

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

    auto pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (pCamera)
    {
        pCamera->ApplyViewMatrix();
        pCamera->ApplyProjectionMatrix();
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
    CCameraEntity *pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (pCamera)
    {
        // 关键点：在固定管线中设置光源位置前，通常需要 LoadIdentity
        // 否则光源会随着相机的平移而发生错误的相对位移
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        Vector3 camPos = pCamera->GetPosition();
        GLfloat lightPos[] = {camPos.x, camPos.y + 50.0f, camPos.z, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

        glPopMatrix();
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
    if (!pCamera)
        return;

    // ======================================================================
    // 1. 场景全局快捷键
    // ======================================================================

    // 按 F2 键切换地形线框模式
    static BOOL s_lastF2 = FALSE;
    BOOL isF2Down = inputMgr->IsKeyDown(Hotkeys::ToggleWireframe);
    if (isF2Down && !s_lastF2)
    {
        if (m_pTerrain)
        {
            m_pTerrain->SetWireframe(!m_pTerrain->IsWireframe());
            LogDebug(L"[线框模式] 切换至: %d\n", m_pTerrain->IsWireframe());
        }
        if (m_pPossessedEntity)
        {
            m_pPossessedEntity->SetWireframe(!m_pPossessedEntity->IsWireframe());
            LogDebug(L"[线框模式] 切换至: %d\n", m_pPossessedEntity->IsWireframe());
        }
    }
    s_lastF2 = isF2Down;

    static BOOL s_lastF3 = FALSE;
    BOOL isF3Down = inputMgr->IsKeyDown(Hotkeys::ToggleNormals);
    if (isF3Down && !s_lastF3)
    {
        if (m_pTerrain)
        {
            m_pTerrain->SetDrawNormals(!m_pTerrain->IsDrawNormals());
            LogDebug(L"[法线模式] 切换至: %d\n", m_pTerrain->IsDrawNormals());
        }
        if (m_pPossessedEntity)
        {
            m_pPossessedEntity->SetDrawNormals(!m_pPossessedEntity->IsDrawNormals());
            LogDebug(L"[法线模式] 切换至: %d\n", m_pPossessedEntity->IsDrawNormals());
        }
    }
    s_lastF3 = isF3Down;

    // 按 F4 键切换包围盒显示
    static BOOL s_lastF4 = FALSE;
    BOOL isF4Down = inputMgr->IsKeyDown(Hotkeys::ToggleBoundingBox);
    if (isF4Down && !s_lastF4)
    {
        if (m_pPossessedEntity)
        {
            m_pPossessedEntity->SetDrawBoundingBox(!m_pPossessedEntity->IsDrawBoundingBox());
            LogDebug(L"[法线模式] 切换至: %d\n", m_pPossessedEntity->IsDrawBoundingBox());
        }
    }
    s_lastF4 = isF4Down;
    // ======================================================================
    // 2. 实体控制逻辑
    // ======================================================================
    CameraMode mode = pCamera->GetMode();

    if (!m_pPossessedEntity)
        return;
    if (mode == CameraMode::FreeLook)
    {
        UpdateFreeLookCamera(deltaTime);
    }
    else if (mode == CameraMode::FirstPerson || mode == CameraMode::ThirdPerson)
    {
        UpdateEntities(deltaTime);
        // SyncCameraToEntity(deltaTime);
    }
    else if (mode == CameraMode::Orbital)
    {
        // UpdateOrbitalCamera(deltaTime);
    }
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

    if (inputMgr->IsKeyDown(Hotkeys::MoveForward))
        inputDir.z += 1.0f;
    if (inputMgr->IsKeyDown(Hotkeys::MoveBackward))
        inputDir.z -= 1.0f;
    if (inputMgr->IsKeyDown(Hotkeys::MoveRight))
        inputDir.x += 1.0f;
    if (inputMgr->IsKeyDown(Hotkeys::MoveLeft))
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
        m_pPossessedEntity->SetRotation(Vector3(0.0f, m_PossessedEntityYaw, 0.0f));

        // 3. 应用位移
        float moveSpeed = 5.0f; // 基础速度
        if (inputMgr->IsKeyDown(VK_SHIFT))
            moveSpeed *= 2.0f; // 鸭子加速

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

// ======================================================================
// 相机更新逻辑
void CDemoScene::UpdateFreeLookCamera(float deltaTime)
{
    auto inputMgr = CGameEngine::GetInstance().GetInputManager();
    auto pCamera = CGameEngine::GetInstance().GetMainCamera();

    float moveSpeed = 5.0f * deltaTime;
    if (inputMgr->IsKeyDown(VK_SHIFT))
        moveSpeed *= 2.5f;

    Vector3 moveVec(0, 0, 0);
    if (inputMgr->IsKeyDown(Hotkeys::MoveForward))
        moveVec += pCamera->GetForward();
    if (inputMgr->IsKeyDown(Hotkeys::MoveBackward))
        moveVec -= pCamera->GetForward();
    if (inputMgr->IsKeyDown(Hotkeys::MoveRight))
        moveVec += pCamera->GetRight();
    if (inputMgr->IsKeyDown(Hotkeys::MoveLeft))
        moveVec -= pCamera->GetRight();
    if (inputMgr->IsKeyDown(Hotkeys::MoveUp) || inputMgr->IsKeyDown(Hotkeys::MoveUpAlt))
        moveVec += Vector3(0, 1, 0);
    if (inputMgr->IsKeyDown(Hotkeys::MoveDown))
        moveVec -= Vector3(0, 1, 0);

    if (moveVec.LengthSquared() > 0.0f)
    {
        moveVec.Normalize();
        pCamera->SetPosition(pCamera->GetPosition() + moveVec * moveSpeed);
    }
}

void CDemoScene::SyncCameraToEntity(float deltaTime)
{
    auto pCamera = CGameEngine::GetInstance().GetMainCamera();
    if (!pCamera || !m_pPossessedEntity)
        return;

    CameraMode mode = pCamera->GetMode();

    // 1. 确保相机的父子关系正确 (状态切换检测)
    if (pCamera->GetParent() != m_pPossessedEntity)
    {
        // 先脱离原有的父节点
        auto pCurrentParent = pCamera->GetParent();
        if (pCurrentParent)
        {
            // 使用安全的方式移除子节点
            pCurrentParent->RemoveChild(pCamera->GetID());
            LogDebug(L"相机已从原父节点分离\n");
        }

        // 绑定到被控实体
        m_pPossessedEntity->AddChild(pCamera->shared_from_this());
        LogInfo(L"相机已绑定到实体: %s\n", m_pPossessedEntity->GetName().c_str());

        // 切换模式时的初始化设置
        if (mode == CameraMode::FirstPerson)
        {
            pCamera->SetPosition(Vector3(0, 1.8f, 0)); // 眼睛高度
            LogDebug(L"第一人称模式：设置相机高度为 1.8\n");
        }
        else if (mode == CameraMode::ThirdPerson)
        {
            pCamera->SetPosition(Vector3(0, 1.5f, 5.0f)); // 第三人称默认偏移
            LogDebug(L"第三人称模式：设置相机初始位置\n");
        }
    }

    // 2. 根据模式处理本地变换
    if (mode == CameraMode::ThirdPerson)
    {
        // 第三人称：本地坐标位于父节点后上方
        float dist = 5.0f; // 默认距离
        float yawRad = pCamera->GetYaw() * 0.01745f;
        float pitchRad = pCamera->GetPitch() * 0.01745f;

        // 计算球坐标偏移
        Vector3 localOffset(
            sinf(yawRad) * cosf(pitchRad) * dist,
            sinf(pitchRad) * dist + 1.5f, // 基础高度 1.5
            cosf(yawRad) * cosf(pitchRad) * dist);

        // 设置相机相对于父节点的位置
        Vector3 desiredLocalPos = localOffset;

        // 应用平滑位置 (防止相机瞬间跳变)
        Vector3 currentPos = pCamera->GetPosition();
        Vector3 newPos = Vector3::Lerp(currentPos, desiredLocalPos, deltaTime * 8.0f);
        pCamera->SetPosition(newPos);

        // 让相机看向父节点中心上方
        Vector3 lookAtTarget(0, 1.5f, 0); // 看向父节点上方 1.5 单位处
        pCamera->LookAt(lookAtTarget);
    }
    else if (mode == CameraMode::FirstPerson)
    {
        // 第一人称：相机位置固定，旋转跟随父节点
        // 相机位置已经在父子关系中确定，只需要同步旋转
        Vector3 parentRotation = m_pPossessedEntity->GetRotation().ToEuler();
        pCamera->SetRotation(Quaternion::FromEuler(0, parentRotation.y, 0));
    }
    else if (mode == CameraMode::Orbital)
    {
        // 轨道模式：围绕父节点旋转
        float dist = 8.0f; // 轨道距离
        static float orbitAngle = 0.0f;
        orbitAngle += deltaTime * 1.0f; // 每秒旋转 1 弧度

        Vector3 orbitalOffset(
            sinf(orbitAngle) * dist,
            3.0f, // 轨道高度
            cosf(orbitAngle) * dist);

        pCamera->SetPosition(orbitalOffset);
        pCamera->LookAt(Vector3(0, 1.5f, 0)); // 看向父节点中心
    }
    else if (mode == CameraMode::FreeLook)
    {
        // 自由视角：解除父子关系，相机独立控制
        auto pCurrentParent = pCamera->GetParent();
        if (pCurrentParent)
        {
            pCurrentParent->RemoveChild(pCamera->GetID());
            LogDebug(L"自由视角模式：解除相机父子关系\n");
        }
    }
}