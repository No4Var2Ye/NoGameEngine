
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

    // 2. 处理动态动画
    if (m_pModel && m_bIsAnimated)
    {
        // 累加时间
        m_fAnimTime += deltaTime * m_fAnimSpeed;

        // 调用你之前在 CModel 里写的更新函数
        // 这会触发 CPU 蒙皮计算，更新顶点坐标
        m_pModel->UpdateAnimation(m_fAnimTime);
    }
}

void CModelEntity::Render()
{
    if (!m_bVisible || !m_pModel)
        return;

    // 1. 保存当前 OpenGL 矩阵状态
    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT | GL_LINE_BIT);

    glPushMatrix();

    // 2. 应用变换逻辑
    ApplyTransform();

    // 3. 处理线框模式
    if (m_bWireframe)
    {
        // 切换为线框模式
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // 线框模式下通常关闭光照和纹理，让结构更清晰
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        // 设置线框颜色
        glColor3f(0.0f, 0.0f, 1.0f);
        glLineWidth(0.1f);
    }
    else
    {
        // 正常填充模式
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
        // 如果模型有纹理，可以在这里由 Model 类内部处理绑定
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // 3. 真正绘制模型数据
    m_pModel->Draw();

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // 4. 绘制包围盒
    if (m_bDrawBBox)
    {
        m_pModel->DrawBoundingBox();
    }

    // 5. 绘制法线
    if (m_bDrawNormals)
    {
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