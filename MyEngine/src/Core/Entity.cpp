
// ======================================================================
#include "stdafx.h"
#include <queue>
#include "Core/Entity.h"
#include "Resources/Model.h"
// ======================================================================

// 静态变量初始化
unsigned int CEntity::s_nextID = 0;

CEntity::CEntity()
    : m_uID(++s_nextID),                  //
      m_name(L"New Entity"),              //
      m_position(0.0f, 0.0f, 0.0f),       //
      m_rotation(Quaternion::Identity()), //
      m_scale(1.0f, 1.0f, 1.0f),          //
      m_bWorldDirty(TRUE),                //
      m_bVisible(TRUE)                    //
{
    m_cachedWorldMatrix = Matrix4::Identity();
}

void CEntity::SetParent(std::shared_ptr<CEntity> pParent)
{
    // 1. 检查是否已经是这个父节点
    auto pCurrentParent = m_pParent.lock();
    if (pCurrentParent == pParent)
        return;

    // 2. 如果存在旧父节点，先从旧父节点的子列表中移除自己
    if (pCurrentParent)
    {
        // 注意：这里需要一个内部移除逻辑，避免递归调用 SetParent
        pCurrentParent->InternalRemoveChild(m_uID);
    }

    // 3. 更新父节点引用
    if (pParent)
    {
        m_pParent = pParent;
        // 将自己加入新父节点的容器中
        pParent->InternalAddChild(shared_from_this());
    }
    else
    {
        m_pParent.reset();
    }

    MarkDirty();
}

void CEntity::AddChild(std::shared_ptr<CEntity> pChild)
{
    if (!pChild || pChild.get() == this)
        return;

    pChild->SetParent(shared_from_this());
}

BOOL CEntity::RemoveChild(unsigned int id)
{
    auto itMap = m_childrenMap.find(id);
    if (itMap != m_childrenMap.end())
    {
        std::shared_ptr<CEntity> pChild = itMap->second;

        // 1. 从 Map 移除
        m_childrenMap.erase(itMap);

        // 2. 从 Vector 移除
        m_children.erase(std::remove(m_children.begin(), m_children.end(), pChild), m_children.end());

        // 3. 重置子节点的父指针
        pChild->m_pParent.reset();
        pChild->MarkDirty();

        return TRUE;
    }
    return FALSE;
}

void CEntity::InternalAddChild(std::shared_ptr<CEntity> pChild)
{
    if (pChild && m_childrenMap.find(pChild->GetID()) == m_childrenMap.end())
    {
        m_children.push_back(pChild);
        m_childrenMap[pChild->GetID()] = pChild;
    }
}

void CEntity::InternalRemoveChild(unsigned int uID)
{
    auto it = m_childrenMap.find(uID);
    if (it != m_childrenMap.end())
    {
        auto pChild = it->second;
        m_childrenMap.erase(it);
        m_children.erase(std::remove(m_children.begin(), m_children.end(), pChild), m_children.end());
    }
}

std::shared_ptr<CEntity> CEntity::GetChild(size_t index) const
{
    if (index < m_children.size())
        return m_children[index];
    return nullptr;
}
// 递归查找子实体
std::shared_ptr<CEntity> CEntity::FindChildByName(const std::wstring &name)
{
    // 1. 检查自己是不是
    if (m_name == name)
    {
        return shared_from_this();
    }

    // 2. 递归查找子节点
    for (auto &child : m_children)
    {
        auto found = child->FindChildByName(name);
        if (found)
            return found;
    }

    return nullptr;
}

void CEntity::Update(float deltaTime)
{
    if (!m_bVisible)
        return;

    // 递归更新所有子节点
    for (auto &pChild : m_children)
    {
        if (pChild)
        {
            pChild->Update(deltaTime);
        }
    }
}

void CEntity::SetPosition(const Vector3 &pos)
{
    m_position = pos;
    MarkDirty();
}

void CEntity::SetRotation(const Vector3 &euler)
{
    // 将欧拉角转为四元数存储
    m_rotation = Quaternion::FromEuler(euler.x, euler.y, euler.z);
    MarkDirty();
}

void CEntity::SetScale(const Vector3 &scale)
{
    m_scale = scale;
    MarkDirty();
}

Vector3 CEntity::GetWorldPosition() const
{
    return GetWorldMatrix().GetTranslation();
}

Matrix4 CEntity::GetWorldMatrix() const
{
    if (m_bWorldDirty)
    {
        Matrix4 localMatrix = Matrix4::TRS(m_position, m_rotation, m_scale);

        // 使用 lock() 安全获取父节点
        if (auto pParent = m_pParent.lock())
        {
            m_cachedWorldMatrix = pParent->GetWorldMatrix() * localMatrix;
        }
        else
        {
            m_cachedWorldMatrix = localMatrix;
        }

        m_bWorldDirty = FALSE;
    }
    return m_cachedWorldMatrix;
}

void CEntity::ApplyTransform() const
{
    // 获取最新的世界矩阵并应用到 OpenGL 堆栈
    Matrix4 mat = GetWorldMatrix();
    glMultMatrixf(mat.GetData());
}

void CEntity::MarkDirty()
{
    if (m_bWorldDirty)
        return; // 已经脏了，避免重复传播

    m_bWorldDirty = TRUE;

    // 使用 BFS（广度优先）传播脏标记，避免深层级递归栈溢出
    std::queue<CEntity *> queue;
    for (auto &child : m_children)
        if (child)
            queue.push(child.get());

    while (!queue.empty())
    {
        CEntity *current = queue.front();
        queue.pop();

        if (!current->m_bWorldDirty)
        {
            current->m_bWorldDirty = TRUE;
            for (auto &child : current->m_children)
                if (child)
                    queue.push(child.get());
        }
    }
}
