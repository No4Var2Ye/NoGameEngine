
// ======================================================================
#include "stdafx.h"
#include "Resources/ResourceManager.h"
#include "Resources/Texture.h"
#include "Resources/Model.h"
#include "Utils/StringUtils.h"
// ======================================================================

BOOL CResourceManager::Initialize(const ResourceConfig &config)
{
    // 存储配置
    m_Config = config;

    // 预先清空容器
    m_Textures.clear();
    m_Models.clear();
    m_Shaders.clear();

    // 加载兜底资源
    if (!CreateDefaultResources())
    {
        LogError(L"创建引擎默认资源失败\n");
        return FALSE;
    }

    return TRUE;
}

void CResourceManager::Shutdown()
{
    // 1. 释放兜底资源
    // 如果不置空，即使容器清空了，它依然会占着显存
    m_DefaultTexture.reset();

    // 2. 清空所有弱引用容器
    m_Textures.clear();
    m_Models.clear();
    m_Shaders.clear();

    // 3. 可以在这里打印一份资源残留报告
    // 如果此时还有资源没释放，说明代码里有地方产生了“内存泄漏”（循环引用或没删除的shared_ptr）

    LogInfo(L"========= 资源管理器关闭成功 =========\n");
}

void CResourceManager::ReleaseUnusedResources()
{
    // 遍历纹理缓存
    for (auto it = m_Textures.begin(); it != m_Textures.end();)
    {
        // 如果资源已经失效（引用计数为0）
        if (it->second.expired())
        {
            it = m_Textures.erase(it); // 移除无效条目
        }
        else
        {
            ++it;
        }
    }

    // 清理模型
    for (auto it = m_Models.begin(); it != m_Models.end();)
    {
        if (it->second.expired())
            it = m_Models.erase(it);
        else
            ++it;
    }

    // Shader 通常生命周期贯穿始终，也可以清理，但通常没那么多
    // for (auto it = m_Shaders.begin(); it != m_Shaders.end();)
    // {
    //     if (it->second.expired())
    //         it = m_Shaders.erase(it);
    //     else
    //         ++it;
    // }

    OutputDebugStringW(L"[ResMgr]: 已清理过期资源引用。\n");
}

BOOL CResourceManager::CreateDefaultResources()
{
    LogInfo(L"开始创建默认资源...\n");

    // 1. 创建默认纹理
    m_DefaultTexture = CreateDefaultTexture();
    if (!m_DefaultTexture || !m_DefaultTexture->IsValid())
    {
        LogError(L"创建默认纹理失败\n");
        return FALSE;
    }
    m_DefaultTexture->SetWrapMode(GL_REPEAT, GL_REPEAT);
    m_DefaultTexture->SetFilterMode(GL_LINEAR, GL_LINEAR);

    // 2. 创建默认模型
    m_DefaultModel = CreateDefaultModel();
    if (!m_DefaultModel)
    {
        LogError(L"创建默认模型失败\n");
        return FALSE;
    }

    // 3. 创建默认着色器（如果需要的话）
    // m_DefaultShader = CreateDefaultShader();
    // if (!m_DefaultShader)
    // {
    //     LogWarning(L"创建默认着色器失败（着色器是可选的）\n");
    // }

    LogInfo(L"默认资源创建成功！\n");
    LogInfo(L"  - 默认纹理: %dx%d\n",
            m_DefaultTexture->GetWidth(), m_DefaultTexture->GetHeight());
    // LogInfo(L"  - 默认模型: 顶点数=%zu, 三角形数=%zu\n",
    //    m_DefaultModel->GetVertexCount(), m_DefaultModel->GetTriangleCount());

    return TRUE;
}

BOOL CResourceManager::RecreateDefaultResources()
{
    LogInfo(L"重新创建默认资源...\n");

    // 清理旧的兜底资源
    m_DefaultTexture.reset();
    m_DefaultModel.reset();
    m_DefaultShader.reset();

    // 重新创建
    return CreateDefaultResources();
}

std::shared_ptr<CTexture> CResourceManager::CreateDefaultTexture()
{
    // 创建明显的棋盘格纹理，便于调试
    const int size = 64;
    std::vector<unsigned char> data(size * size * 4); // RGBA

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            int idx = (y * size + x) * 4;
            bool isPink = ((x / 8) + (y / 8)) % 2 == 0;

            if (isPink)
            {
                data[idx] = 255;     // R: 品红
                data[idx + 1] = 0;   // G
                data[idx + 2] = 255; // B
                data[idx + 3] = 255; // A: 不透明
            }
            else
            {
                data[idx] = 0;       // R: 黑色
                data[idx + 1] = 0;   // G
                data[idx + 2] = 0;   // B
                data[idx + 3] = 255; // A
            }
        }
    }

    auto texture = std::make_shared<CTexture>();
    if (texture->LoadFromMemory(data.data(), size, size, 4, GL_RGBA))
    {
        // 给纹理一个描述性的名字
        // texture->SetPath(L"[DefaultTexture]");
        return texture;
    }

    LogError(L"从内存创建默认纹理失败\n");
    return nullptr;
}

std::shared_ptr<CModel> CResourceManager::CreateDefaultModel()
{
    // 返回一个立方体作为默认模型
    return CreateCubeModel();
}

std::shared_ptr<CTexture> CResourceManager::GetTexture(const std::wstring &filepath, PathType pathType)
{
    // 提取文件名
    std::wstring fileName = filepath;
    size_t lastSlash = filepath.find_last_of(L"/\\");
    if (lastSlash != std::wstring::npos)
    {
        fileName = filepath.substr(lastSlash + 1);
    }

    // 1. 缓存查找逻辑 (lock weak_ptr)...
    auto it = m_Textures.find(filepath);
    if (it != m_Textures.end())
    {
        // 尝试提升为 shared_ptr
        if (auto shared = it->second.lock())
        {
            return shared; // 资源还在内存中，直接复用
        }
        else
        {
            // 资源已经销毁，但 map 的 key 还在，需要移除无效节点
            m_Textures.erase(it);
        }
    }
    // 2. 如果没找到，加载默认材质
    auto newTex = std::make_shared<CTexture>();

    std::wstring fullPath;
    if (pathType == PathType::Relative)
    {
        fullPath = m_Config.GetTexturePath() + filepath;
    }
    else
    {
        // 走传入的原始路径: res/Models/Duck/DuckCM.png
        // 这样就不会出现 res/Textures/res/Models/... 的套娃问题
        fullPath = filepath;
    }

    // 3. 执行加载
    if (newTex->LoadFromFile(fullPath))
    {
        m_Textures[fileName] = newTex;
        return newTex;
    }

    LogWarning(L"加载纹理失败: %ls. 使用默认样式. \n", fileName.c_str());

    return m_DefaultTexture;
}

std::shared_ptr<CModel> CResourceManager::GetModel(const std::wstring &filepath, PathType pathType)
{
    // 提取文件名
    std::wstring fileName = filepath;
    size_t lastSlash = filepath.find_last_of(L"/\\");
    if (lastSlash != std::wstring::npos)
    {
        fileName = filepath.substr(lastSlash + 1);
    }
    // filename = Duck.obj

    // 1. 缓存查找
    auto it = m_Models.find(filepath);
    if (it != m_Models.end())
    {
        if (auto shared = it->second.lock())
        {
            return shared; // 命中缓存
        }
        else
        {
            m_Models.erase(it); // 清理过期的弱引用
        }
    }

    // 2. 加载新模型
    auto newModel = std::make_shared<CModel>();

    // 拼接完整路径：res/Models/fileName // fileName = Duck/Duck.obj
    // exp: fullPath = res/Models/Duck/Duck.obj
    std::wstring fullPath = m_Config.GetModelPath() + filepath;

    // 传入 this，允许 CModel 在加载过程中调用 GetTexture
    if (newModel->LoadFromFile(fullPath, this))
    {
        m_Models[fileName] = newModel;
        return newModel;
    }

    // 3. 如果模型加载失败
    LogError(L"无法加载模型文件: %ls, 使用默认模型.\n", fullPath.c_str());

    // 确保默认模型存在
    if (!m_DefaultModel)
    {
        m_DefaultModel = CreateDefaultModel();
        if (!m_DefaultModel)
        {
            LogError(L"创建默认模型也失败了! \n");
            return nullptr;
        }
    }

    return m_DefaultModel;
}

std::shared_ptr<CModel> CResourceManager::CreateCubeModel()
{
    // 创建立方体顶点数据
    std::vector<Vertex> vertices = {
        // 前面 (Z+)
        {Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 0.0f)},
        {Vector3(0.5f, -0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 0.0f)},
        {Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f)},
        {Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.0f, 0.0f, 1.0f), Vector2(0.0f, 1.0f)},

        // 后面 (Z-)
        {Vector3(0.5f, -0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 0.0f)},
        {Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 0.0f)},
        {Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f), Vector2(1.0f, 1.0f)},
        {Vector3(0.5f, 0.5f, -0.5f), Vector3(0.0f, 0.0f, -1.0f), Vector2(0.0f, 1.0f)},

        // 右面 (X+)
        {Vector3(0.5f, -0.5f, 0.5f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)},
        {Vector3(0.5f, -0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)},
        {Vector3(0.5f, 0.5f, -0.5f), Vector3(1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)},
        {Vector3(0.5f, 0.5f, 0.5f), Vector3(1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)},

        // 左面 (X-)
        {Vector3(-0.5f, -0.5f, -0.5f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f)},
        {Vector3(-0.5f, -0.5f, 0.5f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 0.0f)},
        {Vector3(-0.5f, 0.5f, 0.5f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(1.0f, 1.0f)},
        {Vector3(-0.5f, 0.5f, -0.5f), Vector3(-1.0f, 0.0f, 0.0f), Vector2(0.0f, 1.0f)},

        // 顶面 (Y+)
        {Vector3(-0.5f, 0.5f, 0.5f), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 0.0f)},
        {Vector3(0.5f, 0.5f, 0.5f), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f)},
        {Vector3(0.5f, 0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f), Vector2(1.0f, 1.0f)},
        {Vector3(-0.5f, 0.5f, -0.5f), Vector3(0.0f, 1.0f, 0.0f), Vector2(0.0f, 1.0f)},

        // 底面 (Y-)
        {Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 0.0f)},
        {Vector3(0.5f, -0.5f, -0.5f), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 0.0f)},
        {Vector3(0.5f, -0.5f, 0.5f), Vector3(0.0f, -1.0f, 0.0f), Vector2(1.0f, 1.0f)},
        {Vector3(-0.5f, -0.5f, 0.5f), Vector3(0.0f, -1.0f, 0.0f), Vector2(0.0f, 1.0f)}};

    std::vector<unsigned int> indices = {
        // 前面
        0, 1, 2, 0, 2, 3,
        // 后面
        4, 5, 6, 4, 6, 7,
        // 右面
        8, 9, 10, 8, 10, 11,
        // 左面
        12, 13, 14, 12, 14, 15,
        // 顶面
        16, 17, 18, 16, 18, 19,
        // 底面
        20, 21, 22, 20, 22, 23};

    // 创建网格
    auto mesh = std::make_shared<CMesh>(vertices, indices, m_DefaultTexture);

    // 创建模型
    auto model = std::make_shared<CModel>();
    // 这里需要你的CModel有AddMesh方法
    model->AddMesh(mesh);

    // 设置模型名称
    model->SetName(L"DefaultCube");

    return model;
}
