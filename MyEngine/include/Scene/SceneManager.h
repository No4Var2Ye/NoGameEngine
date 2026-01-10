
// ======================================================================
#ifndef __SCENEMANAGER_H__
#define __SCENEMANAGER_H__

// ======================================================================
#include <memory>
#include <vector>
#include <string>
#include <functional>
// ======================================================================


// ======================================================================
/**
 * @brief 场景基类
 * @details 所有游戏场景必须继承自此类
 */
class CScene
{
protected:
    std::string m_Name;         // 场景名称
    BOOL m_Initialized = FALSE; // 是否已初始化
    BOOL m_Active = FALSE;      // 是否激活状态
    BOOL m_Paused = FALSE;      // 是否暂停状态

public:
    CScene(const std::string &name) : m_Name(name) {}
    virtual ~CScene() = default;

    // 禁止拷贝
    CScene(const CScene &) = delete;
    CScene &operator=(const CScene &) = delete;

    const std::string &GetName() const { return m_Name; } // 获取场景名称

    BOOL IsInitialized() const { return m_Initialized; } // 检查场景是否已初始化
    BOOL IsActive() const { return m_Active; }           // 检查场景是否激活
    BOOL IsPaused() const { return m_Paused; }           // 检查场景是否暂停

    // ======================================================================
    // 生命周期方法
    // ======================================================================
    virtual BOOL Initialize() = 0;            // 初始化场景
    virtual void Shutdown() = 0;              // 关闭场景
    virtual void Render() = 0;                // 渲染场景
    virtual void Update(FLOAT deltaTime) = 0; // 更新场景逻辑

    virtual void OnActivate() {}   // 场景激活时调用
    virtual void OnDeactivate() {} // 场景失活时调用
    virtual void OnPause() {}      // 场景暂停时调用
    virtual void OnResume() {}     // 场景恢复时调用

    virtual void ProcessInput(FLOAT deltaTime) {} // 处理输入

    virtual void Reload() {} // 重新加载场景资源
};

// ======================================================================
/**
 * @brief 场景管理器类
 * @details 管理游戏场景的切换、更新和渲染
 */
class CSceneManager
{
private:
    std::vector<std::shared_ptr<CScene>> m_Scenes; // 所有场景
    std::shared_ptr<CScene> m_CurrentScene;        // 当前场景
    std::shared_ptr<CScene> m_NextScene;           // 下一个场景

    BOOL m_Initialized = FALSE;        // 是否已初始化
    BOOL m_SceneChangePending = FALSE; // 是否有场景切换请求
    BOOL m_Paused = FALSE;             // 管理器是否暂停
    BOOL m_UpdateEnabled = TRUE;       // 是否启用更新
    BOOL m_RenderEnabled = TRUE;       // 是否启用渲染

    // 场景过渡效果
    enum class TransitionState
    {
        None,    // 无过渡
        FadeOut, // 淡出
        FadeIn,  // 淡入
        Waiting  // 等待
    };

    TransitionState m_TransitionState = TransitionState::None;
    FLOAT m_TransitionAlpha = 0.0f;   // 过渡透明度
    FLOAT m_TransitionSpeed = 2.0f;   // 过渡速度
    DWORD m_TransitionStartTime = 0;  // 过渡开始时间
    DWORD m_TransitionWaitTime = 500; // 过渡等待时间（毫秒）

    void PerformSceneChange(); // 执行场景切换
    void UpdateTransition(FLOAT deltaTime); // 更新场景过渡效果
    void RenderTransition(); // 渲染场景过渡效果

public:
    CSceneManager();
    ~CSceneManager();

    // 禁止拷贝
    CSceneManager(const CSceneManager &) = delete;
    CSceneManager &operator=(const CSceneManager &) = delete;

    BOOL Initialize(); // 初始化场景管理器
    void Shutdown(); // 关闭场景管理器

    // ======================================================================
    // 场景管理
    // ======================================================================

    // 注册场景, 传入场景指针
    BOOL RegisterScene(std::shared_ptr<CScene> scene);

    // 取消注册场景 传入场景名称
    BOOL UnregisterScene(const std::string &sceneName);

    /**
     * @brief 获取场景
     * @param sceneName 场景名称
     * @return 场景指针，找不到返回nullptr
     */
    std::shared_ptr<CScene> GetScene(const std::string &sceneName) const;

    // 获取当前场景
    std::shared_ptr<CScene> GetCurrentScene() const { return m_CurrentScene; }

    // 获取场景数量
    size_t GetSceneCount() const { return m_Scenes.size(); }

    // 获取所有场景名称
    void GetAllSceneNames(std::vector<std::string> &names) const;

    // ======================================================================
    // 场景切换
    // ======================================================================

    /**
     * @brief 切换到指定场景
     * @param sceneName 场景名称
     * @param withTransition 是否使用过渡效果
     * @return 切换成功返回TRUE，失败返回FALSE
     */
    BOOL ChangeScene(const std::string &sceneName, BOOL withTransition = TRUE);

    /**
     * @brief 直接切换到指定场景（无过渡效果）
     * @param sceneName 场景名称
     * @return 切换成功返回TRUE，失败返回FALSE
     */
    BOOL ChangeSceneImmediate(const std::string &sceneName);

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

    // ======================================================================
    // 场景控制
    // ======================================================================

    void PauseCurrentScene();     // 暂停当前场景
    void ResumeCurrentScene();    // 恢复当前场景
    void ReloadCurrentScene();    // 重新加载当前场景
    void Update(FLOAT deltaTime); // 更新所有场景
    void Render();                // 渲染所有场景

    // ======================================================================
    // 管理器控制
    // ======================================================================

    void Pause() { m_Paused = TRUE; }          // 暂停场景管理器
    void Resume() { m_Paused = FALSE; }        // 恢复场景管理器
    BOOL IsPaused() const { return m_Paused; } // 检查管理器是否暂停

    void EnableUpdate() { m_UpdateEnabled = TRUE; }          // 启用更新
    void DisableUpdate() { m_UpdateEnabled = FALSE; }        // 禁用更新
    BOOL IsUpdateEnabled() const { return m_UpdateEnabled; } // 检查更新是否启用

    void EnableRender() { m_RenderEnabled = TRUE; }          // 启用渲染
    void DisableRender() { m_RenderEnabled = FALSE; }        // 禁用渲染
    BOOL IsRenderEnabled() const { return m_RenderEnabled; } // 检查渲染是否启用

    // ======================================================================
    // 过渡效果控制
    // ======================================================================

    void SetTransitionSpeed(FLOAT speed) { m_TransitionSpeed = speed; }                // 设置过渡速度
    FLOAT GetTransitionSpeed() const { return m_TransitionSpeed; }                     // 获取过渡速度
    void SetTransitionWaitTime(DWORD waitTime) { m_TransitionWaitTime = waitTime; }    // 设置过渡等待时间
    DWORD GetTransitionWaitTime() const { return m_TransitionWaitTime; }               // 获取过渡等待时间
    BOOL IsInTransition() const { return m_TransitionState != TransitionState::None; } // 检查是否正在过渡
    void SkipTransition();                                                             // 跳过当前过渡

    // ======================================================================
    // 实用功能
    // ======================================================================

    void CleanupUnusedScenes();   // 清理所有未使用的场景
    BOOL SaveCurrentSceneState(); // 保存当前场景状态
    BOOL LoadSavedSceneState();   // 加载保存的场景状态
};

#endif // __SCENEMANAGER_H__