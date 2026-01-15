
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
    static std::shared_ptr<CModelEntity> Create(std::shared_ptr<CModel> pModel);

    virtual ~CModelEntity() = default;

    // 实现基类的核心接口
    virtual void Update(FLOAT deltaTime) override;
    virtual void Render() override;

    // 模型特有操作
    void SetModel(std::shared_ptr<CModel> pModel) { m_pModel = pModel; }
    std::shared_ptr<CModel> GetModel() const { return m_pModel; }

    void SetDrawBoundingBox(BOOL bDraw) { m_bDrawBBox = bDraw; }

protected:
    // 构造函数受保护，强制使用 Create
    CModelEntity(std::shared_ptr<CModel> pModel);

private:
    std::shared_ptr<CModel> m_pModel; // 引用模型资源

    BOOL m_bDrawBBox = FALSE;
};

#endif // __MODEL_ENTITY_H__