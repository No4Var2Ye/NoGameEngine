// ======================================================================
#ifndef __GRID_ENTITY_H__
#define __GRID_ENTITY_H__
// ======================================================================
#include "Core/Entity.h"
#include "Math/Vector3.h"
// ======================================================================
struct GridVertex
{
    float x, y, z;
    unsigned char r, g, b, a;

    GridVertex(float x, float y, float z, unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
        : x(x), y(y), z(z), r(r), g(g), b(b), a(a) {}
};

class CGridEntity : public CEntity
{
private:
    static unsigned int s_nextID;

public:
    virtual ~CGridEntity() = default;

    // 创建网格实体
    template <typename... Args>
    static std::shared_ptr<CGridEntity> Create(Args &&...args)
    {
        auto entity = std::shared_ptr<CGridEntity>(new CGridEntity(std::forward<Args>(args)...));
        entity->m_uID = ++s_nextID;
        return entity;
    }

    // 属性设置
    void SetSize(FLOAT size) { m_fSize = size; }
    void SetStep(FLOAT step) { m_fStep = step; }
    void SetGridColor(const Vector3 &mainColor, const Vector3 &subColor)
    {
        m_MainColor = mainColor;
        m_SubColor = subColor;
    }

    virtual void Update(float deltaTime) override;
    virtual void Render() override;
    void DrawFadingLine(float coord, BOOL isParallelToZ, const Vector3 &camPos, const Vector3 &color, float maxDist);

    // 世界坐标
    void SetFadeDist(float minor, float major)
    {
        m_fMinorFadeDist = minor;
        m_fMajorFadeDist = major;
    }
    void SetShowAxes(BOOL bShow) { m_bShowAxes = bShow; }
    void SetGridColors(Vector3 main, Vector3 sub)
    {
        m_MainColor = main;
        m_SubColor = sub;
    }

protected:
    CGridEntity(FLOAT size = 100.0f, FLOAT step = 1.0f);

private:
    std::vector<GridVertex> m_gridVertices;
    std::vector<GridVertex> m_MainGridVertices; // 粗网格 (Major)
    std::vector<GridVertex> m_SubGridVertices;  // 细网格 (Minor)

    FLOAT m_fSize;
    FLOAT m_fStep;
    Vector3 m_MainColor; // 轴线颜色
    Vector3 m_SubColor;  // 普通网格颜色

    // 世界坐标
    BOOL m_bShowAxes;       // 是否显示 XYZ 轴
    BOOL m_bEnableFade;     // 是否启用距离淡出
    float m_fMinorFadeDist; // 细网格消失距离
    float m_fMajorFadeDist; // 粗网格消失距离

    Vector3 m_AxisColorX = Vector3(1.0f, 0.2f, 0.2f);
    Vector3 m_AxisColorY = Vector3(0.2f, 1.0f, 0.2f);
    Vector3 m_AxisColorZ = Vector3(0.2f, 0.2f, 1.0f);

    void InitGrid();
    void BuildGeometry();
    void BuildGeometryWithLOD();
    int CalculateLODLevel() const;
    BOOL ShouldRebuildLOD() const;
    void SetupDistanceFog();
    void RenderGridWithVertexArrays();
    void RenderCoordinateAxes();
};

#endif // __GRID_ENTITY_H__