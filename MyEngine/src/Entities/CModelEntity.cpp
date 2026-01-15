
// ======================================================================
#include "stdafx.h"
#include "Entities/CModelEntity.h"
#include "Resources/Model.h"
// ======================================================================

std::shared_ptr<CModelEntity> CModelEntity::Create(std::shared_ptr<CModel> pModel)
{
    // 使用 struct 辅助技巧或直接 new（因为构造函数在 protected 作用域内）
    return std::shared_ptr<CModelEntity>(new CModelEntity(pModel));
}

CModelEntity::CModelEntity(std::shared_ptr<CModel> pModel)
    : m_pModel(pModel)
{
    SetName(L"Model Entity");
}

void CModelEntity::Update(FLOAT deltaTime)
{
    // 调用基类 Update 以确保子节点的递归更新
    CEntity::Update(deltaTime);

    // 此处可添加模型特有逻辑，例如骨骼动画更新等
}

void CModelEntity::Render()
{
    if (!m_bVisible || !m_pModel)
        return;

    // 1. 保存当前 OpenGL 矩阵状态
    glPushMatrix();

    // 2. 应用变换逻辑（基类计算的世界矩阵）
    ApplyTransform();

    // 3. 真正绘制模型数据
    m_pModel->Draw();

    if(m_bDrawBBox){
        m_pModel->DrawBoundingBox();
    }
    
    // 4. 恢复矩阵状态
    glPopMatrix();
    
    // 5. 递归渲染子节点
    for (auto &pChild : m_children)
    {
        if (pChild)
            pChild->Render();
    }
}