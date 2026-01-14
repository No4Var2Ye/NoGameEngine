
// ======================================================================
#include "stdafx.h"
#include "Resources/Entity.h"
#include "Resources/Model.h"
// ======================================================================

CEntity::CEntity(std::shared_ptr<CModel> pModel) : m_pModel(pModel)
{
    m_position = Vector3(0, 0, 0);
    m_rotation = Vector3(0, 0, 0);
    m_scale = Vector3(1, 1, 1);
}

void CEntity::Update(float deltaTime)
{
    // 在这里处理旋转动画或逻辑移动
}

void CEntity::Render()
{
    if (!m_pModel)
        return;

    glPushMatrix();
    // 1. 合成你的 Matrix4 变换矩阵 (T * R * S)
    // 利用你之前设计的 Matrix4 静态方法
    Matrix4 transform = Matrix4::Translation(m_position) * Matrix4::RotationY(m_rotation.y * Math::DEG_TO_RAD) * // 示例
                        Matrix4::Scale(m_scale);

    // 2. 将你的列主序矩阵传给 OpenGL 固定管线
    glMultMatrixf(transform.m);

    // 3. 执行渲染
    m_pModel->Draw();
    glPopMatrix();
}