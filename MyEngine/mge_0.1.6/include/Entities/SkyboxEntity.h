
// ======================================================================
#ifndef __SKYBOX_ENTITY_H__
#define __SKYBOX_ENTITY_H__
// ======================================================================
#include "Core/Entity.h"
#include "Graphics/Camera/Camera.h"
// ======================================================================

class CSkyboxEntity : public CEntity
{
private:
    static unsigned int s_nextID; // 静态ID计数器
public:
    virtual ~CSkyboxEntity() = default;

    // 静态创建工厂，保持与你的 CModelEntity 风格一致
    template <typename... Args>
    static std::shared_ptr<CSkyboxEntity> Create(Args &&...args)
    {
        auto entity = std::shared_ptr<CSkyboxEntity>(new CSkyboxEntity(std::forward<Args>(args)...));
        entity->m_uID = ++s_nextID;
        return entity;
    }

    // 设置立方体贴图 ID (通常从 ResourceManager 获取)
    void SetCubemapTexture(GLuint textureID) { m_uCubemapID = textureID; }
    void SetSize(FLOAT size) { m_fSize = size; }
    void EnableRotation(BOOL enable) { m_bEnableRotation = enable; }
    void SetRotationSpeed(FLOAT speed) { m_fRotationSpeed = speed; }

    virtual void Update(float deltaTime) override;
    virtual void Render() override;

protected:
    CSkyboxEntity(GLuint textureID = 0);

private:
    GLuint m_uCubemapID;
    FLOAT m_fSize;
    BOOL m_bEnableRotation;
    FLOAT m_fRotationSpeed;
    FLOAT m_fCurrentRotation;

    void DrawCube(); // 内部辅助绘制 1x1x1 立方体
};

#endif // __SKYBOX_ENTITY_H__