#ifndef __GAMEENGINE_H__
#define __GAMEENGINE_H__

#include <windows.h>
#include <memory>  // 引入智能指针, 用于自动管理动态分配的对象内存.

#include "EngineConfig.h"


// 前向声明 - 告诉编译器这些是什么
class CWindow;
class CRenderer;
class CInputManager;
class CResourceManager;
class CSceneManager;


class CGameEngine
/**
 * @brief 游戏引擎主类
 */
{
private:
    static CGameEngine *s_Instance;  // 静态私有成员，保存唯一实例

    CGameEngine();  // 私有构造函数，防止外部创建实例
    CGameEngine(const CGameEngine&) = delete;  // 防止拷贝构造和赋值
    CGameEngine& operator=(const CGameEngine&) = delete;
    ~CGameEngine();

    BOOL m_Initialized = false;
    BOOL m_Running = false;

    // 引擎子系统
    std::unique_ptr<CWindow> m_Window;  // m_ 成员变量
    std::unique_ptr<CRenderer> m_Renderer;
    std::unique_ptr<CInputManager> m_InputManager;
    // std::unique_ptr<CResourceManager> m_ResourceManager;
    std::unique_ptr<CSceneManager> m_SceneManager;

    // TODO: 输入处理
    // 或者创建专用的输入处理类
    void ProcessInput(float delatTime);

    // TODO: 相机控制相关
    void ProcessCameraInput(float deltaTime);
    void ProcessUInput(float deltaTime);

public:
    static CGameEngine &GetInstance();  // @brief 获取单例实例

    BOOL Initialize(HINSTANCE hInstance, const EngineConfig &config);  // 初始化游戏引擎
    INT Run();  // 运行主循环
    void Shutdown();  // 关闭引擎

    // 获取引擎子系统
    CWindow *GetWindow() const { return m_Window.get(); }  // 使用 get() 返回原始指针
    CRenderer *GetRenderer() const { return m_Renderer.get(); }
    CInputManager *GetInputManager() const { return m_InputManager.get(); }
}; // class GameEngine
#endif // __GAMEENGINE_H__