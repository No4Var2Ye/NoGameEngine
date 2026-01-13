// ======================================================================
#ifndef __RESOURCE_MANAGER_H__
#define __RESOURCE_MANAGER_H__
// ======================================================================

#include <windows.h>
#include <string>
#include <unordered_map>
#include <memory>
#include "EngineConfig.h"
// ======================================================================

class CShader;
class CTexture;
class CModel;
// ======================================================================

class CResourceManager
{
public:
    CResourceManager() = default;
    ~CResourceManager() = default;
    CResourceManager(const CResourceManager &) = delete;
    CResourceManager &operator=(const CResourceManager &) = delete;
    // static CResourceManager &GetInstance()
    // {
    //     static CResourceManager instance;
    //     return instance;
    // }

    BOOL Initialize(const ResourceConfig &config);
    void Shutdown();

    // ======================================================================
    // 资源加载接口
    std::shared_ptr<CTexture> GetTexture(const std::string &fileName);
    std::shared_ptr<CModel> GetModel(const std::string &fileName);
    // std::shared_ptr<CShader> GetShader(const std::string &name, const std::string &vPath, const std::string &fPath);

    // ======================================================================
    // 清理资源
    void ReleaseUnusedResources();

private:


    ResourceConfig m_Config;

    // ======================================================================
    // 资源容器
    std::unordered_map<std::string, std::shared_ptr<CTexture>> m_Textures;
    std::unordered_map<std::string, std::shared_ptr<CModel>> m_Models;
    std::unordered_map<std::string, std::shared_ptr<CShader>> m_Shaders;
};

#endif // __RESOURCE_MANAGER_H__