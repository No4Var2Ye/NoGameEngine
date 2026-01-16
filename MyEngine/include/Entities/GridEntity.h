// ======================================================================
#ifndef __GRID_ENTITY_H__
#define __GRID_ENTITY_H__
// ======================================================================
#include "Core/Entity.h"
#include "Math/Vector3.h"
// ======================================================================

class CGridEntity : public CEntity
{
private:
    static unsigned int s_nextID;

public:
    virtual ~CGridEntity() = default;

    // 创建网格实体
    template <typename... Args>
    static std::shared_ptr<CGridEntity> Create(Args &&...args)
    {
        auto entity = std::shared_ptr<CGridEntity>(new CGridEntity(std::forward<Args>(args)...));
        entity->m_uID = ++s_nextID;
        return entity;
    }

    // 属性设置
    void SetSize(FLOAT size) { m_fSize = size; }
    void SetStep(FLOAT step) { m_fStep = step; }
    void SetGridColor(const Vector3 &mainColor, const Vector3 &subColor)
    {
        m_MainColor = mainColor;
        m_SubColor = subColor;
    }

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    void DrawFadingLine(float coord, BOOL isParallelToZ, const Vector3& camPos, const Vector3& color, float maxDist);


protected:
    CGridEntity(FLOAT size = 100.0f, FLOAT step = 1.0f);

private:
    FLOAT m_fSize;
    FLOAT m_fStep;
    Vector3 m_MainColor; // 轴线颜色
    Vector3 m_SubColor;  // 普通网格颜色
};

#endif // __GRID_ENTITY_H__