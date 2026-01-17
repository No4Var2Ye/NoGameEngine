
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

    // TODO: 此处可添加模型特有逻辑，例如骨骼动画更新等
}

void CModelEntity::Render()
{
    if (!m_bVisible || !m_pModel)
        return;
        
    // 1. 保存当前 OpenGL 矩阵状态
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushMatrix();

    // 2. 禁用光照
    glDisable(GL_LIGHTING);

    // 3. 设置纯白色，确保模型纹理正常显示
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // 2. 应用变换逻辑
    ApplyTransform();

    GLint currentTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
    // LogDebug(L"模型渲染前绑定的纹理: %d.\n", currentTexture);

    // 3. 真正绘制模型数据
    m_pModel->Draw();

    // 检查渲染后的纹理状态
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
    // LogDebug(L"渲染后绑定的纹理: %d.\n", currentTexture);

    // 4. 绘制包围盒
    if (m_bDrawBBox) {
        m_pModel->DrawBoundingBox(); 
    }

    // 5. 绘制法线
    if (m_bDrawNormals) {
        m_pModel->DrawNormals(m_fNormalScale, m_uNormalStep);
    }

    // 4. 恢复矩阵状态
    glPopMatrix();
    glPopAttrib();


    // 5. 递归渲染子节点
    for (auto &pChild : m_children)
    {
        if (pChild)
            pChild->Render();
    }
}