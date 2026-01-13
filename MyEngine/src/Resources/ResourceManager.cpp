
// ======================================================================
#include "stdafx.h"
#include "Resources/ResourceManager.h"
#include "Resources/Texture.h"
#include "Utils/StringUtils.h"
// ======================================================================


int CResourceManager::Initialize(const ResourceConfig& config)
{
    // Store configuration
    m_Config = config;
    
    // Initialize texture cache
    // m_Textures.clear();
    
    // Initialize other resource caches
    // m_Models.clear();
    // m_Shaders.clear();
    // m_Sounds.clear();
    
    // Create default resources if needed
    // CreateDefaultResources();
    
    return 0; // Return 0 for success, non-zero for error
}

std::shared_ptr<CTexture> CResourceManager::GetTexture(const std::string &fileName)
{
    // 检查缓存
    auto it = m_Textures.find(fileName);
    if (it != m_Textures.end())
        return it->second;

    // 路径处理
    std::wstring wPath = m_Config.GetTexturePath() + CStringUtils::StringToWString(fileName);

    // 加载
    auto texture = std::make_shared<CTexture>();
    if (texture->LoadFromFile(wPath))
    {
        m_Textures[fileName] = texture;
        return texture;
    }

    return nullptr; // 或者返回一个默认的错误纹理
}