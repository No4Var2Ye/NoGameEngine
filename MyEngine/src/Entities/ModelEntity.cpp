
// ======================================================================
#include "stdafx.h"
#include "Entities/ModelEntity.h"
#include "Resources/Model.h"
// ======================================================================

CModelEntity::CModelEntity(std::shared_ptr<CModel> pModel)
    : m_pModel(pModel)
{
    // 初始化默认属性
    m_name = L"Model Entity";
    m_bDrawBBox = FALSE;
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

    // 2. 禁用光照
    glDisable(GL_LIGHTING);

    // 3. 设置纯白色，确保模型纹理正常显示
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // 2. 应用变换逻辑
    ApplyTransform();

    // 3. 真正绘制模型数据
    m_pModel->Draw();

    // FIXME:
    SetDrawBoundingBox(TRUE);

    if (m_bDrawBBox)
    {
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