
// ======================================================================
#ifndef __SCENE_H__
#define __SCENE_H__
// ======================================================================
#include "Windows.h"
#include <string>
#include <vector>
#include <memory>
#include "Core/Entity.h"
// ======================================================================

class CScene
{
protected:
    std::string m_Name;          // 场景名称
    BOOL m_bInitialized = FALSE; // 是否已初始化
    BOOL m_bIsActive = FALSE;    // 是否激活状态
    BOOL m_bIsPaused = FALSE;    // 是否暂停状态

    std::shared_ptr<CEntity> m_pRootEntity; // 根实体

public:
    CScene(const std::string &name) : m_Name(name) {}
    virtual ~CScene() = default;

    // 禁止拷贝
    CScene(const CScene &) = delete;
    CScene &operator=(const CScene &) = delete;

    const std::string &GetName() const { return m_Name; } // 获取场景名称

    BOOL IsInitialized() const { return m_bInitialized; } // 检查场景是否已初始化
    BOOL IsActive() const { return m_bIsActive; }         // 检查场景是否激活
    BOOL IsPaused() const { return m_bIsPaused; }         // 检查场景是否暂停

    // ======================================================================
    // 生命周期方法
    // ======================================================================
    virtual BOOL Initialize() = 0; // 初始化场景
    virtual void Shutdown() = 0;   // 关闭场景

    virtual void Render()
    {
        if (m_pRootEntity)
            m_pRootEntity->Render();
    }

    virtual void Update(float deltaTime)
    {
        if (m_pRootEntity && !m_bIsPaused)
            m_pRootEntity->Update(deltaTime);
    }

    virtual void OnActivate() {}   // 场景激活时调用
    virtual void OnDeactivate() {} // 场景失活时调用
    virtual void OnPause() {}      // 场景暂停时调用
    virtual void OnResume() {}     // 场景恢复时调用

    virtual void ProcessInput(FLOAT deltaTime) {} // 处理输入
};

#endif // __SCENE_H__