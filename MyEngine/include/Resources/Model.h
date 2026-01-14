
// ======================================================================
#ifndef __MODEL_H__
#define __MODEL_H__
// ======================================================================
#include <string>
#include <vector>
#include <memory>
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "Resources/Mesh.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
// ======================================================================

class CResourceManager;

class CModel
{
public:
    CModel() = default;
    ~CModel() = default;

    // 加载模型入口
    BOOL LoadFromFile(const std::wstring &filePath, CResourceManager *pResMgr);
    void Unload();

    void Draw() const; // 渲染模型（遍历所有网格绘制）

    void SetPosition(const Vector3 &position);
    void SetRotation(const Vector3 &eulerAngles);
    void SetScale(const Vector3 &scale);
    const Matrix4& GetWorldMatrix() const;

private:
    // 递归处理 Assimp 节点
    void ProcessNode(aiNode *node, const aiScene *scene, CResourceManager *pResMgr);

    // 将 Assimp 的网格转换为我们的 CMesh
    std::shared_ptr<CMesh> ProcessMesh(aiMesh *mesh, const aiScene *scene, CResourceManager *pResMgr);

    // 加载材质贴图
    std::shared_ptr<CTexture> LoadMaterialTexture(aiMaterial *mat, aiTextureType type, CResourceManager *pResMgr);
    // 转换网格数据
    std::shared_ptr<CMesh> ProcessMesh(aiMesh *mesh, const aiScene *scene);

    std::vector<std::shared_ptr<CMesh>> m_meshes; // 一个模型由多个网格组成
    std::wstring m_directory;                     // 方便寻找贴图的目录

    Vector3 m_position;
    Quaternion m_rotation; // 建议用四元数，避免万向节死锁
    Vector3 m_scale = Vector3(1, 1, 1);

    mutable Matrix4 m_transform;   // 模型变换矩阵
    mutable BOOL m_isDirty = true; // 标记是否需要重新计算

    // 边界框
    Vector3 m_minBounds;
    Vector3 m_maxBounds;
    Vector3 m_center;
    float m_radius = 0.0f;
};
#endif // __MODEL_H__