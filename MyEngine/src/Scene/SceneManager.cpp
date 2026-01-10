
// ======================================================================
#include "stdafx.h"

#include <algorithm>
#include "Scene/SceneManager.h"
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

    m_Initialized = TRUE;

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

    // 最简单：设置一个纯色背景
    glClearColor(0.2f, 0.3f, 0.8f, 1.0f); // 蓝色背景
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();  // 保存当前矩阵状态

    // glTranslatef(0.0f, 0.0f, -10.0f);     // 最后：平移到(3,0,0)
    glRotatef(60.0f, 1.0f, 1.0f, 0.0f); // 中间：绕Y轴旋转45度
    // glScalef(0.5f, 0.5f, 0.5f);         // 首先：缩小到一半
    // 四面体的4个顶点（正四面体）
    FLOAT tetrahedronVertices[4][3] = {
        {1.0f, 1.0f, 1.0f},   // 顶点0: 右上前
        {-1.0f, -1.0f, 1.0f}, // 顶点1: 左下前
        {-1.0f, 1.0f, -1.0f}, // 顶点2: 左后上
        {1.0f, -1.0f, -1.0f}  // 顶点3: 右后下
    };
    // 四面体的4个面颜色
    FLOAT tetrahedronColors[4][4] = {
        {1.0f, 0.0f, 0.0f, 1.0f}, // 面0: 红色
        {0.0f, 1.0f, 0.0f, 1.0f}, // 面1: 绿色
        {0.0f, 0.0f, 1.0f, 1.0f}, // 面2: 蓝色
        {1.0f, 1.0f, 0.0f, 1.0f}  // 面3: 黄色
    };
    glBegin(GL_TRIANGLES);
    // 面0: 顶点0,1,2
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3fv(tetrahedronVertices[0]);
    glVertex3fv(tetrahedronVertices[1]);
    glVertex3fv(tetrahedronVertices[2]);
    // 面1: 顶点0,2,3
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3fv(tetrahedronVertices[0]);
    glVertex3fv(tetrahedronVertices[2]);
    glVertex3fv(tetrahedronVertices[3]);
    // 面2: 顶点0,3,1
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3fv(tetrahedronVertices[0]);
    glVertex3fv(tetrahedronVertices[3]);
    glVertex3fv(tetrahedronVertices[1]);
    // 面3: 顶点1,3,2
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3fv(tetrahedronVertices[1]);
    glVertex3fv(tetrahedronVertices[3]);
    glVertex3fv(tetrahedronVertices[2]);
    glEnd();
    glPopMatrix();  // 恢复矩阵
    
    // SwapBuffers(wglGetCurrentDC());
    
    // if (!m_Initialized || !m_RenderEnabled)
    //     return;
    
    // // 渲染当前场景
    // if (m_CurrentScene && !IsInTransition())
    // {
    //     m_CurrentScene->Render();
    // }
    // else if (IsInTransition())
    // {
    //     // 在过渡期间，可能需要渲染前后两个场景
    //     // 这里简化处理，只渲染当前场景
    //     if (m_CurrentScene)
    //     {
    //         m_CurrentScene->Render();
    //     }
    // }

    // // 渲染过渡效果
    // if (IsInTransition())
    // {
    //     RenderTransition();
    // }
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