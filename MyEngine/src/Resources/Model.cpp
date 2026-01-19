
// ======================================================================
#include "stdafx.h"
#include <iostream>
#include <cfloat>
#include "Resources/Model.h"
#include "Resources/Mesh.h"
#include "Resources/ResourceManager.h"
#include "Math/MathConverter.h"
#include "Utils/StringUtils.h"
// ======================================================================

// filepath 参考系 是exe文件
// 这里传入的文件路径是fullpath, 保证通用性
BOOL CModel::LoadFromFile(const std::wstring &filePath, CResourceManager *pResMgr)
{
    if (!pResMgr)
    {
        LogError(L"ResourceManager为空\n");
        return FALSE;
    }

    // --- 1. 初始化动画容器 ---
    m_FinalMatrices.clear();
    m_FinalMatrices.resize(MAX_BONES, Matrix4::Identity());
    m_BoneMapping.clear(); // 清空旧模型的骨骼映射

    Unload();

    // 转换语义
    // fullPath = res/Models/Duck/Duck.obj
    std::wstring fullPath = filePath;

    // m_filePath = res/Models/Duck/Duck.obj
    m_filePath = fullPath;

    // Assimp::Importer importer;

    // 1. 将 wstring 转为 string (Assimp 接口要求)
    // 这里可以使用之前提到的 CStringUtils 或简单的转换

    std::string pathStr = CStringUtils::WStringToString(fullPath);

    // 2. 读取文件并进行预处理
    // WARNING: 贴图翻转
    unsigned int flags =
        aiProcess_Triangulate |           // 转为三角形
        aiProcess_FlipUVs |               // 翻转纹理坐标
        aiProcess_GenSmoothNormals |      // 生成平滑法线
        aiProcess_CalcTangentSpace |      // 计算切线空间（如果需要法线贴图）
        aiProcess_JoinIdenticalVertices | // 合并重复顶点
        aiProcess_ImproveCacheLocality |  // 优化缓存局部性
        aiProcess_ValidateDataStructure | // 验证数据结构
        aiProcess_OptimizeMeshes;         // 优化网格

    // --- 3. 使用类成员 m_Importer 加载 ---
    // 注意：不要再声明 Assimp::Importer importer; 局部变量了
    m_pScene = m_Importer.ReadFile(pathStr, flags);

    if (!m_pScene)
    {
        LogError(L"Assimp Error: %hs \n", m_Importer.GetErrorString());
        return FALSE;
    }

    if (m_pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        LogWarning(L"[Model] 场景不完整，模型可能显示异常: %ls. \n", fullPath.c_str());
    }

    if (!m_pScene->mRootNode)
    {
        LogError(L"[Model] 缺少根节点，无法解析模型: %ls. \n", fullPath.c_str());
        return FALSE;
    }

    // 3. 获取模型目录
    size_t lastSlash = fullPath.find_last_of(L"/\\");
    m_directory = (lastSlash != std::wstring::npos) ? fullPath.substr(0, lastSlash) : L"";
    // 模型目录 m_directory = res/Models/Duck

    // 4. 递归处理节点
    m_meshes.reserve(m_pScene->mNumMeshes); // 预分配内存
    ProcessNode(m_pScene->mRootNode, m_pScene, pResMgr);

    if (m_meshes.empty())
    {
        LogWarning(L"当前路径: %ls 下没有网格加载", fullPath);
        return FALSE;
    }

    // 5. 计算边界框
    size_t lastDot = fullPath.find_last_of(L".");
    if (lastSlash != std::wstring::npos && lastDot != std::wstring::npos && lastDot > lastSlash)
        m_name = fullPath.substr(lastSlash + 1, lastDot - lastSlash - 1);
    else
        m_name = fullPath;

    // LogDebug(L"模型加载成功: %ls, 网格数: %d.\n", m_name.c_str(), (int)m_meshes.size());

    return TRUE;
}

void CModel::Unload()
{
    m_meshes.clear();
    m_directory.clear();
    m_name.clear();

    // 重置统计数据
    m_totalVertices = 0;
    m_totalTriangles = 0;

    m_minBounds = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    m_maxBounds = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    m_center = Vector3::Zero();
    m_radius = 0.0f;

    m_position = Vector3::Zero();
    m_rotation = Quaternion::Identity();
    m_scale = Vector3(1, 1, 1);
    m_isDirty = true;
    m_transform = Matrix4::Identity();
}

void CModel::AddMesh(std::shared_ptr<CMesh> pMesh)
{
    if (!pMesh)
        return;

    m_meshes.push_back(pMesh);

    // 累加统计数据
    m_totalVertices += pMesh->GetVertexCount();
    m_totalTriangles += pMesh->GetTriangleCount();

    // 只需要拿 CMesh 的 Min/Max 更新 CModel 的 Min/Max
    const Vector3 &meshMin = pMesh->GetMinBounds();
    const Vector3 &meshMax = pMesh->GetMaxBounds();

    if (m_meshes.size() == 1)
    {
        m_minBounds = meshMin;
        m_maxBounds = meshMax;
    }
    else
    {
        m_minBounds.x = Math::Min(m_minBounds.x, meshMin.x);
        m_minBounds.y = Math::Min(m_minBounds.y, meshMin.y);
        m_minBounds.z = Math::Min(m_minBounds.z, meshMin.z);

        m_maxBounds.x = Math::Max(m_maxBounds.x, meshMax.x);
        m_maxBounds.y = Math::Max(m_maxBounds.y, meshMax.y);
        m_maxBounds.z = Math::Max(m_maxBounds.z, meshMax.z);
    }

    // 更新模型中心和半径
    m_center = (m_minBounds + m_maxBounds) * 0.5f;
    m_radius = (m_maxBounds - m_minBounds).Length() * 0.5f;
}

void CModel::ProcessNode(aiNode *node, const aiScene *scene, CResourceManager *pResMgr)
{
    // 处理当前节点的所有网格
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        unsigned int meshIndex = node->mMeshes[i];
        if (meshIndex < scene->mNumMeshes)
        {
            aiMesh *mesh = scene->mMeshes[meshIndex];

            auto pMesh = ProcessMesh(mesh, scene, pResMgr);
            if (pMesh)
            {
                // m_meshes.push_back(pMesh);
                AddMesh(pMesh);
            }
        }
    }

    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, pResMgr);
    }
}

std::shared_ptr<CMesh> CModel::ProcessMesh(aiMesh *mesh, const aiScene *scene, CResourceManager *pResMgr)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    vertices.reserve(mesh->mNumVertices);
    indices.reserve(mesh->mNumFaces * 3); // 假设都是三角形

    // 1. 提取顶点数据
    vertices.resize(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex &vertex = vertices[i];

        // 位置
        vertex.Position = CMathConverter::ToVector3(mesh->mVertices[i]);
        // --- 新增：存储原始位置，用于 CPU 蒙皮计算 ---

        if (mesh->HasNormals())
        {
            vertex.Normal = CMathConverter::ToVector3(mesh->mNormals[i]);
        }
        else
        {
            vertex.Normal = Vector3(0.0f, 1.0f, 0.0f);
        }

        vertex.BasePosition = vertex.Position;
        vertex.BaseNormal = vertex.Normal;

        // 纹理坐标
        if (mesh->mTextureCoords[0])
        {
            const aiVector3D &texCoord = mesh->mTextureCoords[0][i];
            vertex.TexCoords.x = texCoord.x;
            vertex.TexCoords.y = texCoord.y;
        }
        else
        {
            vertex.TexCoords = Vector2(0.0f, 0.0f);
        }

        // --- 新增：初始化骨骼数据，默认 -1 表示不受骨骼影响 ---
        for (int j = 0; j < 4; j++)
        {
            vertex.BoneIDs[j] = -1;
            vertex.Weights[j] = 0.0f;
        }
    }

    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        aiBone *bone = mesh->mBones[i];
        std::string boneName = bone->mName.C_Str();
        int boneID = -1;

        // 如果映射表里没有这根骨骼，就分配一个新的全局 ID
        if (m_BoneMapping.find(boneName) == m_BoneMapping.end())
        {
            boneID = (int)m_BoneMapping.size();
            BoneInfo bi;
            bi.id = boneID;
            bi.offset = CMathConverter::ToMatrix4(bone->mOffsetMatrix); // 转换偏移矩阵
            m_BoneMapping[boneName] = bi;
        }
        else
        {
            boneID = m_BoneMapping[boneName].id;
        }

        // 遍历这根骨骼影响的所有顶点
        for (unsigned int j = 0; j < bone->mNumWeights; j++)
        {
            unsigned int vertexID = bone->mWeights[j].mVertexId;
            float weight = bone->mWeights[j].mWeight;

            // 把骨骼 ID 和权重填入顶点的空槽位（最多4个）
            for (int k = 0; k < 4; k++)
            {
                if (vertices[vertexID].BoneIDs[k] == -1) // 找到空位
                {
                    vertices[vertexID].BoneIDs[k] = boneID;
                    vertices[vertexID].Weights[k] = weight;
                    break;
                }
            }
        }
    }

    // 2. 提取索引数据
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace &face = mesh->mFaces[i];
        if (face.mNumIndices == 3) // 确保是三角形
        {
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }
    }

    // 3. 如果没有纹理坐标，但模型有顶点颜色，可以存储颜色
    if (!mesh->mTextureCoords[0] && mesh->HasVertexColors(0))
    {
        // 可以存储顶点颜色到自定义属性
    }

    // 4. 处理材质贴图
    std::shared_ptr<CTexture> pTex = nullptr;

    CMesh::SimpleMaterial material; // 默认材质

    if (mesh->mMaterialIndex >= 0)
    {
        aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];

        // 加载漫反射贴图
        pTex = LoadMaterialTexture(mat, aiTextureType_DIFFUSE, pResMgr);

        // 如果没有漫反射贴图，尝试其他类型
        if (!pTex)
        {
            pTex = LoadMaterialTexture(mat, aiTextureType_AMBIENT, pResMgr);
        }

        // 获取材质名称
        aiString matName;
        if (mat->Get(AI_MATKEY_NAME, matName) == AI_SUCCESS)
        {
            material.name = CStringUtils::StringToWString(matName.C_Str());
        }

        // 获取材质颜色
        aiColor3D color(0.0f, 0.0f, 0.0f);

        // 漫反射颜色
        if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
        {
            // LogDebug(L"Material: %ls, R: %f, G: %f, B: %f.\n", material.name.c_str(), color.r, color.g, color.b);
            material.diffuse = Vector3(color.r, color.g, color.b);
        }

        // 镜面反射颜色
        if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
        {
            material.specular = Vector3(color.r, color.g, color.b);
        }

        // 环境光颜色
        if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
        {
            material.ambient = Vector3(color.r, color.g, color.b);
        }

        // 高光指数
        if (mat->Get(AI_MATKEY_SHININESS, material.shininess) != AI_SUCCESS)
        {
            material.shininess = 32.0f;
        }

        // 透明度
        float opacity = 1.0f;
        if (mat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS)
        {
            material.opacity = opacity;
        }

        // 高光强度
        float shininessStrength = 1.0f;
        if (mat->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength) == AI_SUCCESS)
        {
            material.specular *= shininessStrength;
        }
    }

    // 6. 创建网格
    auto pMesh = std::make_shared<CMesh>(vertices, indices, pTex);

    // 7. 设置材质
    pMesh->SetMaterial(material);

    // 创建网格ID
    pMesh->SetSubMeshID(mesh->mMaterialIndex);

    return pMesh;
}

std::shared_ptr<CTexture> CModel::LoadMaterialTexture(aiMaterial *mat, aiTextureType type, CResourceManager *pResMgr)
{
    if (!mat || !pResMgr)
        return nullptr;

    if (mat->GetTextureCount(type) <= 0)
        return nullptr;

    aiString str;
    if (mat->GetTexture(type, 0, &str) != AI_SUCCESS)
        return nullptr;

    std::string fileName = str.C_Str();
    if (fileName.empty())
        return nullptr;

    // 转换为宽字符串
    std::wstring wFileName(fileName.begin(), fileName.end());

    // 处理嵌入纹理
    if (fileName[0] == '*')
    {
        LogWarning(L"嵌入式纹理不支持: %ls", wFileName.c_str());
        return nullptr;
    }

    // 获取文件名
    size_t lastSlash = wFileName.find_last_of(L"/\\");
    if (lastSlash != std::wstring::npos)
    {
        wFileName = wFileName.substr(lastSlash + 1);
    }

    // m_directory = res/Models/Duck
    std::wstring fullPath = m_directory + L"/" + wFileName;

    // FIXME: 将全路径转化为相对路径
    // fullPath = res/Models/Duck/default.png
    // 这里要做一个转化 将全路径转化为相对路径

    return pResMgr->GetTexture(fullPath, CResourceManager::PathType::Absolute);
}

void CModel::Draw() const
{
    if (m_meshes.empty())
        return;

    // 保存所有相关状态
    glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);

    // 保存当前矩阵
    glPushMatrix();

    // 应用模型变换
    const Matrix4 &worldMat = GetWorldMatrix();
    glMultMatrixf(worldMat.GetData());

    for (const auto &mesh : m_meshes)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);

        mesh->Draw();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    // 恢复矩阵
    glPopMatrix();
    glPopAttrib(); // 恢复纹理状态
}

void CModel::SetPosition(const Vector3 &position)
{
    m_position = position;
    m_isDirty = true;
}

void CModel::SetRotation(const Vector3 &eulerAngles)
{
    m_rotation = Quaternion::FromEuler(eulerAngles.x, eulerAngles.y, eulerAngles.z);
    m_isDirty = true;
}

void CModel::SetScale(const Vector3 &scale)
{
    m_scale = scale;
    m_isDirty = true;
}

const Matrix4 &CModel::GetWorldMatrix() const
{
    if (m_isDirty)
    {
        // 矩阵合成顺序：缩放 -> 旋转 -> 平移 (TRS)
        // 注意：矩阵乘法顺序取决于你的 Matrix4 实现，通常是 T * R * S
        m_transform = Matrix4::TRS(m_position, m_rotation, m_scale);
        m_invTransform = m_transform.Inversed(); // 预先算好逆矩阵
        m_isDirty = FALSE;
    }
    return m_transform;
}

void CModel::CalculateBoundingBox()
{
    m_minBounds = Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
    m_maxBounds = Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

    if (m_meshes.empty())
    {
        m_center = Vector3::Zero();
        m_radius = 0.0f;
        return;
    }

    for (const auto &mesh : m_meshes)
    {
        // 调用上面刚写的 AddMesh 逻辑中的更新部分即可
        const Vector3 &mMin = mesh->GetMinBounds();
        const Vector3 &mMax = mesh->GetMaxBounds();

        m_minBounds.x = Math::Min(m_minBounds.x, mMin.x);
        m_minBounds.y = Math::Min(m_minBounds.y, mMin.y);
        m_minBounds.z = Math::Min(m_minBounds.z, mMin.z);

        m_maxBounds.x = Math::Max(m_maxBounds.x, mMax.x);
        m_maxBounds.y = Math::Max(m_maxBounds.y, mMax.y);
        m_maxBounds.z = Math::Max(m_maxBounds.z, mMax.z);
    }

    m_center = (m_minBounds + m_maxBounds) * 0.5f;
    m_radius = (m_maxBounds - m_minBounds).Length() * 0.5f;
}

BOOL CModel::IsPointInside(const Vector3 &point) const
{
    Matrix4 invWorld = GetWorldMatrix().Inversed();

    Vector3 localPoint = invWorld * point;
    // Vector3 localPoint = GetWorldMatrix().Invert().MultiplyPoint(point);

    // 简单AABB检测
    return (localPoint.x >= m_minBounds.x && localPoint.x <= m_maxBounds.x &&
            localPoint.y >= m_minBounds.y && localPoint.y <= m_maxBounds.y &&
            localPoint.z >= m_minBounds.z && localPoint.z <= m_maxBounds.z);
}

BOOL CModel::IntersectsSphere(const Vector3 &center, float radius) const
{
    Vector3 closestPoint;

    // 找到AABB上离球心最近的点
    closestPoint.x = std::max(m_minBounds.x, std::min(center.x, m_maxBounds.x));
    closestPoint.y = std::max(m_minBounds.y, std::min(center.y, m_maxBounds.y));
    closestPoint.z = std::max(m_minBounds.z, std::min(center.z, m_maxBounds.z));

    // 计算距离
    Vector3 difference = center - closestPoint;
    float distanceSquared = difference.LengthSquared();

    return distanceSquared <= (radius * radius);
}

void CModel::UpdateBoundingBox(const Vector3 &point)
{
    m_minBounds.x = std::min(m_minBounds.x, point.x);
    m_minBounds.y = std::min(m_minBounds.y, point.y);
    m_minBounds.z = std::min(m_minBounds.z, point.z);

    m_maxBounds.x = std::max(m_maxBounds.x, point.x);
    m_maxBounds.y = std::max(m_maxBounds.y, point.y);
    m_maxBounds.z = std::max(m_maxBounds.z, point.z);

    // 标记需要更新 Center 和 Radius
    m_center = (m_minBounds + m_maxBounds) * 0.5f;
    m_radius = (m_maxBounds - m_minBounds).Length() * 0.5f;
}

void CModel::DrawBoundingBox(const Vector3 &color) const
{
    glPushMatrix();

    // 应用模型的世界变换，这样包围盒会跟着模型走
    glMultMatrixf(GetWorldMatrix().GetData());

    // 禁用灯光，确保调试线段颜色纯正
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);

    glColor3f(color.x, color.y, color.z);

    glBegin(GL_LINES);
    {
        // 底面
        glVertex3f(m_minBounds.x, m_minBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_minBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_minBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_minBounds.y, m_maxBounds.z);
        glVertex3f(m_maxBounds.x, m_minBounds.y, m_maxBounds.z);
        glVertex3f(m_minBounds.x, m_minBounds.y, m_maxBounds.z);
        glVertex3f(m_minBounds.x, m_minBounds.y, m_maxBounds.z);
        glVertex3f(m_minBounds.x, m_minBounds.y, m_minBounds.z);

        // 顶面
        glVertex3f(m_minBounds.x, m_maxBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_maxBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_maxBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_maxBounds.y, m_maxBounds.z);
        glVertex3f(m_maxBounds.x, m_maxBounds.y, m_maxBounds.z);
        glVertex3f(m_minBounds.x, m_maxBounds.y, m_maxBounds.z);
        glVertex3f(m_minBounds.x, m_maxBounds.y, m_maxBounds.z);
        glVertex3f(m_minBounds.x, m_maxBounds.y, m_minBounds.z);

        // 连接上下面的 4 条垂直线
        glVertex3f(m_minBounds.x, m_minBounds.y, m_minBounds.z);
        glVertex3f(m_minBounds.x, m_maxBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_minBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_maxBounds.y, m_minBounds.z);
        glVertex3f(m_maxBounds.x, m_minBounds.y, m_maxBounds.z);
        glVertex3f(m_maxBounds.x, m_maxBounds.y, m_maxBounds.z);
        glVertex3f(m_minBounds.x, m_minBounds.y, m_maxBounds.z);
        glVertex3f(m_minBounds.x, m_maxBounds.y, m_maxBounds.z);
    }
    glEnd();

    glPopAttrib();
    glPopMatrix();
}

void CModel::DrawNormals(float scale, unsigned int step)
{
    glPushMatrix();
    // 假设你的 CModel 有应用变换的逻辑，或者在 Entity 层处理
    // ApplyTransform();

    for (auto &mesh : m_meshes)
    {
        mesh->DrawNormals(scale, step);
    }

    glPopMatrix();
}

void CModel::UpdateAnimation(float timeInSeconds)
{
    if (!m_pScene || !m_pScene->HasAnimations())
        return;

    // 1. 计算当前动画的时间点
    float TicksPerSecond = (float)(m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
    float TimeInTicks = timeInSeconds * TicksPerSecond;
    float AnimationTime = fmod(TimeInTicks, (float)m_pScene->mAnimations[0]->mDuration);

    // 2. 递归计算所有骨骼的当前矩阵 (ReadNodeHierarchy)
    // 这里会得到一个 FinalMatrices 数组
    Matrix4 Identity;
    ReadNodeHierarchy(AnimationTime, m_pScene->mRootNode, Identity);

    // 3. CPU 蒙皮：手动更新每个 Mesh 的顶点
    for (auto &mesh : m_meshes)
    {
        // 获取顶点的引用以便修改
        auto &vertices = const_cast<std::vector<Vertex> &>(mesh->GetVertices());

        for (auto &v : vertices)
        {
            // 存储混合后的 3x4 变换矩阵（法线不需要第四行，位置只需要前三行的结果）
            // 初始化为全 0
            float r00 = 0, r01 = 0, r02 = 0, r03 = 0;
            float r10 = 0, r11 = 0, r12 = 0, r13 = 0;
            float r20 = 0, r21 = 0, r22 = 0, r23 = 0;

            // 混合四个骨骼的矩阵分量
            for (int i = 0; i < 4; i++)
            {
                if (v.BoneIDs[i] == -1 || v.Weights[i] <= 0.0f)
                    continue;

                float w = v.Weights[i];
                const Matrix4 &b = m_FinalMatrices[v.BoneIDs[i]];

                r00 += b.m00 * w;
                r01 += b.m01 * w;
                r02 += b.m02 * w;
                r03 += b.m03 * w;
                r10 += b.m10 * w;
                r11 += b.m11 * w;
                r12 += b.m12 * w;
                r13 += b.m13 * w;
                r20 += b.m20 * w;
                r21 += b.m21 * w;
                r22 += b.m22 * w;
                r23 += b.m23 * w;
            }

            // --- 计算新位置 (包含矩阵的位移列 r03, r13, r23) ---
            v.Position.x = r00 * v.BasePosition.x + r01 * v.BasePosition.y + r02 * v.BasePosition.z + r03;
            v.Position.y = r10 * v.BasePosition.x + r11 * v.BasePosition.y + r12 * v.BasePosition.z + r13;
            v.Position.z = r20 * v.BasePosition.x + r21 * v.BasePosition.y + r22 * v.BasePosition.z + r23;

            // --- 计算新法线 (不计位移，只计旋转缩放部分) ---
            v.Normal.x = r00 * v.BaseNormal.x + r01 * v.BaseNormal.y + r02 * v.BaseNormal.z;
            v.Normal.y = r10 * v.BaseNormal.x + r11 * v.BaseNormal.y + r12 * v.BaseNormal.z;
            v.Normal.z = r20 * v.BaseNormal.x + r21 * v.BaseNormal.y + r22 * v.BaseNormal.z;

            // 建议：如果你的 Vector3 有 Normalize 方法，最好加上，否则光照会随着模型缩放变奇怪
            // v.Normal.Normalize();
        }
    }
    CalculateBoundingBox();
}

void CModel::ReadNodeHierarchy(float AnimationTime, const aiNode *pNode, const Matrix4 &ParentTransform)
{
    std::string NodeName(pNode->mName.data);
    const aiAnimation *pAnim = m_pScene->mAnimations[0];
    Matrix4 NodeTransformation = CMathConverter::ToMatrix4(pNode->mTransformation);

    // 查找当前节点是否有动画信息
    const aiNodeAnim *pNodeAnim = nullptr;
    for (unsigned int i = 0; i < pAnim->mNumChannels; i++)
    {
        if (std::string(pAnim->mChannels[i]->mNodeName.data) == NodeName)
        {
            pNodeAnim = pAnim->mChannels[i];
            break;
        }
    }

    if (pNodeAnim)
    {
        aiVector3D Translation = pNodeAnim->mPositionKeys[0].mValue;
        for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
        {
            if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime)
            {
                Translation = pNodeAnim->mPositionKeys[i].mValue;
                break;
            }
        }

        // 2. 计算旋转 (同理)
        aiQuaternion Rotation = pNodeAnim->mRotationKeys[0].mValue;
        for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
        {
            if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime)
            {
                Rotation = pNodeAnim->mRotationKeys[i].mValue;
                break;
            }
        }

        // 3. 构造变换矩阵 (需要你的 Matrix4 支持从 Quaternion 和 Translation 构造)
        Matrix4 matRotation = CMathConverter::ToMatrix4(Rotation.GetMatrix());
        Matrix4 matTranslation = Matrix4::Translation(Vector3(Translation.x, Translation.y, Translation.z));

        NodeTransformation = matTranslation * matRotation;
    }

    Matrix4 GlobalTransformation = ParentTransform * NodeTransformation;

    // 如果该节点是骨骼，计算最终矩阵：Global * Offset
    if (m_BoneMapping.find(NodeName) != m_BoneMapping.end())
    {
        unsigned int BoneIndex = m_BoneMapping[NodeName].id;
        m_FinalMatrices[BoneIndex] = GlobalTransformation * m_BoneMapping[NodeName].offset;
    }

    // 递归子节点
    for (unsigned int i = 0; i < pNode->mNumChildren; i++)
    {
        ReadNodeHierarchy(AnimationTime, pNode->mChildren[i], GlobalTransformation);
    }
}