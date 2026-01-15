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

class CTexture;
class CModel;
class CShader;
// ======================================================================

class CResourceManager
{
public:
    CResourceManager() = default;
    ~CResourceManager() = default;

    // 禁用拷贝
    CResourceManager(const CResourceManager &) = delete;
    CResourceManager &operator=(const CResourceManager &) = delete;

    BOOL Initialize(const ResourceConfig &config);
    void Shutdown();

    // ======================================================================
    // 资源加载接口
    std::shared_ptr<CTexture> GetTexture(const std::wstring &fileName);
    std::shared_ptr<CModel> GetModel(const std::wstring &fileName);
    // std::shared_ptr<CShader> GetShader(const std::wstring &name, const std::wstring &vPath, const std::wstring &fPath);

    // 创建 兜底资源
    BOOL CreateDefaultResources();
    std::shared_ptr<CTexture> CreateDefaultTexture();
    std::shared_ptr<CModel> CreateDefaultModel();
    std::shared_ptr<CShader> CreateDefaultShader();
    // ======================================================================
    // 清理资源
    void ReleaseUnusedResources();

private:
    ResourceConfig m_Config;

    // ======================================================================
    // 资源容器
    std::unordered_map<std::wstring, std::weak_ptr<CTexture>> m_Textures;
    std::unordered_map<std::wstring, std::weak_ptr<CModel>> m_Models;
    std::unordered_map<std::wstring, std::weak_ptr<CShader>> m_Shaders;

    // 兜底资源：当加载失败时返回，防止引擎崩溃
    std::shared_ptr<CTexture> m_DefaultTexture;
    std::shared_ptr<CModel> m_DefaultModel;
    std::shared_ptr<CShader> m_DefaultShader;
};

#endif // __RESOURCE_MANAGER_H__