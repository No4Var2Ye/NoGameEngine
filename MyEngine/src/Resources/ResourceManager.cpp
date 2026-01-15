
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
    // 创建默认纹理
    // 这种格子图即使不看代码也能立刻发现纹理丢失
    const int size = 64;
    unsigned char data[size * size * 3];
    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            int idx = (y * size + x) * 3;
            bool isPink = ((x / 8) + (y / 8)) % 2 == 0;
            if (isPink)
            {
                data[idx] = 255;
                data[idx + 1] = 0;
                data[idx + 2] = 255; // Magenta
            }
            else
            {
                data[idx] = 0;
                data[idx + 1] = 0;
                data[idx + 2] = 0; // Black
            }
        }
    }

    // 注意：默认纹理存入一个特殊的 shared_ptr 成员变量
    // 这样就永远不会被释放，直到管理器关闭
    m_DefaultTexture = std::make_shared<CTexture>();
    if (!m_DefaultTexture->LoadFromMemory(data, size, size, 3, GL_RGB))
    {
        return FALSE;
    }

    // TODO: 创建默认 Shader
    // 可以创建一个简单的只渲染纯色的 Shader 供出错时使用

    return TRUE;
}

std::shared_ptr<CModel> CResourceManager::CreateDefaultModel()
{
    // 创建一个简单的立方体模型
    static std::weak_ptr<CModel> s_DefaultModel;

    if (auto model = s_DefaultModel.lock())
        return model;

    auto model = std::make_shared<CModel>();
    // 创建立方体数据...
    s_DefaultModel = model;
    return model;
}

std::shared_ptr<CTexture> CResourceManager::GetTexture(const std::wstring &fileName)
{
    // 1. 缓存查找逻辑 (lock weak_ptr)...
    auto it = m_Textures.find(fileName);
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

    std::wstring fullPath = m_Config.rootPath + L"/Textures/" + fileName;
    if (newTex->LoadFromFile(fullPath))
    {
        m_Textures[fileName] = newTex;
        return newTex;
    }

    // 3. 文件加载失败
    LogWarning(L"加载纹理失败: %ls. 使用默认样式。", fileName.c_str());

    return m_DefaultTexture;
}

std::shared_ptr<CModel> CResourceManager::GetModel(const std::wstring &fileName)
{
    // 1. 缓存查找
    auto it = m_Models.find(fileName);
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

    // 拼接完整路径：rootPath/Models/fileName
    std::wstring fullPath = m_Config.rootPath + L"/Models/" + fileName;

    // 传入 this，允许 CModel 在加载过程中调用 GetTexture
    if (newModel->LoadFromFile(fullPath, this))
    {
        m_Models[fileName] = newModel;
        return newModel;
    }

    // 3. 如果模型加载失败
    // 注意：模型通常没有“兜底模型”，建议返回 nullptr 或一个简单的立方体模型
    LogError((L"[Error]: 无法加载模型文件: " + fullPath + L"\n").c_str());

    if (!newModel->LoadFromFile(fullPath, this))
    {
        LogError((L"无法加载模型文件: " + fullPath + L"\n").c_str());

        // 返回一个简单的立方体模型作为默认
        return CreateDefaultModel();
    }

    return newModel;
}