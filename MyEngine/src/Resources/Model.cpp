
// ======================================================================
#include "stdafx.h"
#include <iostream>
#include "Resources/Model.h"
#include "Resources/ResourceManager.h"
#include "Math/MathConverter.h" // 之前编写的转换类
#include "Utils/StringUtils.h"
// ======================================================================

BOOL CModel::LoadFromFile(const std::wstring &filePath, CResourceManager *pResMgr)
{
    Assimp::Importer importer;

    // 1. 将 wstring 转为 string (Assimp 接口要求)
    // 这里可以使用之前提到的 CStringUtils 或简单的转换

    std::string pathStr = CStringUtils::WStringToString(filePath);

    // 2. 读取文件并进行预处理
    // 2. 优化导入标志
    unsigned int flags =
        aiProcess_Triangulate |           // 转为三角形
        aiProcess_FlipUVs |               // 翻转纹理坐标
        aiProcess_GenSmoothNormals |      // 生成平滑法线
        aiProcess_CalcTangentSpace |      // 计算切线空间（如果需要法线贴图）
        aiProcess_JoinIdenticalVertices | // 合并重复顶点
        aiProcess_ImproveCacheLocality |  // 优化缓存局部性
        aiProcess_ValidateDataStructure | // 验证数据结构
        aiProcess_OptimizeMeshes;         // 优化网格

    const aiScene *scene = importer.ReadFile(pathStr, flags);

    if (!scene)
    {
        std::string error = "Assimp Error: " + std::string(importer.GetErrorString());
        OutputDebugStringA(error.c_str());
        return FALSE;
    }

    if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        OutputDebugStringA("[Model] Warning: Scene is incomplete\n");
    }

    if (!scene->mRootNode)
    {
        OutputDebugStringA("[Model] Error: No root node\n");
        return FALSE;
    }

    // 3. 获取模型目录
    size_t lastSlash = filePath.find_last_of(L"/\\");
    m_directory = (lastSlash != std::wstring::npos) ? filePath.substr(0, lastSlash) : L"";

    // 4. 递归处理节点
    m_meshes.reserve(scene->mNumMeshes); // 预分配内存
    ProcessNode(scene->mRootNode, scene, pResMgr);

    return true;
}

void CModel::Unload()
{
    m_meshes.clear();
    m_directory.clear();
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
                m_meshes.push_back(pMesh);
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

        // 法线
        if (mesh->HasNormals())
        {
            vertex.Normal = CMathConverter::ToVector3(mesh->mNormals[i]);
        }
        else
        {
            // 生成默认法线
            vertex.Normal = Vector3(0.0f, 1.0f, 0.0f);
        }

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

    // 4. 处理材质（贴图）
    std::shared_ptr<CTexture> pTex = nullptr;
    std::shared_ptr<CTexture> pSpecularTex = nullptr;
    std::shared_ptr<CTexture> pNormalTex = nullptr;

    if (mesh->mMaterialIndex >= 0 && pResMgr)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // 尝试加载漫反射贴图
        pTex = LoadMaterialTexture(material, aiTextureType_DIFFUSE, pResMgr);

        // 如果没有漫反射贴图，尝试其他类型
        if (!pTex)
        {
            pTex = LoadMaterialTexture(material, aiTextureType_AMBIENT, pResMgr);
        }

        // 加载高光贴图
        pSpecularTex = LoadMaterialTexture(material, aiTextureType_SPECULAR, pResMgr);

        // 加载法线贴图
        pNormalTex = LoadMaterialTexture(material, aiTextureType_NORMALS, pResMgr);
        if (!pNormalTex)
        {
            pNormalTex = LoadMaterialTexture(material, aiTextureType_HEIGHT, pResMgr);
        }
    }

    // 6. 创建网格
    auto pMesh = std::make_shared<CMesh>(vertices, indices, pTex);

    // 7. 可以设置其他材质属性
    if (pMesh && mesh->mMaterialIndex >= 0 && pResMgr)
    {
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        // 获取材质颜色
        aiColor3D diffuse(0.f, 0.f, 0.f);
        aiColor3D specular(0.f, 0.f, 0.f);
        aiColor3D ambient(0.f, 0.f, 0.f);
        float shininess = 0.0f;

        material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
        material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
        material->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
        material->Get(AI_MATKEY_SHININESS, shininess);

        // 可以将这些值存储到网格中
        // pMesh->SetMaterialProperties(diffuse, specular, ambient, shininess);
    }

    return std::make_shared<CMesh>(vertices, indices, pTex);
}

std::shared_ptr<CTexture> CModel::LoadMaterialTexture(aiMaterial *mat, aiTextureType type, CResourceManager *pResMgr)
{
    if (mat->GetTextureCount(type) > 0)
    {
        aiString str;
        mat->GetTexture(type, 0, &str);

        // INFO: 将 Assimp 路径转为相对路径（假设贴图在模型同级目录）
        std::string fileName = str.C_Str();
        std::wstring wFileName(fileName.begin(), fileName.end());

        // 从资源管理器获取贴图
        return pResMgr->GetTexture(wFileName);
    }
    return nullptr;
}

void CModel::Draw() const
{
    for (const auto &mesh : m_meshes)
    {
        mesh->Draw();
    }
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

const Matrix4& CModel::GetWorldMatrix() const {
    if (m_isDirty) {
        // 矩阵合成顺序：缩放 -> 旋转 -> 平移 (TRS)
        // 注意：矩阵乘法顺序取决于你的 Matrix4 实现，通常是 T * R * S
        m_transform = Matrix4::Translation(m_position) * m_rotation.ToMatrix() * Matrix4::Scale(m_scale);
        m_isDirty = false;
    }
    return m_transform;
}