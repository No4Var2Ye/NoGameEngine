
// ======================================================================
#include "stdafx.h"
#include <cfloat>
#include "Resources/Mesh.h"
#include "Resources/Texture.h"
// ======================================================================

CMesh::CMesh(const std::vector<Vertex> &vertices,
             const std::vector<unsigned int> &indices,
             std::shared_ptr<CTexture> pTexture)
    : m_vertices(vertices), m_indices(indices), m_pTexture(pTexture)
{
    if (m_vertices.empty())
    {
        throw std::runtime_error("Mesh has no vertices");
    }
    if (m_indices.size() % 3 != 0)
    {
        throw std::runtime_error("Indices count must be multiple of 3");
    }

    CalculateBoundingBox();
}

CMesh::~CMesh()
{
    // vector 会自动析构，shared_ptr 会自动减引用
}

void CMesh::Draw() const
{
    if (m_vertices.empty() || m_indices.empty())
        return;

    // 保存当前OpenGL状态
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT);

    // 0. 应用材质
    glMaterialfv(GL_FRONT, GL_AMBIENT, m_material.ambient.GetData());
    glMaterialfv(GL_FRONT, GL_DIFFUSE, m_material.diffuse.GetData());
    glMaterialfv(GL_FRONT, GL_SPECULAR, m_material.specular.GetData());
    glMaterialf(GL_FRONT, GL_SHININESS, m_material.shininess);

    // 0.1 处理透明度（如果需要）
    if (m_material.opacity < 1.0f)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 设置材质透明度
        float ambientWithAlpha[] = {m_material.ambient.x, m_material.ambient.y,
                                    m_material.ambient.z, m_material.opacity};
        float diffuseWithAlpha[] = {m_material.diffuse.x, m_material.diffuse.y,
                                    m_material.diffuse.z, m_material.opacity};
        float specularWithAlpha[] = {m_material.specular.x, m_material.specular.y,
                                     m_material.specular.z, m_material.opacity};

        glMaterialfv(GL_FRONT, GL_AMBIENT, ambientWithAlpha);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseWithAlpha);
        glMaterialfv(GL_FRONT, GL_SPECULAR, specularWithAlpha);
    }

    // 1. 绑定纹理
    if (m_pTexture && m_pTexture->IsValid())
    {
        glActiveTexture(GL_TEXTURE0); // 明确指定纹理单元
        glEnable(GL_TEXTURE_2D);
        m_pTexture->Bind(GL_TEXTURE0); // 传递纹理单元参数

        // 设置纹理环境
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    else
    {
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0); // 确保解绑
    }

    // 2. 启用顶点数组状态
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // 3. 设置指针
    // 注意：利用 sizeof(Vertex) 作为步长，并指向结构体成员的地址
    const GLsizei stride = sizeof(Vertex);

    glVertexPointer(3, GL_FLOAT, stride, &m_vertices[0].Position);
    glNormalPointer(GL_FLOAT, stride, &m_vertices[0].Normal);
    glTexCoordPointer(2, GL_FLOAT, stride, &m_vertices[0].TexCoords);

    // 4. 绘图
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
                   GL_UNSIGNED_INT, m_indices.data());

    // 5. 关闭状态
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    // 6. 清理纹理状态
    glActiveTexture(GL_TEXTURE0);
    if (m_pTexture)
    {
        m_pTexture->Unbind(GL_TEXTURE0); // 解绑纹理
    }
    glBindTexture(GL_TEXTURE_2D, 0); // 双重保险
    glDisable(GL_TEXTURE_2D);

    
    // 6. 关闭混合（如果开启了）
    if (m_material.opacity < 1.0f)
    {
        glDisable(GL_BLEND);
    }
    
    glPopAttrib();
}

void CMesh::CalculateBoundingBox()
{
    if (m_vertices.empty())
    {
        m_boundingBox.min = m_boundingBox.max = m_boundingBox.center = m_boundingBox.size = Vector3::Zero();
        return;
    }

    Vector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
    Vector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    for (const auto &v : m_vertices)
    {
        vMin.x = Math::Min(vMin.x, v.Position.x);
        vMin.y = Math::Min(vMin.y, v.Position.y);
        vMin.z = Math::Min(vMin.z, v.Position.z);

        vMax.x = Math::Max(vMax.x, v.Position.x);
        vMax.y = Math::Max(vMax.y, v.Position.y);
        vMax.z = Math::Max(vMax.z, v.Position.z);
    }

    m_boundingBox.min = vMin;
    m_boundingBox.max = vMax;
    m_boundingBox.center = (vMin + vMax) * 0.5f;
    m_boundingBox.size = vMax - vMin;
}

void CMesh::DrawNormals(float scale, unsigned int step, const Vector3 &color) const
{
    if (m_vertices.empty())
        return;

    // 确保步长至少为1，防止死循环
    if (step < 1)
        step = 1;

    // 1. 保护现有渲染状态
    // GL_ENABLE_BIT: 备份 glEnable/Disable 状态
    // GL_CURRENT_BIT: 备份当前颜色等状态
    // GL_LINE_BIT: 备份线宽状态
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);

    // 2. 强制关闭干扰状态
    glDisable(GL_LIGHTING);   // 调试线不需要光照
    glDisable(GL_TEXTURE_2D); // 调试线不需要贴图

    // 如果希望法线永远显示在模型前面（透视效果），取消下面这行的注释
    // glDisable(GL_DEPTH_TEST);

    // 3. 开始绘制
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (size_t i = 0; i < m_vertices.size(); i += step)
    {
        const auto &v = m_vertices[i];

        // 起点：红色
        glColor3f(color.x, color.y, color.z);
        glVertex3f(v.Position.x, v.Position.y, v.Position.z);

        // 终点：黄色
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(v.Position.x + v.Normal.x * scale,
                   v.Position.y + v.Normal.y * scale,
                   v.Position.z + v.Normal.z * scale);
    }
    glEnd();

    // 4. 恢复状态
    glPopAttrib();
}