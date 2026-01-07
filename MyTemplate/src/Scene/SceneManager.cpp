#include "stdafx.h"
#include "Scene/SceneManager.h"
#include <algorithm>

/**
 * @brief 构造函数
 */
CSceneManager::CSceneManager()
    : m_CurrentScene(nullptr)
    , m_NextScene(nullptr)
    , m_Initialized(FALSE)
    , m_SceneChangePending(FALSE)
    , m_Paused(FALSE)
    , m_UpdateEnabled(TRUE)
    , m_RenderEnabled(TRUE)
    , m_TransitionState(TransitionState::None)
    , m_TransitionAlpha(0.0f)
    , m_TransitionSpeed(2.0f)
    , m_TransitionStartTime(0)
    , m_TransitionWaitTime(500)
{
}

/**
 * @brief 析构函数
 */
CSceneManager::~CSceneManager()
{
    Shutdown();
}

/**
 * @brief 初始化场景管理器
 */
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

/**
 * @brief 关闭场景管理器
 */
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
    for (auto& scene : m_Scenes)
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

/**
 * @brief 注册场景
 */
BOOL CSceneManager::RegisterScene(std::shared_ptr<CScene> scene)
{
    if (!scene)
        return FALSE;
    
    // 检查是否已存在同名场景
    for (const auto& existingScene : m_Scenes)
    {
        if (existingScene->GetName() == scene->GetName())
        {
            return FALSE;
        }
    }
    
    m_Scenes.push_back(scene);
    return TRUE;
}

/**
 * @brief 取消注册场景
 */
BOOL CSceneManager::UnregisterScene(const std::string& sceneName)
{
    auto it = std::remove_if(m_Scenes.begin(), m_Scenes.end(),
        [&sceneName](const std::shared_ptr<CScene>& scene) {
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

/**
 * @brief 获取场景
 */
std::shared_ptr<CScene> CSceneManager::GetScene(const std::string& sceneName) const
{
    auto it = std::find_if(m_Scenes.begin(), m_Scenes.end(),
        [&sceneName](const std::shared_ptr<CScene>& scene) {
            return scene->GetName() == sceneName;
        });
    
    if (it != m_Scenes.end())
    {
        return *it;
    }
    
    return nullptr;
}

/**
 * @brief 获取所有场景名称
 */
void CSceneManager::GetAllSceneNames(std::vector<std::string>& names) const
{
    names.clear();
    for (const auto& scene : m_Scenes)
    {
        names.push_back(scene->GetName());
    }
}

/**
 * @brief 切换到指定场景
 */
BOOL CSceneManager::ChangeScene(const std::string& sceneName, BOOL withTransition)
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

/**
 * @brief 直接切换到指定场景
 */
BOOL CSceneManager::ChangeSceneImmediate(const std::string& sceneName)
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

/**
 * @brief 切换到下一个场景
 */
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

/**
 * @brief 切换到上一个场景
 */
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

/**
 * @brief 重启当前场景
 */
void CSceneManager::RestartCurrentScene(BOOL withTransition)
{
    if (!m_CurrentScene)
        return;
    
    ChangeScene(m_CurrentScene->GetName(), withTransition);
}

/**
 * @brief 执行场景切换
 */
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

/**
 * @brief 暂停当前场景
 */
void CSceneManager::PauseCurrentScene()
{
    if (m_CurrentScene && !m_CurrentScene->IsPaused())
    {
        m_CurrentScene->OnPause();
    }
}

/**
 * @brief 恢复当前场景
 */
void CSceneManager::ResumeCurrentScene()
{
    if (m_CurrentScene && m_CurrentScene->IsPaused())
    {
        m_CurrentScene->OnResume();
    }
}

/**
 * @brief 重新加载当前场景
 */
void CSceneManager::ReloadCurrentScene()
{
    if (!m_CurrentScene)
        return;
    
    m_CurrentScene->Reload();
}

/**
 * @brief 更新所有场景
 */
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

/**
 * @brief 渲染所有场景
 */
// TODO: 渲染场景
void CSceneManager::Render()
{
    // 最简单：设置一个纯色背景
    glClearColor(0.2f, 0.3f, 0.8f, 1.0f);  // 蓝色背景
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    DrawTestTriangle();

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

void CSceneManager::DrawTestTriangle()
{
    // 绘制一个简单的彩色三角形
    glBegin(GL_TRIANGLES);
    
    glColor3f(1.0f, 0.0f, 0.0f);  // 红色
    glVertex3f(-0.5f, -0.5f, 0.0f);
    
    glColor3f(0.0f, 1.0f, 0.0f);  // 绿色
    glVertex3f(0.5f, -0.5f, 0.0f);
    
    glColor3f(0.0f, 0.0f, 1.0f);  // 蓝色
    glVertex3f(0.0f, 0.5f, 0.0f);
    
    glEnd();
}

/**
 * @brief 更新场景过渡效果
 */
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

/**
 * @brief 渲染场景过渡效果
 */
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

/**
 * @brief 跳过当前过渡
 */
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

/**
 * @brief 清理所有未使用的场景
 */
void CSceneManager::CleanupUnusedScenes()
{
    for (auto it = m_Scenes.begin(); it != m_Scenes.end(); )
    {
        auto& scene = *it;
        
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

/**
 * @brief 保存当前场景状态
 */
BOOL CSceneManager::SaveCurrentSceneState()
{
    // 这是一个占位函数，具体实现取决于场景的设计
    // 子类可以重写此函数来实现具体的状态保存逻辑
    return TRUE;
}

/**
 * @brief 加载保存的场景状态
 */
BOOL CSceneManager::LoadSavedSceneState()
{
    // 这是一个占位函数，具体实现取决于场景的设计
    // 子类可以重写此函数来实现具体的状态加载逻辑
    return TRUE;
}