
// ======================================================================
#include "stdafx.h"
#include "EngineConfig.h"
#include "Entities/TerrainEntity.h"
#include "Core/GameEngine.h"
#include "Graphics/Camera/Camera.h"
#include "Resources/ResourceManager.h"
#include "Utils/StringUtils.h"
#include "Utils/stb_image.h"
// ======================================================================

unsigned int CTerrainEntity::s_nextID = 3000;

CTerrainEntity::CTerrainEntity()
    : m_pTexture(nullptr),                    // 纹理
      m_width(0),                             // 宽度
      m_height(0),                            // 高度
      m_cellSize(0.0f),                       //
      m_maxHeight(15.0f),                     // 最大高度
      m_bWireframe(FALSE),                    //
      m_iLODLevel(1),                         //
      m_bUseVBO(FALSE),                       //
      m_vertexBuffer(0),                      //
      m_indexBuffer(0),                       //
      m_terrainColor(0.5f, 0.5f, 0.5f, 1.0f), // 地形颜色灰色
      m_bDrawNormals(FALSE),                  // 是否绘制法线
      m_uNormalStep(5),                       // 法线步长
      m_fNormalScale(10.0f)                   // 法线长度
{
    SetName(L"Terrain");
}

CTerrainEntity::~CTerrainEntity()
{
    // 清理 VBO
    if (m_bUseVBO)
    {
        if (m_vertexBuffer)
            glDeleteBuffers(1, &m_vertexBuffer);
        if (m_indexBuffer)
            glDeleteBuffers(1, &m_indexBuffer);
    }
}

std::shared_ptr<CTerrainEntity> CTerrainEntity::Create(const std::wstring &heightmapPath, const std::wstring &texturePath, float size, float maxHeight)
{
    auto entity = std::shared_ptr<CTerrainEntity>(new CTerrainEntity());
    entity->m_uID = ++s_nextID;

    if (entity->LoadHeightmap(heightmapPath, size, maxHeight))
    {
        if (!texturePath.empty())
        {
            // 通过游戏引擎单例获取资源管理器
            auto pResMgr = CGameEngine::GetInstance().GetResourceManager();
            if (pResMgr)
            {
                // 改为调用 GetTexture 获取 shared_ptr
                auto pTex = pResMgr->GetTexture(texturePath, CResourceManager::PathType::Absolute);
                entity->SetTexture(pTex); // SetTexture 参数也需同步修改
            }
        }
        return entity;
    }
    return nullptr;
}

std::shared_ptr<CTerrainEntity> CTerrainEntity::CreateProcedural(int width, int height, float size, float maxHeight)
{
    auto entity = std::shared_ptr<CTerrainEntity>(new CTerrainEntity());
    entity->m_uID = ++s_nextID;
    entity->GenerateProceduralTerrain(width, height, size, maxHeight);
    return entity;
}

BOOL CTerrainEntity::LoadHeightmap(const std::wstring &path, float size, float maxHeight)
{
    // 设置参数
    m_maxHeight = maxHeight;

    // 转换宽字符串到窄字符串
    std::string narrowPath = CStringUtils::WStringToString(path);

    // 3. 配置 stbi 并加载图片
    int channels;
    stbi_set_flip_vertically_on_load(true); // 翻转Y轴以匹配 OpenGL 坐标系

    unsigned char *data = stbi_load(narrowPath.c_str(), &m_width, &m_height, &channels, 1);

    // 4. 安全检查
    if (!data || m_width <= 0 || m_height <= 0)
    {
        LogError(L"高度图加载失败: %ls. 请检查路径是否存在或格式是否正确.\n", path.c_str());
        if (data)
            stbi_image_free(data);
        return FALSE;
    }

    // 5. 存储高度数据并计算格子大小
    m_heightData.clear();
    m_heightData.resize(m_width * m_height);

    // 确保分母不为0
    m_cellSize = (m_width > 1) ? size / (m_width - 1) : size;

    // 6. 生成顶点数据
    m_vertices.clear();
    m_vertices.resize(m_width * m_height);

    for (int z = 0; z < m_height; ++z)
    {
        for (int x = 0; x < m_width; ++x)
        {
            int index = z * m_width + x;
            Vertex &v = m_vertices[index];

            // 计算世界空间位置（中心对齐）
            v.pos.x = (x - m_width * 0.5f) * m_cellSize;
            v.pos.z = (z - m_height * 0.5f) * m_cellSize;

            // 从灰度值转换高度 (0-255 -> 0.0-maxHeight)
            float h = (float)data[index] / 255.0f * m_maxHeight;
            v.pos.y = h;
            m_heightData[index] = h;

            // UV坐标：设置纹理重复次数
            float textureRepeat = 20.0f;
            v.uv.x = ((float)x / (m_width - 1)) * textureRepeat;
            v.uv.y = ((float)z / (m_height - 1)) * textureRepeat;

            // 设置初始属性
            v.color = m_terrainColor;
            v.normal = Vector3(0, 1, 0); // 稍后在 CalculateNormals 计算
        }
    }
    // 7. 释放原始图片内存
    stbi_image_free(data);

    // 计算法线和生成索引
    GenerateIndices();
    CalculateNormals();

    // 尝试创建VBO
    CreateVBO();

    LogInfo(L"地形加载成功: %ls. 分辨率: %dx%d, 实际尺寸: %.1fx%.1f\n",
            path.c_str(), m_width, m_height, size, size);
    return TRUE;
}

void CTerrainEntity::GenerateProceduralTerrain(int width, int height, float size, float maxHeight)
{
    m_width = width;
    m_height = height;
    m_maxHeight = maxHeight;
    m_cellSize = size / (width - 1);

    // 生成程序化地形（使用柏林噪声或正弦波）
    m_vertices.resize(m_width * m_height);
    m_heightData.resize(m_width * m_height);

    for (int z = 0; z < m_height; ++z)
    {
        for (int x = 0; x < m_width; ++x)
        {
            int index = z * m_width + x;
            Vertex &v = m_vertices[index];

            v.pos.x = (x - m_width * 0.5f) * m_cellSize;
            v.pos.z = (z - m_height * 0.5f) * m_cellSize;

            // 使用多种噪声组合创建有趣的地形
            float noise1 = sin(v.pos.x * 0.1f) * cos(v.pos.z * 0.1f) * 2.0f;
            float noise2 = sin(v.pos.x * 0.05f) * 1.5f;
            float noise3 = cos(v.pos.z * 0.03f) * 1.2f;

            v.pos.y = (noise1 + noise2 + noise3) * m_maxHeight * 0.1f;
            m_heightData[index] = v.pos.y;

            v.uv.x = (float)x / (m_width - 1);
            v.uv.y = (float)z / (m_height - 1);
            v.color = m_terrainColor;
            v.normal = Vector3(0, 1, 0);
        }
    }

    CalculateNormals();
    GenerateIndices();
    CreateVBO();
}

void CTerrainEntity::CalculateNormals()
{
    // 先清零所有法线
    for (auto &v : m_vertices)
    {
        v.normal = Vector3(0, 0, 0);
    }

    // 计算每个面的法线并累加到顶点
    for (size_t i = 0; i < m_indices.size(); i += 3)
    {
        unsigned int i1 = m_indices[i];
        unsigned int i2 = m_indices[i + 1];
        unsigned int i3 = m_indices[i + 2];

        Vector3 v1 = m_vertices[i1].pos;
        Vector3 v2 = m_vertices[i2].pos;
        Vector3 v3 = m_vertices[i3].pos;

        Vector3 edge1 = v2 - v1;
        Vector3 edge2 = v3 - v1;
        Vector3 normal = Vector3::Cross(edge1, edge2).Normalized();

        m_vertices[i1].normal += normal;
        m_vertices[i2].normal += normal;
        m_vertices[i3].normal += normal;
    }

    // 归一化所有法线
    for (auto &v : m_vertices)
    {
        v.normal.Normalize();
    }

    // for (int i = 0; i < 5 && i < m_vertices.size(); ++i)
    // {
    //     LogDebug(L"Vertex %d Normal: %.2f, %.2f, %.2f.\n", i, m_vertices[i].normal.x, m_vertices[i].normal.y, m_vertices[i].normal.z);
    // }
}

void CTerrainEntity::GenerateIndices()
{
    m_indices.clear();
    m_indices.reserve((m_width - 1) * (m_height - 1) * 6);

    // 根据LOD级别跳过一些顶点
    int step = m_iLODLevel;

    for (int z = 0; z < m_height - 1; z += step)
    {
        for (int x = 0; x < m_width - 1; x += step)
        {
            int i0 = z * m_width + x;
            int i1 = (z + step) * m_width + x;
            int i2 = z * m_width + (x + step);
            int i3 = (z + step) * m_width + (x + step);

            // 两个三角形组成一个四边形
            m_indices.push_back(i0);
            m_indices.push_back(i1);
            m_indices.push_back(i2);

            m_indices.push_back(i2);
            m_indices.push_back(i1);
            m_indices.push_back(i3);
        }
    }
}

void CTerrainEntity::CreateVBO()
{
    // 检查OpenGL扩展支持
    // 直接检查版本字符串
    const char *versionStr = (const char *)glGetString(GL_VERSION);
    if (!versionStr)
    {
        m_bUseVBO = false;
        LogWarning(L"无法获取OpenGL版本信息\n");
        return;
    }

    // 解析版本号 "Major.Minior"
    int major = 0, minor = 0;
    if (sscanf_s(versionStr, "%d.%d", &major, &minor) != 2)
    {
        m_bUseVBO = false;

        // 将char*转换为wchar_t*用于日志输出
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, versionStr, -1, nullptr, 0);
        if (wideLen > 0)
        {
            std::vector<wchar_t> wideVersion(wideLen);
            MultiByteToWideChar(CP_UTF8, 0, versionStr, -1, wideVersion.data(), wideLen);
            LogWarning(L"无法解析OpenGL版本: %s", wideVersion.data());
        }
        else
        {
            LogWarning(L"无法解析OpenGL版本字符串");
        }
        return;
    }

    // OpenGL 1.5+ 支持VBO
    if (major > 1 || (major == 1 && minor >= 5))
    {
        m_bUseVBO = true;
        // LogInfo(L"OpenGL版本 %d.%d 支持VBO\n", major, minor);
    }
    else
    {
        m_bUseVBO = false;
        LogWarning(L"OpenGL版本过低 (%d.%d), VBO不可用", major, minor);
        return;
    }

    // 创建顶点缓冲区
    glGenBuffers(1, &m_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
                 m_vertices.data(), GL_STATIC_DRAW);

    // 创建索引缓冲区
    glGenBuffers(1, &m_indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
                 m_indices.data(), GL_STATIC_DRAW);

    // 检查OpenGL错误
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        LogError(L"创建VBO时发生OpenGL错误: %d", error);
        m_bUseVBO = false;

        // 清理已创建的缓冲区
        if (m_vertexBuffer)
            glDeleteBuffers(1, &m_vertexBuffer);
        if (m_indexBuffer)
            glDeleteBuffers(1, &m_indexBuffer);
        return;
    }

    LogDebug(L"地形VBO创建成功, 顶点数: %d, 三角形数: %d.\n",
             m_vertices.size(), m_indices.size() / 3);
}

void CTerrainEntity::Update(float deltaTime)
{
    CEntity::Update(deltaTime);
    // 可以在这里添加地形的动态更新逻辑
}

void CTerrainEntity::Render()
{
    if (!m_bVisible || m_vertices.empty())
        return;

    // GLint currentTexture;
    // glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
    // LogDebug(L"地形渲染前强制清理纹理: %d -> 0.\n", currentTexture);

    glPushAttrib(GL_ENABLE_BIT | GL_POLYGON_BIT | GL_LIGHTING_BIT | GL_TEXTURE_BIT);

    // 设置渲染状态
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    // ?: 测试光源
    // glEnable(GL_LIGHT0); // 启用默认光源
    // glDisable(GL_LIGHTING);  // 先关闭光照, 看地形是否显示
    glEnable(GL_COLOR_MATERIAL); // CAUTION: 关键：让顶点颜色生效
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // 如果没有光源, 这行可以让你看清地形（但失去阴影感）
    // glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    if (m_bWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    glPushMatrix();
    ApplyTransform();

    // 设置材质
    // TODO: 设置地形材质
    GLfloat matAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};  // 环境光反射
    GLfloat matDiffuse[] = {0.6f, 0.6f, 0.6f, 1.0f};  // 漫反射
    GLfloat matSpecular[] = {0.9f, 0.9f, 0.9f, 1.0f}; // 镜面反射
    GLfloat matShininess[] = {100.0f};                // 光泽度

    // 塑料质感
    // GLfloat matAmbient[] = {0.1f, 0.1f, 0.1f, 1.0f};    // 减少环境光
    // GLfloat matDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};    // 保持中性色
    // GLfloat matSpecular[] = {0.8f, 0.8f, 0.8f, 1.0f};   // 增强镜面反射
    // GLfloat matShininess[] = {80.0f};                   // 提高光泽度

    // 金属质感
    // GLfloat matAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    // GLfloat matDiffuse[] = {0.6f, 0.6f, 0.6f, 1.0f};
    // GLfloat matSpecular[] = {0.9f, 0.9f, 0.9f, 1.0f};   // 强镜面反射
    // GLfloat matShininess[] = {100.0f};                  // 高光泽度

    // 绿色草地质感
    // GLfloat matAmbient[] = {0.2f, 0.3f, 0.1f, 1.0f};    // 绿色环境光
    // GLfloat matDiffuse[] = {0.2f, 0.5f, 0.1f, 1.0f};    // 绿色漫反射
    // GLfloat matSpecular[] = {0.1f, 0.2f, 0.05f, 1.0f};  // 弱绿色高光
    // GLfloat matShininess[] = {5.0f};                    // 非常粗糙

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    if (m_pTexture && m_pTexture->GetID() != 0)
    {
        // LogDebug(L"绑定地形纹理: %d.\n", m_uTextureID);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_pTexture->GetID());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        // 检查纹理绑定状态
        GLint boundTexture;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);
        // LogDebug(L"当前绑定的纹理: %d.\n", boundTexture);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
        LogWarning(L"地形纹理ID为0, 使用颜色渲染\n");
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (m_bUseVBO && m_iLODLevel == 1)
    {
        // 使用VBO渲染（最高质量）
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

        // 设置顶点指针
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_NORMAL_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, pos));
        glNormalPointer(GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, normal));
        glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, uv));
        glColorPointer(4, GL_FLOAT, sizeof(Vertex), (void *)offsetof(Vertex, color));

        glDrawElements(GL_TRIANGLES, (GLsizei)m_indices.size(), GL_UNSIGNED_INT, 0);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDisableClientState(GL_VERTEX_ARRAY); // CAUTION: 必要
    }
    else
    {
        // 使用立即模式渲染（支持LOD）
        glBegin(GL_TRIANGLES);
        for (size_t i = 0; i < m_indices.size(); ++i)
        {
            const Vertex &v = m_vertices[m_indices[i]];
            glNormal3f(v.normal.x, v.normal.y, v.normal.z);
            glTexCoord2f(v.uv.x, v.uv.y);
            // glColor4f(v.color.x, v.color.y, v.color.z, v.color.w);
            glVertex3f(v.pos.x, v.pos.y, v.pos.z);
        }
        glEnd();
    }

    // 渲染法线
    if (m_bDrawNormals)
    {
        DrawNormals(m_fNormalScale, m_uNormalStep);
    }

    // 解绑纹理
    if (m_pTexture && m_pTexture->GetID() != 0)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

    glPopMatrix();
    glPopAttrib();

    // 渲染子实体
    CEntity::Render();
}

float CTerrainEntity::GetHeightAt(float worldX, float worldZ) const
{
    // 获取地形当前的位置（假设没有旋转和缩放）
    Vector3 terrainPos = GetPosition();

    // 将世界坐标转为相对于地形左上角的局部坐标
    // 这里的逻辑要和你生成顶点时的 (x - m_width * 0.5f) * m_cellSize 对应
    float localX = (worldX - terrainPos.x) / m_cellSize + (m_width - 1) * 0.5f;
    float localZ = (worldZ - terrainPos.z) / m_cellSize + (m_height - 1) * 0.5f;

    int x0 = (int)floor(localX);
    int z0 = (int)floor(localZ);

    // 边界检查
    if (x0 < 0 || x0 >= m_width - 1 || z0 < 0 || z0 >= m_height - 1)
        return 0.0f;

    float dx = localX - x0;
    float dz = localZ - z0;

    // 四个角的高度
    float h00 = m_heightData[z0 * m_width + x0];
    float h10 = m_heightData[z0 * m_width + (x0 + 1)];
    float h01 = m_heightData[(z0 + 1) * m_width + x0];
    float h11 = m_heightData[(z0 + 1) * m_width + (x0 + 1)];

    // 双线性插值公式
    float h = (1 - dx) * (1 - dz) * h00 +
              dx * (1 - dz) * h10 +
              (1 - dx) * dz * h01 +
              dx * dz * h11;

    return h + terrainPos.y; // 加上地形本身的 Y 轴位移
}

void CTerrainEntity::RenderSimpleGeometry()
{
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < m_indices.size(); ++i)
    {
        const Vertex &v = m_vertices[m_indices[i]];
        glVertex3f(v.pos.x, v.pos.y, v.pos.z);
    }
    glEnd();
}

void CTerrainEntity::DrawNormals(float scale, unsigned int step)
{
    // 使用默认值如果参数为默认值
    float actualScale = (scale < 0) ? m_fNormalScale : scale;
    unsigned int actualStep = (step == 0) ? m_uNormalStep : step;

    DrawNormalsImpl(actualScale, actualStep);
}

void CTerrainEntity::DrawNormalsImpl(float scale, unsigned int step)
{
    if (m_vertices.empty())
    {
        LogWarning(L"无法绘制法线：顶点数据为空.\n");
        return;
    }

    // 保存OpenGL状态
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);

    // 禁用深度测试
    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    // 保存当前颜色
    GLfloat currentColor[4];
    glGetFloatv(GL_CURRENT_COLOR, currentColor);

    // 设置法线颜色（亮红色, 便于观察）
    glColor3f(1.0f, 0.2f, 0.2f);
    glLineWidth(1.0f);

    // 绘制法线线条
    glBegin(GL_LINES);
    size_t vertexCount = m_vertices.size();
    for (size_t i = 0; i < vertexCount; i += step)
    {
        const Vertex &v = m_vertices[i];

        if (v.normal.Length() < 0.1f)
            continue; // 跳过无效法线
        Vector3 endPos = v.pos + v.normal * scale;

        glVertex3f(v.pos.x, v.pos.y, v.pos.z);
        glVertex3f(endPos.x, endPos.y, endPos.z);
    }
    glEnd();

    // 恢复状态
    glLineWidth(1.0f);
    glColor4fv(currentColor);
    glPopAttrib();

    // LogDebug(L"绘制了 %d 条法线, 缩放: %.1f, 步长: %d.\n",
    //          m_vertices.size() / step, scale, step);
}