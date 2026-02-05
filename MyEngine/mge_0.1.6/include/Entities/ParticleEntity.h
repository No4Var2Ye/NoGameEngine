#ifndef __PARTICLE_ENTITY_H__
#define __PARTICLE_ENTITY_H__

#include "Core/Entity.h"

struct Particle
{
    Vector3 position;
    Vector3 velocity;
    float length;
    BOOL active;
};

class CParticleEntity : public CEntity
{
public:
    virtual ~CParticleEntity() = default;

    static std::shared_ptr<CParticleEntity> Create(int maxParticles = 1000)
    {
        auto entity = std::shared_ptr<CParticleEntity>(new CParticleEntity(maxParticles));
        entity->m_uID = ++s_nextID;
        return entity;
    }

    virtual void Update(float deltaTime) override;
    virtual void Render() override;

    // 设置雨的范围（通常随相机移动）
    void SetEmitterRange(float range) { m_Range = range; }

protected:
    CParticleEntity(int maxParticles);

private:
    std::vector<Particle> m_particles;
    float m_Range = 20.0f;     // 雨滴生成的水平范围
    float m_Height = 15.0f;    // 生成高度
    float m_FallSpeed = 25.0f; // 下落速度
};

#endif