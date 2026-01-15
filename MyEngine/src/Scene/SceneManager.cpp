
// ======================================================================
#include "stdafx.h"
#include "EngineConfig.h"
#include "Scene/SceneManager.h"
#include "Core/GameEngine.h"
#include "Resources/Texture.h"
#include "Resources/ResourceManager.h"
#include "Resources/Model.h"
#include "Core/Entity.h"
// ======================================================================
// TODO: 测试
namespace
{
    ResourceConfig config;
    // 测试贴图加载
    std::shared_ptr<CTexture> g_pTexture = nullptr;
    BOOL g_bTextureLoaded = FALSE;

    // 测试 模型加载
    std::shared_ptr<CModel> g_pTestModel = nullptr;
    BOOL g_bModelLoaded = FALSE;
    FLOAT g_modelRotation = 0.0f;
    Vector3 g_modelPosition = Vector3(0.0f, 0.0f, 0.0f);
    FLOAT g_modelScale = 0.01f;
    BOOL g_showModel = TRUE;
}

void DrawGrid()
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

void DrawColorCube()
{
    glPushMatrix();
    {
        // 将立方体稍微抬高一点，放在坐标原点上方
        glTranslatef(0.0f, 1.0f, 0.0f);

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

void DrawTexturedCube()
{
    glDisable(GL_LIGHTING);            // 暂时关掉光照，排除光照干扰
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // 强制设为纯白，确保纹理 1:1 输出

    static bool bAttempted = false;

    if (!bAttempted)
    {
        bAttempted = true; // 无论成功失败，只试一次
        g_pTexture = std::make_shared<CTexture>();
        if (!g_pTexture->LoadFromFile(L"res/Textures/test-texture.png"))
        {
            // 如果加载失败，使用资源管理器里的兜底纹理
            g_pTexture = CGameEngine::GetInstance().GetResourceManager()->GetDefaultTexture();
        }
    }

    glPushMatrix();
    glTranslatef(4.0f, 1.0f, 0.0f);

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

// ======================================================================
// =========================== 公有方法 ==================================
// ======================================================================
CSceneManager::CSceneManager()
    : m_CurrentScene(nullptr),                  //
      m_NextScene(nullptr),                     //
      m_Initialized(FALSE),                     //
      m_SceneChangePending(FALSE),              //
      m_Paused(FALSE),                          //
      m_UpdateEnabled(TRUE),                    //
      m_RenderEnabled(TRUE),                    //
      m_TransitionState(TransitionState::None), //
      m_TransitionAlpha(0.0f),                  //
      m_TransitionSpeed(2.0f),                  //
      m_TransitionStartTime(0),                 //
      m_TransitionWaitTime(500)                 //
{
}

CSceneManager::~CSceneManager()
{
    Shutdown();
}

BOOL CSceneManager::Initialize()
{
    if (m_Initialized)
        return TRUE;

    m_Scenes.clear();
    m_CurrentScene = nullptr;
    m_NextScene = nullptr;
    m_SceneChangePending = FALSE;
    m_Paused = FALSE;
    m_UpdateEnabled = TRUE;
    m_RenderEnabled = TRUE;

    m_TransitionState = TransitionState::None;
    m_TransitionAlpha = 0.0f;

    // 测试场景
    InitTestResources();

    m_Initialized = TRUE;

    LogInfo(L"------------------- 场景管理器初始化成功 ----------------------\n");

    return TRUE;
}

void CSceneManager::Shutdown()
{
    if (!m_Initialized)
        return;

    // 关闭当前场景
    if (m_CurrentScene)
    {
        m_CurrentScene->OnDeactivate();
        m_CurrentScene->Shutdown();
        m_CurrentScene = nullptr;
    }

    // 关闭所有场景
    for (auto &scene : m_Scenes)
    {
        if (scene->IsInitialized())
        {
            scene->Shutdown();
        }
    }

    m_Scenes.clear();
    m_NextScene = nullptr;
    m_SceneChangePending = FALSE;

    m_Initialized = FALSE;

    LogInfo(L"------------------- 场景管理器关闭成功 -------------------------\n");
}

BOOL CSceneManager::RegisterScene(std::shared_ptr<CScene> scene)
{
    if (!scene)
        return FALSE;

    // 检查是否已存在同名场景
    for (const auto &existingScene : m_Scenes)
    {
        if (existingScene->GetName() == scene->GetName())
        {
            return FALSE;
        }
    }

    m_Scenes.push_back(scene);
    return TRUE;
}

BOOL CSceneManager::UnregisterScene(const std::string &sceneName)
{
    auto it = std::remove_if(m_Scenes.begin(), m_Scenes.end(),
                             [&sceneName](const std::shared_ptr<CScene> &scene)
                             {
                                 return scene->GetName() == sceneName;
                             });

    if (it != m_Scenes.end())
    {
        // 如果正在卸载当前场景，需要先停用
        if (m_CurrentScene && m_CurrentScene->GetName() == sceneName)
        {
            m_CurrentScene->OnDeactivate();
            m_CurrentScene = nullptr;
        }

        m_Scenes.erase(it, m_Scenes.end());
        return TRUE;
    }

    return FALSE;
}

std::shared_ptr<CScene> CSceneManager::GetScene(const std::string &sceneName) const
{
    auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(),
                           [&sceneName](const std::shared_ptr<CScene> &scene)
                           {
                               return scene->GetName() == sceneName;
                           });

    if (it != m_Scenes.end())
    {
        return *it;
    }

    return nullptr;
}

void CSceneManager::GetAllSceneNames(std::vector<std::string> &names) const
{
    names.clear();
    for (const auto &scene : m_Scenes)
    {
        names.push_back(scene->GetName());
    }
}

BOOL CSceneManager::ChangeScene(const std::string &sceneName, BOOL withTransition)
{
    std::shared_ptr<CScene> newScene = GetScene(sceneName);
    if (!newScene)
        return FALSE;

    if (withTransition)
    {
        // 使用过渡效果
        m_NextScene = newScene;
        m_SceneChangePending = TRUE;
        m_TransitionState = TransitionState::FadeOut;
        m_TransitionAlpha = 0.0f;
        m_TransitionStartTime = GetTickCount();
        return TRUE;
    }
    else
    {
        // 立即切换
        return ChangeSceneImmediate(sceneName);
    }
}

BOOL CSceneManager::ChangeSceneImmediate(const std::string &sceneName)
{
    std::shared_ptr<CScene> newScene = GetScene(sceneName);
    if (!newScene)
        return FALSE;

    // 停用当前场景
    if (m_CurrentScene)
    {
        m_CurrentScene->OnDeactivate();
    }

    // 初始化新场景（如果尚未初始化）
    if (!newScene->IsInitialized())
    {
        if (!newScene->Initialize())
        {
            return FALSE;
        }
    }

    // 切换到新场景
    m_CurrentScene = newScene;
    m_CurrentScene->OnActivate();

    // 重置过渡状态
    m_TransitionState = TransitionState::None;
    m_SceneChangePending = FALSE;

    return TRUE;
}

BOOL CSceneManager::ChangeToNextScene(BOOL withTransition)
{
    if (m_Scenes.empty())
        return FALSE;

    if (!m_CurrentScene)
    {
        // 如果当前没有场景，切换到第一个场景
        return ChangeScene(m_Scenes[0]->GetName(), withTransition);
    }

    // 查找当前场景的索引
    size_t currentIndex = 0;
    for (size_t i = 0; i < m_Scenes.size(); ++i)
    {
        if (m_Scenes[i]->GetName() == m_CurrentScene->GetName())
        {
            currentIndex = i;
            break;
        }
    }

    // 计算下一个场景索引
    size_t nextIndex = (currentIndex + 1) % m_Scenes.size();

    return ChangeScene(m_Scenes[nextIndex]->GetName(), withTransition);
}

BOOL CSceneManager::ChangeToPreviousScene(BOOL withTransition)
{
    if (m_Scenes.empty())
        return FALSE;

    if (!m_CurrentScene)
    {
        // 如果当前没有场景，切换到最后一个场景
        return ChangeScene(m_Scenes.back()->GetName(), withTransition);
    }

    // 查找当前场景的索引
    size_t currentIndex = 0;
    for (size_t i = 0; i < m_Scenes.size(); ++i)
    {
        if (m_Scenes[i]->GetName() == m_CurrentScene->GetName())
        {
            currentIndex = i;
            break;
        }
    }

    // 计算上一个场景索引
    size_t prevIndex = (currentIndex == 0) ? m_Scenes.size() - 1 : currentIndex - 1;

    return ChangeScene(m_Scenes[prevIndex]->GetName(), withTransition);
}

void CSceneManager::RestartCurrentScene(BOOL withTransition)
{
    if (!m_CurrentScene)
        return;

    ChangeScene(m_CurrentScene->GetName(), withTransition);
}

void CSceneManager::PerformSceneChange()
{
    if (!m_NextScene)
        return;

    // 停用当前场景
    if (m_CurrentScene)
    {
        m_CurrentScene->OnDeactivate();
    }

    // 初始化新场景（如果尚未初始化）
    if (!m_NextScene->IsInitialized())
    {
        if (!m_NextScene->Initialize())
        {
            // 初始化失败，恢复原场景
            if (m_CurrentScene)
            {
                m_CurrentScene->OnActivate();
            }
            m_NextScene = nullptr;
            m_SceneChangePending = FALSE;
            m_TransitionState = TransitionState::None;
            return;
        }
    }

    // 切换到新场景
    m_CurrentScene = m_NextScene;
    m_CurrentScene->OnActivate();

    m_NextScene = nullptr;
    m_SceneChangePending = FALSE;
}

void CSceneManager::PauseCurrentScene()
{
    if (m_CurrentScene && !m_CurrentScene->IsPaused())
    {
        m_CurrentScene->OnPause();
    }
}

void CSceneManager::ResumeCurrentScene()
{
    if (m_CurrentScene && m_CurrentScene->IsPaused())
    {
        m_CurrentScene->OnResume();
    }
}

void CSceneManager::ReloadCurrentScene()
{
    if (!m_CurrentScene)
        return;

    m_CurrentScene->Reload();
}

void CSceneManager::Update(FLOAT deltaTime)
{
    if (!m_Initialized || m_Paused || !m_UpdateEnabled)
        return;

    // 更新过渡效果
    if (IsInTransition())
    {
        UpdateTransition(deltaTime);
    }

    // 更新当前场景
    if (m_CurrentScene && !m_CurrentScene->IsPaused() && !IsInTransition())
    {
        m_CurrentScene->ProcessInput(deltaTime);
        m_CurrentScene->Update(deltaTime);
    }
}

// TODO: 渲染场景
void CSceneManager::Render()
{
    if (!m_Initialized || !m_RenderEnabled)
        return;

    // glMatrixMode(GL_MODELVIEW);
    // DrawGrid();
    // // DrawColorCube();
    // // DrawTexturedCube();
    // RenderTestModel();

    SwapBuffers(wglGetCurrentDC());

    if (!m_Initialized || !m_RenderEnabled)
        return;

    // 渲染当前场景
    if (m_CurrentScene && !IsInTransition())
    {
        m_CurrentScene->Render();
    }
    else if (IsInTransition())
    {
        // 在过渡期间，可能需要渲染前后两个场景
        // 这里简化处理，只渲染当前场景
        if (m_CurrentScene)
        {
            m_CurrentScene->Render();
        }
    }

    // 渲染过渡效果
    if (IsInTransition())
    {
        RenderTransition();
    }
}

void CSceneManager::UpdateTransition(FLOAT deltaTime)
{
    if (m_TransitionState == TransitionState::FadeOut)
    {
        // 淡出
        m_TransitionAlpha += m_TransitionSpeed * deltaTime;
        if (m_TransitionAlpha >= 1.0f)
        {
            m_TransitionAlpha = 1.0f;
            m_TransitionState = TransitionState::Waiting;
            m_TransitionStartTime = GetTickCount();

            // 执行场景切换
            PerformSceneChange();
        }
    }
    else if (m_TransitionState == TransitionState::Waiting)
    {
        // 等待
        DWORD currentTime = GetTickCount();
        if (currentTime - m_TransitionStartTime >= m_TransitionWaitTime)
        {
            m_TransitionState = TransitionState::FadeIn;
        }
    }
    else if (m_TransitionState == TransitionState::FadeIn)
    {
        // 淡入
        m_TransitionAlpha -= m_TransitionSpeed * deltaTime;
        if (m_TransitionAlpha <= 0.0f)
        {
            m_TransitionAlpha = 0.0f;
            m_TransitionState = TransitionState::None;
        }
    }
}

void CSceneManager::RenderTransition()
{
    // 保存当前OpenGL状态
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

    // 禁用深度测试
    glDisable(GL_DEPTH_TEST);

    // 启用混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 设置正交投影
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // 绘制黑色矩形
    glColor4f(0.0f, 0.0f, 0.0f, m_TransitionAlpha);

    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();

    // 恢复矩阵
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    // 恢复OpenGL状态
    glPopAttrib();
}

void CSceneManager::SkipTransition()
{
    if (!IsInTransition())
        return;

    if (m_TransitionState == TransitionState::FadeOut)
    {
        m_TransitionAlpha = 1.0f;
        PerformSceneChange();
        m_TransitionState = TransitionState::None;
    }
    else if (m_TransitionState == TransitionState::Waiting)
    {
        PerformSceneChange();
        m_TransitionState = TransitionState::None;
    }
    else if (m_TransitionState == TransitionState::FadeIn)
    {
        m_TransitionAlpha = 0.0f;
        m_TransitionState = TransitionState::None;
    }
}

void CSceneManager::CleanupUnusedScenes()
{
    for (auto it = m_Scenes.begin(); it != m_Scenes.end();)
    {
        auto &scene = *it;

        // 如果场景不是当前场景，且引用计数为1（只有管理器持有），则可以清理
        if ((!m_CurrentScene || scene->GetName() != m_CurrentScene->GetName()) &&
            scene.use_count() == 1)
        {
            if (scene->IsInitialized())
            {
                scene->Shutdown();
            }
            it = m_Scenes.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

BOOL CSceneManager::SaveCurrentSceneState()
{
    // 这是一个占位函数，具体实现取决于场景的设计
    // 子类可以重写此函数来实现具体的状态保存逻辑
    return TRUE;
}

BOOL CSceneManager::LoadSavedSceneState()
{
    // 这是一个占位函数，具体实现取决于场景的设计
    // 子类可以重写此函数来实现具体的状态加载逻辑
    return TRUE;
}

void CSceneManager::InitTestResources()
{
    // 如果模型已经加载过，直接返回，避免重复加载
    if (g_bModelLoaded)
        return;

    auto resMgr = CGameEngine::GetInstance().GetResourceManager();
    if (!resMgr)
    {
        LogError(L"资源管理器未初始化，无法加载模型\n");
        return;
    }

    g_pTestModel = resMgr->GetModel(L"Duck/glTF/Duck.gltf");
    // g_pTestModel = resMgr->GetModel(L"Teapot/teapot.obj");

    if (!g_pTestModel)
    {
        LogError(L"模型加载完全失败\n");
        g_bModelLoaded = FALSE;
        return;
    }

    auto defaultModel = resMgr->GetDefaultModel();
    if (defaultModel && (g_pTestModel == defaultModel))
    {
        LogWarning(L"鸭子模型不存在，已自动使用默认立方体模型\n");
        LogWarning(L"请检查文件是否存在: res/Models/Duck/glTF/Duck.gltf\n");

        // 调整默认模型的显示
        g_modelScale = 1.0f;
    }
    else
    {
        LogInfo(L"鸭子模型加载成功\n");
    }

    // 设置模型变换
    g_pTestModel->SetPosition(g_modelPosition);
    g_pTestModel->SetScale(Vector3(g_modelScale, g_modelScale, g_modelScale));

    g_bModelLoaded = TRUE;
}

void CSceneManager::RenderTestModel()
{
    if (!g_bModelLoaded || !g_pTestModel)
        return;

    glDisable(GL_LIGHTING);            // 暂时关掉光照，排除光照干扰
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f); // 强制设为纯白，确保纹理 1:1 输出

    glPushMatrix();

    g_pTestModel->Draw();
    g_pTestModel->DrawBoundingBox();

    glPopMatrix();
}