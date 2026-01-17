
// ======================================================================
#ifndef __TERRAIN_ENTITY_H__
#define __TERRAIN_ENTITY_H__
// ======================================================================
#include <vector>
#include "Core/Entity.h"
#include "Resources/Texture.h"
// ======================================================================
class Vector3;
// ======================================================================
class CTerrainEntity : public CEntity
{
private:
    static unsigned int s_nextID;

public:
    virtual ~CTerrainEntity();

    // 创建地形
    static std::shared_ptr<CTerrainEntity> Create(const std::wstring &heightmapPath,
                                                  const std::wstring &texturePath,
                                                  float size, float maxHeight);
    static std::shared_ptr<CTerrainEntity> CreateProcedural(int width, int height, float size, float maxHeight);

    virtual void Update(float deltaTime) override;
    virtual void Render() override;

    // 地形查询功能
    float GetHeightAt(float worldX, float worldZ) const;
    float GetGroundHeight(const Vector3& worldPos) const;
    Vector3 GetNormalAt(float worldX, float worldZ) const;
    bool IsPositionOnTerrain(float worldX, float worldZ) const;

    // 设置地形属性
    void SetTexture(std::shared_ptr<CTexture> pTexture)
    {
        if (pTexture)
        {
            LogDebug(L"设置地形纹理ID: %u.\n", pTexture->GetID()); // 假设 CTexture 有 GetID() 方法
        }
        m_pTexture = pTexture;
    }
    void SetColor(const Vector4 &color) { m_terrainColor = color; }

    void EnableWireframe(bool enable) { m_bWireframe = enable; }
    void SetLODLevel(int lod) { m_iLODLevel = std::max(1, std::min(4, lod)); }

    // ======================================================================
    // 法线
    // 开启/关闭法线绘制
    void SetDrawNormals(BOOL bDraw) { m_bDrawNormals = bDraw; }
    BOOL IsDrawNormals() const { return m_bDrawNormals; }
    // 设置法线长度缩放
    void SetNormalScale(float scale) { m_fNormalScale = scale; }
    void SetNormalStep(unsigned int step) { m_uNormalStep = step; }
    // 绘制法线
    void DrawNormals(float scale, unsigned int step);

    void RenderSimpleGeometry();

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
    std::shared_ptr<CTexture> m_pTexture;
    Vector4 m_terrainColor;

    int m_width, m_height; // 宽高
    float m_maxHeight;     // 最大高度
    float m_cellSize;
    BOOL m_bWireframe;

    int m_iLODLevel;
    bool m_bUseVBO;
    GLuint m_vertexBuffer;
    GLuint m_indexBuffer;

    std::vector<float> m_heightData; // 高度图数据

    BOOL m_bDrawNormals = FALSE;    // 是否绘制法线开关
    float m_fNormalScale = 10.0f;   // 法线显示长度
    unsigned int m_uNormalStep = 5; // 法线步长
    void DrawNormalsImpl(float scale, unsigned int step);

    void CreateVBO();
};

#endif // __TERRAIN_ENTITY_H__