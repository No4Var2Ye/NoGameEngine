
// ======================================================================
#ifndef __MESH_H__
#define __MESH_H__
// ======================================================================
#include <Windows.h>
#include <string>
#include <vector>
#include <memory>
#include "GL/gl.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
// ======================================================================
class CTexture;
// ======================================================================

// ======================================================================
// 确保 Vertex 结构体在内存中是紧凑排列的
#pragma pack(push, 1)
struct Vertex
{
    Vector3 Position;
    Vector3 Normal;
    Vector2 TexCoords;
};
#pragma pack(pop)

// ======================================================================
class CMesh
{
public:
    /**
     * @param vertices 顶点数据
     * @param indices 索引数据
     * @param pTexture 该网格关联的贴图（由 CModel 从 ResourceManager 获取后传入）
     */
    CMesh(const std::vector<Vertex> &vertices,
          const std::vector<unsigned int> &indices,
          std::shared_ptr<CTexture> pTexture = nullptr);
    ~CMesh();

    // 渲染网格
    void Draw() const;

    const std::vector<Vertex> &GetVertices() const { return m_vertices; } // 获取顶点数据
    const std::vector<unsigned int> &GetIndices() const { return m_indices; } // 获取索引数据

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::shared_ptr<CTexture> m_pTexture;
};
#endif // __MESH_H__