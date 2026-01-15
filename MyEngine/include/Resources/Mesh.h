
// ======================================================================
#ifndef __MESH_H__
#define __MESH_H__
// ======================================================================
#include <Windows.h>
#include <string>
#include <sstream>
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
    struct SimpleMaterial
    {
        std::wstring name = L"DefaultMaterial";
        Vector3 ambient = Vector3(0.2f, 0.2f, 0.2f);
        Vector3 diffuse = Vector3(0.8f, 0.8f, 0.8f);
        Vector3 specular = Vector3(1.0f, 1.0f, 1.0f);
        float shininess = 32.0f;
        float opacity = 1.0f;
        // float reflectivity = 0.0f;
        // std::shared_ptr<CTexture> normalMap;
        // std::shared_ptr<CTexture> specularMap;

        // 比较操作符
        bool operator==(const SimpleMaterial &other) const
        {
            return name == other.name &&
                   ambient == other.ambient &&
                   diffuse == other.diffuse &&
                   specular == other.specular &&
                   Math::FloatEqual(shininess, other.shininess) &&
                   Math::FloatEqual(opacity, other.opacity);
        }

        bool operator!=(const SimpleMaterial &other) const
        {
            return !(*this == other);
        }

        // 获取材质哈希
        std::wstring GetHash() const
        {
            std::wostringstream woss;
            woss << name << L"_"
                 << ambient.x << L"_" << ambient.y << L"_" << ambient.z << L"_"
                 << diffuse.x << L"_" << diffuse.y << L"_" << diffuse.z << L"_"
                 << specular.x << L"_" << specular.y << L"_" << specular.z << L"_"
                 << shininess << L"_" << opacity;
            return woss.str();
        }
    };

    // 设置材质名称
    void SetMaterialName(const std::wstring &name) { m_material.name = name; }
    const std::wstring &GetMaterialName() const { return m_material.name; }

    // 完整材质设置
    void SetMaterial(const SimpleMaterial &material) { m_material = material; }
    const SimpleMaterial &GetMaterial() const { return m_material; }

    // 设置透明度
    void SetOpacity(float opacity) { m_material.opacity = opacity; }
    float GetOpacity() const { return m_material.opacity; }

    // 纹理相关
    void SetTexture(std::shared_ptr<CTexture> pTexture) { m_pTexture = pTexture; }
    std::shared_ptr<CTexture> GetTexture() const { return m_pTexture; }

    // 子网格标识
    void SetSubMeshID(int id) { m_subMeshID = id; }
    int GetSubMeshID() const { return m_subMeshID; }

    struct BoundingBox
    {
        Vector3 min;
        Vector3 max;
        Vector3 center;
        Vector3 size;
    };

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

    const std::vector<Vertex> &GetVertices() const { return m_vertices; }     // 获取顶点数据
    const std::vector<unsigned int> &GetIndices() const { return m_indices; } // 获取索引数据

    size_t GetVertexCount() const { return m_vertices.size(); }
    size_t GetIndexCount() const { return m_indices.size(); }
    size_t GetTriangleCount() const { return m_indices.size() / 3; }

    // 边界
    const Vector3 &GetMinBounds() const { return m_boundingBox.min; }
    const Vector3 &GetMaxBounds() const { return m_boundingBox.max; }
    const BoundingBox &GetBoundingBox() const { return m_boundingBox; }

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    std::shared_ptr<CTexture> m_pTexture;
    SimpleMaterial m_material;

    int m_subMeshID = -1; // 在模型中的子网格ID

    void CalculateBoundingBox();
    BoundingBox m_boundingBox;
};
#endif // __MESH_H__