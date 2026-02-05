
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

struct SDuckInstance
{
    std::shared_ptr<CModelEntity> pEntity;
    bool isPickedUp = false;
    float collisionRadius = 1.5f;
};

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
    std::shared_ptr<CEntity> m_pEffectContainer;

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

    std::vector<SDuckInstance> m_TargetDucks;
    int m_Score = 0;
    const int WIN_SCORE = 9;
    bool m_bGameOver = false;

    void ResetGame();

    static const float UI_BG_COLOR[4];
    static const float UI_BORDER_COLOR[4];
    static const float UI_TEXT_COLOR[3];

    void RenderUI();
    void DrawUIRect(float x, float y, float w, float h, const float *bgColor, const float *borderColor);
};

#endif // __DEMO_SCENE_H__