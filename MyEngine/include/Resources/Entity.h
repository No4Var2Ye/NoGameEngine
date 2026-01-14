
// ======================================================================
#ifndef __ENTITY_H__
#define __ENTITY_H__
// ======================================================================
#include <windows.h>
#include <string>
#include <memory>
#include "EngineConfig.h"
#include "GL/gl.h"
#include "Math/Vector3.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
// ======================================================================
class CModel;
// ======================================================================
class CEntity
{
public:
    CEntity(std::shared_ptr<CModel> pModel);
    virtual ~CEntity() = default;

    // 禁用拷贝构造和赋值
    CEntity(const CEntity &) = delete;
    CEntity &operator=(const CEntity &) = delete;

    // CEntity(CEntity&& other) noexcept = default;
    // CEntity& operator=(CEntity&& other) noexcept = default;
    //CEntity(CEntity &&other) = default;
    //CEntity &operator=(CEntity &&other) = default;

    void Update(float deltaTime);
    void Render();

    // 模型相关
    void SetModel(std::shared_ptr<CModel> pModel) { m_pModel = pModel; }
    std::shared_ptr<CModel> GetModel() const { return m_pModel; }
    BOOL HasModel() const { return m_pModel != nullptr; }

    // 实体名称（用于调试和查找）
    void SetName(const std::string &name) { m_name = name; }
    const std::string &GetName() const { return m_name; }

    // 可见性控制
    void SetVisible(BOOL visible) { m_bVisible = visible; }
    BOOL IsVisible() const { return m_bVisible; }

//private:

protected:
    std::shared_ptr<CModel> m_pModel;
    std::string m_name;

    // 变换参数
    Vector3 m_position = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 m_rotation = Vector3(0.0f, 0.0f, 0.0f); // 欧拉角（度）
    Vector3 m_scale = Vector3(1.0f, 1.0f, 1.0f);

    // 缓存的变换矩阵
    Matrix4 m_transformMatrix = Matrix4::Identity();
    BOOL m_bTransformDirty = true; // 变换是否需要重新计算

    // 渲染状态
    BOOL m_bVisible = true;
};

#endif // __ENTITY_H__