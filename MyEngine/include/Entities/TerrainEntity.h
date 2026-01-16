
// ======================================================================
#ifndef __TERRAIN_ENTITY_H__
#define __TERRAIN_ENTITY_H__
// ======================================================================
#include <vector>
#include "Core/Entity.h"
// ======================================================================

class CTerrainEntity : public CEntity
{
private:
    static unsigned int s_nextID;

public:
    virtual ~CTerrainEntity();

    static std::shared_ptr<CTerrainEntity> Create(const std::wstring &heightmapPath, float size, float maxHeight)
    {
        auto entity = std::shared_ptr<CTerrainEntity>(new CTerrainEntity());
        entity->m_uID = ++s_nextID;
        if (entity->LoadHeightmap(heightmapPath, size, maxHeight))
        {
            return entity;
        }
        return nullptr;
    }

    // 静态创建简化版本（用于测试）
    static std::shared_ptr<CTerrainEntity> CreateProcedural(int width, int height, float size, float maxHeight);

    virtual void Update(float deltaTime) override;

    virtual void Render() override;

    // 地形查询功能
    float GetHeightAt(float worldX, float worldZ) const;
    Vector3 GetNormalAt(float worldX, float worldZ) const;
    bool IsPositionOnTerrain(float worldX, float worldZ) const;

    // 设置地形属性
    void SetTexture(GLuint textureID) {
        LogInfo(L"设置地形纹理ID: %d", textureID);
        m_uTextureID = textureID; 
    }
    void SetColor(const Vector4 &color) { m_terrainColor = color; }
    void EnableWireframe(bool enable) { m_bWireframe = enable; }
    void SetLODLevel(int lod) { m_iLODLevel = std::max(1, std::min(4, lod)); }

    void RenderSimpleGeometry();
    void RenderWithLightingDebug();
    void RenderNormalsDebug();

protected:
    CTerrainEntity();
    BOOL LoadHeightmap(const std::wstring &path, float size, float maxHeight);
    void GenerateProceduralTerrain(int width, int height, float size, float maxHeight);
    void CalculateNormals();
    void GenerateIndices();

private:
    struct Vertex
    {
        Vector3 pos;
        Vector2 uv;
        Vector3 normal;
        Vector4 color;
    };

    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    GLuint m_uTextureID;
    Vector4 m_terrainColor;

    int m_width, m_height;
    float m_cellSize;
    float m_maxHeight;
    bool m_bWireframe;
    int m_iLODLevel;
    bool m_bUseVBO;
    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;

    // 高度图数据
    std::vector<float> m_heightData;

    void CreateVBO();
};

#endif // __TERRAIN_ENTITY_H__