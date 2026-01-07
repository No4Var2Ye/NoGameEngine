#ifndef __SCENEMANAGER_H__
#define __SCENEMANAGER_H__

#include <memory>
#include <vector>
#include <string>
#include <functional>

/**
 * @brief 场景基类
 * @details 所有游戏场景必须继承自此类
 */
class CScene
{
protected:
    std::string m_Name;              // 场景名称
    BOOL m_Initialized = FALSE;      // 是否已初始化
    BOOL m_Active = FALSE;           // 是否激活状态
    BOOL m_Paused = FALSE;           // 是否暂停状态

public:
    CScene(const std::string& name) : m_Name(name) {}
    virtual ~CScene() = default;
    
    // 禁止拷贝
    CScene(const CScene&) = delete;
    CScene& operator=(const CScene&) = delete;
    
    /**
     * @brief 获取场景名称
     * @return 场景名称
     */
    const std::string& GetName() const { return m_Name; }
    
    /**
     * @brief 检查场景是否已初始化
     * @return 已初始化返回TRUE，否则返回FALSE
     */
    BOOL IsInitialized() const { return m_Initialized; }
    
    /**
     * @brief 检查场景是否激活
     * @return 激活返回TRUE，否则返回FALSE
     */
    BOOL IsActive() const { return m_Active; }
    
    /**
     * @brief 检查场景是否暂停
     * @return 暂停返回TRUE，否则返回FALSE
     */
    BOOL IsPaused() const { return m_Paused; }
    
    // 生命周期方法
    
    /**
     * @brief 初始化场景
     * @return 初始化成功返回TRUE，失败返回FALSE
     */
    virtual BOOL Initialize() = 0;
    
    /**
     * @brief 关闭场景
     */
    virtual void Shutdown() = 0;
    
    /**
     * @brief 更新场景逻辑
     * @param deltaTime 帧时间
     */
    virtual void Update(FLOAT deltaTime) = 0;
    
    /**
     * @brief 渲染场景
     */
    virtual void Render() = 0;
    
    /**
     * @brief 场景激活时调用
     */
    virtual void OnActivate() {}
    
    /**
     * @brief 场景失活时调用
     */
    virtual void OnDeactivate() {}
    
    /**
     * @brief 场景暂停时调用
     */
    virtual void OnPause() {}
    
    /**
     * @brief 场景恢复时调用
     */
    virtual void OnResume() {}
    
    /**
     * @brief 处理输入
     * @param deltaTime 帧时间
     */
    virtual void ProcessInput(FLOAT deltaTime) {}
    
    /**
     * @brief 重新加载场景资源
     */
    virtual void Reload() {}
};

/**
 * @brief 场景管理器类
 * @details 管理游戏场景的切换、更新和渲染
 */
class CSceneManager
{
private:
    std::vector<std::shared_ptr<CScene>> m_Scenes;      // 所有场景
    std::shared_ptr<CScene> m_CurrentScene;             // 当前场景
    std::shared_ptr<CScene> m_NextScene;                // 下一个场景
    
    BOOL m_Initialized = FALSE;                         // 是否已初始化
    BOOL m_SceneChangePending = FALSE;                  // 是否有场景切换请求
    BOOL m_Paused = FALSE;                              // 管理器是否暂停
    BOOL m_UpdateEnabled = TRUE;                        // 是否启用更新
    BOOL m_RenderEnabled = TRUE;                        // 是否启用渲染
    
    // 场景过渡效果
    enum class TransitionState
    {
        None,       // 无过渡
        FadeOut,    // 淡出
        FadeIn,     // 淡入
        Waiting     // 等待
    };
    
    TransitionState m_TransitionState = TransitionState::None;
    FLOAT m_TransitionAlpha = 0.0f;                     // 过渡透明度
    FLOAT m_TransitionSpeed = 2.0f;                     // 过渡速度
    DWORD m_TransitionStartTime = 0;                    // 过渡开始时间
    DWORD m_TransitionWaitTime = 500;                   // 过渡等待时间（毫秒）
    
    /**
     * @brief 执行场景切换
     */
    void PerformSceneChange();
    
    /**
     * @brief 更新场景过渡效果
     * @param deltaTime 帧时间
     */
    void UpdateTransition(FLOAT deltaTime);
    
    /**
     * @brief 渲染场景过渡效果
     */
    void RenderTransition();
    
public:
    CSceneManager();
    ~CSceneManager();
    
    // 禁止拷贝
    CSceneManager(const CSceneManager&) = delete;
    CSceneManager& operator=(const CSceneManager&) = delete;
    
    /**
     * @brief 初始化场景管理器
     * @return 初始化成功返回TRUE，失败返回FALSE
     */
    BOOL Initialize();
    
    /**
     * @brief 关闭场景管理器
     */
    void Shutdown();
    
    // 场景管理
    
    /**
     * @brief 注册场景
     * @param scene 场景指针
     * @return 注册成功返回TRUE，失败返回FALSE
     */
    BOOL RegisterScene(std::shared_ptr<CScene> scene);
    
    /**
     * @brief 取消注册场景
     * @param sceneName 场景名称
     * @return 取消成功返回TRUE，失败返回FALSE
     */
    BOOL UnregisterScene(const std::string& sceneName);
    
    /**
     * @brief 获取场景
     * @param sceneName 场景名称
     * @return 场景指针，找不到返回nullptr
     */
    std::shared_ptr<CScene> GetScene(const std::string& sceneName) const;
    
    /**
     * @brief 获取当前场景
     * @return 当前场景指针
     */
    std::shared_ptr<CScene> GetCurrentScene() const { return m_CurrentScene; }
    
    /**
     * @brief 获取场景数量
     * @return 场景数量
     */
    size_t GetSceneCount() const { return m_Scenes.size(); }
    
    /**
     * @brief 获取所有场景名称
     * @param names 输出场景名称列表
     */
    void GetAllSceneNames(std::vector<std::string>& names) const;
    
    // 场景切换
    
    /**
     * @brief 切换到指定场景
     * @param sceneName 场景名称
     * @param withTransition 是否使用过渡效果
     * @return 切换成功返回TRUE，失败返回FALSE
     */
    BOOL ChangeScene(const std::string& sceneName, BOOL withTransition = TRUE);
    
    /**
     * @brief 直接切换到指定场景（无过渡效果）
     * @param sceneName 场景名称
     * @return 切换成功返回TRUE，失败返回FALSE
     */
    BOOL ChangeSceneImmediate(const std::string& sceneName);
    
    /**
     * @brief 切换到下一个场景
     * @param withTransition 是否使用过渡效果
     * @return 切换成功返回TRUE，失败返回FALSE
     */
    BOOL ChangeToNextScene(BOOL withTransition = TRUE);
    
    /**
     * @brief 切换到上一个场景
     * @param withTransition 是否使用过渡效果
     * @return 切换成功返回TRUE，失败返回FALSE
     */
    BOOL ChangeToPreviousScene(BOOL withTransition = TRUE);
    
    /**
     * @brief 重启当前场景
     * @param withTransition 是否使用过渡效果
     */
    void RestartCurrentScene(BOOL withTransition = TRUE);
    
    /**
     * @brief 检查是否正在切换场景
     * @return 正在切换返回TRUE，否则返回FALSE
     */
    BOOL IsChangingScene() const { return m_SceneChangePending || m_TransitionState != TransitionState::None; }
    
    // 场景控制
    
    /**
     * @brief 暂停当前场景
     */
    void PauseCurrentScene();
    
    /**
     * @brief 恢复当前场景
     */
    void ResumeCurrentScene();
    
    /**
     * @brief 重新加载当前场景
     */
    void ReloadCurrentScene();
    
    /**
     * @brief 更新所有场景
     * @param deltaTime 帧时间
     */
    void Update(FLOAT deltaTime);
    
    /**
     * @brief 渲染所有场景
     */
    void Render();
    
    // 管理器控制
    
    /**
     * @brief 暂停场景管理器
     */
    void Pause() { m_Paused = TRUE; }
    
    /**
     * @brief 恢复场景管理器
     */
    void Resume() { m_Paused = FALSE; }
    
    /**
     * @brief 检查管理器是否暂停
     * @return 暂停返回TRUE，否则返回FALSE
     */
    BOOL IsPaused() const { return m_Paused; }
    
    /**
     * @brief 启用更新
     */
    void EnableUpdate() { m_UpdateEnabled = TRUE; }
    
    /**
     * @brief 禁用更新
     */
    void DisableUpdate() { m_UpdateEnabled = FALSE; }
    
    /**
     * @brief 检查更新是否启用
     * @return 启用返回TRUE，否则返回FALSE
     */
    BOOL IsUpdateEnabled() const { return m_UpdateEnabled; }
    
    /**
     * @brief 启用渲染
     */
    void EnableRender() { m_RenderEnabled = TRUE; }
    
    /**
     * @brief 禁用渲染
     */
    void DisableRender() { m_RenderEnabled = FALSE; }
    
    /**
     * @brief 检查渲染是否启用
     * @return 启用返回TRUE，否则返回FALSE
     */
    BOOL IsRenderEnabled() const { return m_RenderEnabled; }
    
    // 过渡效果控制
    
    /**
     * @brief 设置过渡速度
     * @param speed 过渡速度
     */
    void SetTransitionSpeed(FLOAT speed) { m_TransitionSpeed = speed; }
    
    /**
     * @brief 获取过渡速度
     * @return 过渡速度
     */
    FLOAT GetTransitionSpeed() const { return m_TransitionSpeed; }
    
    /**
     * @brief 设置过渡等待时间
     * @param waitTime 等待时间（毫秒）
     */
    void SetTransitionWaitTime(DWORD waitTime) { m_TransitionWaitTime = waitTime; }
    
    /**
     * @brief 获取过渡等待时间
     * @return 等待时间
     */
    DWORD GetTransitionWaitTime() const { return m_TransitionWaitTime; }
    
    /**
     * @brief 检查是否正在过渡
     * @return 正在过渡返回TRUE，否则返回FALSE
     */
    BOOL IsInTransition() const { return m_TransitionState != TransitionState::None; }
    
    /**
     * @brief 跳过当前过渡
     */
    void SkipTransition();
    
    // 实用功能
    
    /**
     * @brief 清理所有未使用的场景
     */
    void CleanupUnusedScenes();
    
    /**
     * @brief 保存当前场景状态
     * @return 保存成功返回TRUE，失败返回FALSE
     */
    BOOL SaveCurrentSceneState();
    
    /**
     * @brief 加载保存的场景状态
     * @return 加载成功返回TRUE，失败返回FALSE
     */
    BOOL LoadSavedSceneState();
    void DrawTestTriangle();
};
#endif // __SCENEMANAGER_H__