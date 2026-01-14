
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
}

CMesh::~CMesh()
{
    // vector 会自动析构，shared_ptr 会自动减引用
}

void CMesh::Draw() const
{
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
}