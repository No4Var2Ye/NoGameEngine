
// ======================================================================
#include "stdafx.h"
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
    // 2. 创建场景主相机实体
    m_pMainCamera = CCameraEntity::Create();
    if (m_pMainCamera)
    {
        m_pMainCamera->SetName(L"MainSceneCamera");
        m_pMainCamera->SetPosition(Vector3(0.0f, 5.0f, 10.0f));
        // FLOAT aspect = (startH > 0) ? (FLOAT)startW / (FLOAT)startH : 1.0f;
        // m_pMainCamera->SetProjection(45.0f, aspect, 0.1f, 1000.0f);
        m_pMainCamera->SetMode(CameraMode::FreeLook);

        // CAUTION: 不要一开始就 AddChild 到 Root，因为我们可能要动态绑定到鸭子
        // TODO: 后续修改逻辑
        m_pRootEntity->AddChild(m_pMainCamera);
        LogInfo(L"场景相机加载成功\n");
    }

    // ======================================================================
    // 3. 添加天空盒
    GLuint skyboxTexture = LoadSkybox();
    if (skyboxTexture != 0)
    {
        m_pSkybox = CSkyboxEntity::Create(skyboxTexture);
        m_pSkybox->SetName(L"WorldSkybox");
        m_pSkybox->SetSize(500.0f);        // 设置天空盒大小
        m_pSkybox->EnableRotation(TRUE);   // 启用旋转
        m_pSkybox->SetRotationSpeed(2.0f); // 设置旋转速度

        m_pRootEntity->AddChild(m_pSkybox);
        LogInfo(L"天空盒加载成功\n");
    }

    // ======================================================================
    // 4. 添加地形
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
        LogInfo(L"地形加载成功\n");
    }

    // ======================================================================
    // 5. 创建网格坐标实体
    m_pGrid = CGridEntity::Create(500.0f, 2.0f);
    if (m_pGrid)
    {
        m_pGrid->SetPosition(Vector3(0, 3.3f, 0));
        // m_pGrid->SetPosition(Vector3(0, -0.01f, 0));
        m_pGrid->SetFadeDist(100.0f, 500.0f);
        m_pGrid->SetShowAxes(TRUE);
        m_pGrid->SetVisible(FALSE);

        m_pRootEntity->AddChild(m_pGrid);
        LogInfo(L"网格加载成功\n");
    }

    // ======================================================================
    // 6. 加载鸭子模型资源
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

        // 5.1 添加子鸭子
        // ======================================================================
        auto pChildDuckEntity = CModelEntity::Create(pDuckModel); // 复用同一个模型资源
        if (pChildDuckEntity)
        {
            pChildDuckEntity->SetName(L"FollowerDuck");

            // 注意：这里的变换是相对于父鸭子 (pDuckEntity) 的本地空间
            // 父鸭子缩放是 0.01，子鸭子如果设为 0.5，则实际世界缩放是 0.005
            pChildDuckEntity->SetScale(Vector3(0.5f, 0.5f, 0.5f));

            // 位移： (本地坐标)
            pChildDuckEntity->SetPosition(Vector3(100.0f, 0.0f, 0.0f));

            // 旋转
            pChildDuckEntity->SetRotation(Vector3(0.0f, 0.0f, 0.0f));

            // 将其添加为主鸭子的子节点
            pDuckEntity->AddChild(pChildDuckEntity);

            LogDebug(L"子鸭子创建成功，已挂载到主鸭子下\n");
        }

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

    // auto m_pMainCamera = CGameEngine::GetInstance().m_pMainCamera;
    // if (m_pMainCamera)
    // {
    //     m_pMainCamera->ApplyViewMatrix();
    //     m_pMainCamera->ApplyProjectionMatrix();
    // }
    if (m_pMainCamera)
    {
        m_pMainCamera->ApplyViewMatrix(); // 内部应使用 GetWorldPosition()
        m_pMainCamera->ApplyProjectionMatrix();
    }

    // 3. 优化光照设置 - 只在变化时更新
    static Vector3 s_lastCamPos;
    Vector3 camPos = m_pMainCamera ? m_pMainCamera->GetPosition() : Vector3::Zero();
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
    if (m_pMainCamera)
    {
        // 关键点：在固定管线中设置光源位置前，通常需要 LoadIdentity
        // 否则光源会随着相机的平移而发生错误的相对位移
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        Vector3 camPos = m_pMainCamera->GetPosition();
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

// ======================================================================
// TODO: 输入管理
// ======================================================================
void CDemoScene::ProcessInput(float deltaTime)
{
    // 全局快捷键
    ProcessGlobalHotkeys(deltaTime);

    // 相机快捷键
    ProcessCameraInput(deltaTime);
}

void CDemoScene::ProcessGlobalHotkeys(float deltaTime)
{
    auto inputMgr = CGameEngine::GetInstance().GetInputManager();

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
            // LogDebug(L"[线框模式] 切换至: %d\n", m_pTerrain->IsWireframe());
        }
        if (m_pPossessedEntity)
        {
            m_pPossessedEntity->SetWireframe(!m_pPossessedEntity->IsWireframe());
            // LogDebug(L"[线框模式] 切换至: %d\n", m_pPossessedEntity->IsWireframe());
        }
    }
    s_lastF2 = isF2Down;

    // 按 F3 键切换法线模式
    static BOOL s_lastF3 = FALSE;
    BOOL isF3Down = inputMgr->IsKeyDown(Hotkeys::ToggleNormals);
    if (isF3Down && !s_lastF3)
    {
        if (m_pTerrain)
        {
            m_pTerrain->SetDrawNormals(!m_pTerrain->IsDrawNormals());
            // LogDebug(L"[法线模式] 切换至: %d\n", m_pTerrain->IsDrawNormals());
        }
        if (m_pPossessedEntity)
        {
            m_pPossessedEntity->SetDrawNormals(!m_pPossessedEntity->IsDrawNormals());
            // LogDebug(L"[法线模式] 切换至: %d\n", m_pPossessedEntity->IsDrawNormals());
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
            // LogDebug(L"[法线模式] 切换至: %d\n", m_pPossessedEntity->IsDrawBoundingBox());
        }
    }
    s_lastF4 = isF4Down;

    // 按 F5 键切换网格系统显示
    static BOOL s_lastF5 = FALSE;
    BOOL isF5Down = inputMgr->IsKeyDown(Hotkeys::ToggleGrid);
    if (isF5Down && !s_lastF5)
    {
        if (m_pGrid)
        {
            m_pGrid->SetVisible(!m_pGrid->IsVisible());
            // LogDebug(L"[网格系统] 切换至: %d\n", m_pGrid->IsVisible());
        }
    }
    s_lastF5 = isF5Down;
}

void CDemoScene::ProcessCameraInput(float deltaTime)
{
    auto inputMgr = CGameEngine::GetInstance().GetInputManager();
    if (!m_pMainCamera)
        return;

    // ======================================================================
    // 0. 基础重置
    // ======================================================================
    if (inputMgr->IsKeyPressed(Hotkeys::ResetCamera))
    {
        // 重置相机到默认位置和旋转
        m_pMainCamera->SetMode(CameraMode::FreeLook);
        m_pMainCamera->SetPosition(Vector3(0.0f, 5.0f, 10.0f));
        m_pMainCamera->ResetOrientation(0.0f, 0.0f);
    }

    // 相机模式快速切换
    if (inputMgr->IsKeyPressed(Hotkeys::CameraMode1))
        m_pMainCamera->SetMode(CameraMode::FirstPerson);
    if (inputMgr->IsKeyPressed(Hotkeys::CameraMode2))
        m_pMainCamera->SetMode(CameraMode::ThirdPerson);
    if (inputMgr->IsKeyPressed(Hotkeys::CameraMode3))
        m_pMainCamera->SetMode(CameraMode::FreeLook);
    if (inputMgr->IsKeyPressed(Hotkeys::CameraMode4))
        m_pMainCamera->SetMode(CameraMode::Orbital);

    // 触发震动测试
    if (inputMgr->IsKeyPressed(Hotkeys::CameraShakeTest))
    {
        m_pMainCamera->StartShake(0.2f, 0.5f);
    }

    CameraMode mode = m_pMainCamera->GetMode();

    // ======================================================================
    // 1. 旋转与观察
    // ======================================================================
    // 通常点击左键时才允许相机旋转，这样左键可以留给 UI 或 游戏内交互
    // 无论是否按下，都先获取 Delta，防止数据积压
    if (inputMgr->IsMouseButtonPressed(MouseButton::Left))
    {
        inputMgr->HideCursor();
        inputMgr->LockMouse(); // 限制光标在窗口内
    }

    if (inputMgr->IsMouseButtonDown(MouseButton::Left))
    {
        POINT delta = inputMgr->GetMouseDelta();

        if (delta.x != 0 || delta.y != 0)
        {
            // 调用优化后的四元数合成链路
            m_pMainCamera->ProcessMouseMovement(delta.x, delta.y);
        }
    }

    if (inputMgr->IsMouseButtonReleased(MouseButton::Left))
    {
        inputMgr->ShowCursor(); // 恢复鼠标显示
        inputMgr->UnlockMouse();
    }

    // ======================================================================
    // 2. 缩放处理
    // ======================================================================
    INT wheelDelta = inputMgr->GetMouseWheelDelta();
    if (wheelDelta != 0)
    {
        m_pMainCamera->ProcessMouseWheel(wheelDelta);
    }

    // ======================================================================
    // 3. 实体控制逻辑
    // ======================================================================

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

    if (!m_pPossessedEntity || !m_pMainCamera)
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
        Vector3 camForward = m_pMainCamera->GetForward();
        camForward.y = 0; // 抹平高度差，确保只在地面移动
        camForward.Normalize();

        Vector3 camRight = m_pMainCamera->GetRight();
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

                // 更新最后记录的位置，防止下一帧重复进入
                pEntity->SetLastSnapPos(currentPos);
            }
        }
    }
}

// ======================================================================
// 相机更新逻辑
void CDemoScene::UpdateFreeLookCamera(float deltaTime)
{

    if (!m_pMainCamera)
        return;

    auto inputMgr = CGameEngine::GetInstance().GetInputManager();

    float moveSpeed = 5.0f * deltaTime;
    if (inputMgr->IsKeyDown(VK_SHIFT))
        moveSpeed *= 2.5f;

    Vector3 moveVec(0, 0, 0);
    if (inputMgr->IsKeyDown(Hotkeys::MoveForward))
        moveVec += m_pMainCamera->GetForward();
    if (inputMgr->IsKeyDown(Hotkeys::MoveBackward))
        moveVec -= m_pMainCamera->GetForward();
    if (inputMgr->IsKeyDown(Hotkeys::MoveRight))
        moveVec += m_pMainCamera->GetRight();
    if (inputMgr->IsKeyDown(Hotkeys::MoveLeft))
        moveVec -= m_pMainCamera->GetRight();
    if (inputMgr->IsKeyDown(Hotkeys::MoveUp) || inputMgr->IsKeyDown(Hotkeys::MoveUpAlt))
        moveVec += Vector3(0, 1, 0);
    if (inputMgr->IsKeyDown(Hotkeys::MoveDown))
        moveVec -= Vector3(0, 1, 0);

    if (moveVec.LengthSquared() > 0.0f)
    {
        moveVec.Normalize();
        m_pMainCamera->SetPosition(m_pMainCamera->GetPosition() + moveVec * moveSpeed);
    }
}

void CDemoScene::SyncCameraToEntity(float deltaTime)
{
    if (!m_pMainCamera || !m_pPossessedEntity)
        return;

    CameraMode mode = m_pMainCamera->GetMode();

    // 1. 确保相机的父子关系正确 (状态切换检测)
    if (m_pMainCamera->GetParent() != m_pPossessedEntity)
    {
        // 先脱离原有的父节点
        auto pCurrentParent = m_pMainCamera->GetParent();
        if (pCurrentParent)
        {
            // 使用安全的方式移除子节点
            pCurrentParent->RemoveChild(m_pMainCamera->GetID());
            LogDebug(L"相机已从原父节点分离\n");
        }

        // 绑定到被控实体
        m_pPossessedEntity->AddChild(m_pMainCamera->shared_from_this());
        LogInfo(L"相机已绑定到实体: %s\n", m_pPossessedEntity->GetName().c_str());

        // 切换模式时的初始化设置
        if (mode == CameraMode::FirstPerson)
        {
            m_pMainCamera->SetPosition(Vector3(0, 1.8f, 0)); // 眼睛高度
            LogDebug(L"第一人称模式：设置相机高度为 1.8\n");
        }
        else if (mode == CameraMode::ThirdPerson)
        {
            m_pMainCamera->SetPosition(Vector3(0, 1.5f, 5.0f)); // 第三人称默认偏移
            LogDebug(L"第三人称模式：设置相机初始位置\n");
        }
    }

    // 2. 根据模式处理本地变换
    if (mode == CameraMode::ThirdPerson)
    {
        // 第三人称：本地坐标位于父节点后上方
        float dist = 5.0f; // 默认距离
        float yawRad = m_pMainCamera->GetYaw() * 0.01745f;
        float pitchRad = m_pMainCamera->GetPitch() * 0.01745f;

        // 计算球坐标偏移
        Vector3 localOffset(
            sinf(yawRad) * cosf(pitchRad) * dist,
            sinf(pitchRad) * dist + 1.5f, // 基础高度 1.5
            cosf(yawRad) * cosf(pitchRad) * dist);

        // 设置相机相对于父节点的位置
        Vector3 desiredLocalPos = localOffset;

        // 应用平滑位置 (防止相机瞬间跳变)
        Vector3 currentPos = m_pMainCamera->GetPosition();
        Vector3 newPos = Vector3::Lerp(currentPos, desiredLocalPos, deltaTime * 8.0f);
        m_pMainCamera->SetPosition(newPos);

        // 让相机看向父节点中心上方
        Vector3 lookAtTarget(0, 1.5f, 0); // 看向父节点上方 1.5 单位处
        m_pMainCamera->LookAt(lookAtTarget);
    }
    else if (mode == CameraMode::FirstPerson)
    {
        // 第一人称：相机位置固定，旋转跟随父节点
        // 相机位置已经在父子关系中确定，只需要同步旋转
        Vector3 parentRotation = m_pPossessedEntity->GetRotation().ToEuler();
        m_pMainCamera->SetRotation(Quaternion::FromEuler(0, parentRotation.y, 0));
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

        m_pMainCamera->SetPosition(orbitalOffset);
        m_pMainCamera->LookAt(Vector3(0, 1.5f, 0)); // 看向父节点中心
    }
    else if (mode == CameraMode::FreeLook)
    {
        // 自由视角：解除父子关系，相机独立控制
        auto pCurrentParent = m_pMainCamera->GetParent();
        if (pCurrentParent)
        {
            pCurrentParent->RemoveChild(m_pMainCamera->GetID());
            LogDebug(L"自由视角模式：解除相机父子关系\n");
        }
    }
}