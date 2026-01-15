
// ======================================================================
#include "stdafx.h"
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
}

CMesh::~CMesh()
{
    // vector 会自动析构，shared_ptr 会自动减引用
}

void CMesh::Draw() const
{
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
    if (m_pTexture)
    {
        glEnable(GL_TEXTURE_2D);
        m_pTexture->Bind();
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
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

    // 4. 绘图 (使用索引缓冲区)
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
                   GL_UNSIGNED_INT, m_indices.data());

    // 5. 关闭状态 (良好的编程习惯)
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    // 6. 关闭混合（如果开启了）
    if (m_material.opacity < 1.0f)
    {
        glDisable(GL_BLEND);
    }

    if (m_pTexture)
    {
        glDisable(GL_TEXTURE_2D);
    }
}