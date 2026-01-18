
// ======================================================================
#ifndef __DEMO_SCENE_H__
#define __DEMO_SCENE_H__
// ======================================================================
#include <memory>
#include <vector>
#include "Scene/Scene.h"
#include "Entities/CameraEntity.h"
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
    std::shared_ptr<CCameraEntity> m_pMainCamera;
    std::shared_ptr<CModelEntity> m_pPossessedEntity;

    // 动态实体列表
    std::vector<std::shared_ptr<CEntity>> m_DynamicSnapEntities;
    void RegisterEntityForSnapping(std::shared_ptr<CEntity> pEntity, BOOL isDynamic);

    GLuint LoadSkybox();
    void SetupFog();

    float m_PossessedEntityYaw;

    // ======================================================================
    // 更新逻辑
    void UpdateLogic(float deltaTime);
    void UpdateEntities(float deltaTime);
    void UpdateAutoSnapping();
    // ======================================================================
    // 相机更新逻辑
    void UpdateFreeLookCamera(float deltaTime);
    void SyncCameraToEntity(float deltaTime);
    void SyncEntityToCamera();
    // ======================================================================
    // 按键输入控制
    void ProcessInput(float deltaTime);
    void ProcessGlobalHotkeys(float deltaTime);
    void ProcessCameraInput(float deltaTime);

    void ResetCameraToDefault();
    void SwitchCameraMode(CameraMode newMode);
    void SetupFirstPersonCamera();
    void SetupThirdPersonCamera();
    void SetupOrbitalCamera();
    float m_OrbitalDistance;
    float m_OrbitalHeight;
    float m_OrbitalAngle;
    BOOL m_bOrbitEnabled;

    float m_LastCameraAngle; // 记录上一次的相机角度
    float m_LastDuckYaw;     // 记录被控实体的当前偏航角
    void SetupFreeLookCamera(const Vector3 &worldPos, const Quaternion &worldRot);
};

#endif // __DEMO_SCENE_H__void CDemoScene::SwitchCameraMode(CameraMode newMode)