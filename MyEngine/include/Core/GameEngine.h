
// ======================================================================
#ifndef __GAMEENGINE_H__
#define __GAMEENGINE_H__
// ======================================================================

#include <windows.h>
#include <memory> // 引入智能指针, 用于自动管理动态分配的对象内存.

#include "EngineConfig.h"
// ======================================================================

// ======================================================================
// 前向声明 - 告诉编译器这些是什么
class CWindow;
class CRenderer;
class CInputManager;
class CCamera;
class CResourceManager;
class CSceneManager;
class CUIManager;

// ======================================================================
class CGameEngine
/**
 * @brief 游戏引擎主类
 */
{
private:
    static CGameEngine *s_Instance; // 静态私有成员，保存唯一实例

    CGameEngine();                             // 私有构造函数，防止外部创建实例
    CGameEngine(const CGameEngine &) = delete; // 防止拷贝构造和赋值
    CGameEngine &operator=(const CGameEngine &) = delete;
    ~CGameEngine();

    // ======================================================================
    // 系统状态
    // ======================================================================
    BOOL m_Initialized = FALSE;
    BOOL m_Running = FALSE;

    enum class EngineState
    {
        FadeIn,
        Running,
        FadeOut,
        Finished
    };

    // 成员变量
    EngineState m_State = EngineState::FadeIn;
    FLOAT m_SplashTimer = 0.0f;        // 动画计时器
    const FLOAT SplashDuration = 0.0f; // 动画持续时间（秒）

    // ======================================================================
    // 引擎子系统
    // ======================================================================
    // 基础系统
    std::unique_ptr<CWindow> m_Window;
    std::unique_ptr<CRenderer> m_Renderer;

    // 逻辑系统
    std::unique_ptr<CInputManager> m_InputManager;
    std::unique_ptr<CCamera> m_pMainCamera;
    // std::unique_ptr<CResourceManager> m_ResourceManager;
    std::unique_ptr<CSceneManager> m_SceneManager;

    // UI系统
    std::unique_ptr<CUIManager> m_UIManager;

    // ======================================================================
    // TODO: 输入处理
    // ======================================================================
    // 专用的输入处理类
    void ProcessInput(FLOAT delatTime);
    // TODO: 相机控制相关
    void ProcessCameraInput(FLOAT deltaTime);
    // TODO: UI控制相关
    void ProcessUIInput(FLOAT deltaTime);

    // ======================================================================
    // 系统信息
    // ======================================================================
    // TODO: 显示调试信息
    BOOL m_ShowDebugInfo;
    void DisplayDebugInfo();
    void DisplayStatistics();

    void RenderSplashScreen(FLOAT deltaTime, BOOL isFadeOut);

public:
    static CGameEngine &GetInstance(); // 获取单例实例

    BOOL Initialize(HINSTANCE hInstance, const EngineConfig &config); // 初始化游戏引擎
    INT Run();                                                        // 运行主循环
    void Shutdown();                                                  // 关闭引擎

    // ======================================================================
    // 获取引擎子系统
    CWindow *GetWindow() const { return m_Window.get(); } // 使用 get() 返回原始指针
    CRenderer *GetRenderer() const { return m_Renderer.get(); }
    CInputManager *GetInputManager() const { return m_InputManager.get(); }
    CCamera *GetMainCamera() const { return m_pMainCamera.get(); }
    // CResourceManager *GetResourceManager() const { return m_ResourceManager.get(); }
    CUIManager *GetUIManager() const { return m_UIManager.get(); }

    // ======================================================================
    // 测试
    void TestFontRendering();
}; // class GameEngine

#endif // __GAMEENGINE_H__