
// ======================================================================
#ifndef __MODEL_ENTITY_H__
#define __MODEL_ENTITY_H__
// ======================================================================
#include <Windows.h>
#include "Core/Entity.h"
// ======================================================================
class CModel; // 前向声明
// ======================================================================

class CModelEntity : public CEntity
{
public:
    virtual ~CModelEntity() = default;

    template <typename... Args>
    static std::shared_ptr<CModelEntity> Create(Args &&...args)
    {
        auto entity = std::shared_ptr<CModelEntity>(new CModelEntity(std::forward<Args>(args)...));
        entity->m_uID = ++s_nextID;
        return entity;
    }

    // 实现基类的核心接口
    virtual void Update(float deltaTime) override;
    virtual void Render() override;

    // 模型特有操作
    void SetModel(std::shared_ptr<CModel> pModel) { m_pModel = pModel; }
    std::shared_ptr<CModel> GetModel() const { return m_pModel; }

    // ======================================================================
    // 包围盒
    void SetDrawBoundingBox(BOOL bDraw) { m_bDrawBBox = bDraw; }

    // ======================================================================
    // 法线
    // 开启/关闭法线绘制
    void SetDrawNormals(BOOL bDraw) { m_bDrawNormals = bDraw; }
    BOOL IsDrawNormals() const { return m_bDrawNormals; }
    // 设置法线长度缩放
    void SetNormalScale(float scale) { m_fNormalScale = scale; }
    void SetNormalStep(unsigned int step) { m_uNormalStep = step; }

protected:
    // 构造函数受保护，强制使用 Create
    CModelEntity(std::shared_ptr<CModel> pModel);

private:
    std::shared_ptr<CModel> m_pModel; // 引用模型资源

    BOOL m_bDrawBBox = FALSE;

    BOOL m_bDrawNormals = FALSE; // 是否绘制法线开关
    float m_fNormalScale = 0.5f; // 法线显示长度
    unsigned int m_uNormalStep = 1; // 法线步长
};

#endif // __MODEL_ENTITY_H__