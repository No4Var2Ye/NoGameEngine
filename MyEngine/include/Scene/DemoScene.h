
// ======================================================================
#ifndef __DEMO_SCENE_H__
#define __DEMO_SCENE_H__
// ======================================================================
#include <memory>
#include <vector>
#include "Scene/Scene.h"
// ======================================================================
class CRenderer;
class CSkyboxEntity;
class CTerrainEntity;
class CGridEntity;
class CModelEntity;
// ======================================================================

class CDemoScene : public CScene
{
public:
    CDemoScene();
    virtual ~CDemoScene() = default;

    virtual BOOL Initialize() override;
    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    virtual void Shutdown() override;

    void SetupGlobalLighting();
    void CleanupTextureState();

private:
    // 将关键实体存为成员，避免每帧 FindChild
    std::shared_ptr<CSkyboxEntity> m_pSkybox;
    std::shared_ptr<CTerrainEntity> m_pTerrain;
    std::shared_ptr<CGridEntity> m_pGrid;
    std::shared_ptr<CModelEntity> m_pPossessedEntity;

    // 动态实体列表
    std::vector<std::shared_ptr<CEntity>> m_DynamicSnapEntities;
    void RegisterEntityForSnapping(std::shared_ptr<CEntity> pEntity, BOOL isDynamic);

    GLuint LoadSkybox();
    void SetupFog();

    float m_PossessedEntityYaw;

    // 更新逻辑
    void ProcessInput(float deltaTime);
    void UpdateLogic(float deltaTime);
    void UpdateEntities(float deltaTime);
    void UpdateAutoSnapping();
};

#endif // __DEMO_SCENE_H__