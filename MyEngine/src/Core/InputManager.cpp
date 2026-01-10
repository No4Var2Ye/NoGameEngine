
// ======================================================================
#include "stdafx.h"

#include <windowsx.h>
#include <cassert>
#include "Core/InputManager.h"
// ======================================================================

CInputManager::CInputManager()
    : m_hWnd(nullptr),      // 窗口句柄
      m_hInstance(nullptr), // 应用程序实例
      m_MouseWheelDelta(0), // 鼠标移动增量
      m_InputEnabled(TRUE), // 输入是否启用
      m_MouseLocked(FALSE), // 鼠标是否锁定
      m_ShowCursor(TRUE)    // 是否显示鼠标指针
{
    // 初始化鼠标位置
    m_MousePosition = {0, 0};
    m_PreviousMousePosition = {0, 0};
    m_MouseDelta = {0, 0};
    m_MouseLockPosition = {0, 0};

    // 初始化键盘状态数组
    m_CurrentKeyState.fill(0);
    m_PreviousKeyState.fill(0);
    m_KeyPressed.fill(FALSE);
    m_KeyReleased.fill(FALSE);

    // 初始化鼠标状态数组
    m_CurrentMouseButtons.fill(FALSE);
    m_PreviousMouseButtons.fill(FALSE);
    m_MouseButtonPressed.fill(FALSE);
    m_MouseButtonReleased.fill(FALSE);
}

CInputManager::~CInputManager()
{
    Shutdown();
}

BOOL CInputManager::Initialize(HWND hWnd, HINSTANCE hInstance)
{
    assert(hWnd != nullptr);
    assert(hInstance != nullptr);

    m_hWnd = hWnd;
    m_hInstance = hInstance;

    // 获取初始鼠标位置
    POINT pt;
    if (GetCursorPos(&pt))
    {
        ScreenToClient(m_hWnd, &pt);
        m_MousePosition = pt;
        m_PreviousMousePosition = pt;
    }

    // 设置原始输入
    RAWINPUTDEVICE rid[2];

    // 键盘
    rid[0].usUsagePage = 0x01;
    rid[0].usUsage = 0x06;
    rid[0].dwFlags = 0;
    rid[0].hwndTarget = hWnd;

    // 鼠标
    rid[1].usUsagePage = 0x01;
    rid[1].usUsage = 0x02;
    rid[1].dwFlags = 0;
    rid[1].hwndTarget = hWnd;

    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // 验证是否设置成功
    if (GetWindowLongPtr(m_hWnd, GWLP_USERDATA) == 0)
    {
        OutputDebugStringA("Warning: SetWindowLongPtr 失败！\n");
    }

    if (!RegisterRawInputDevices(rid, 2, sizeof(rid[0])))
    {
        OutputDebugStringA("Warning: 注册原始输入设备失败\n");
        return FALSE;
    }

    return TRUE;
}

void CInputManager::Shutdown()
{
    // 解锁鼠标
    if (m_MouseLocked)
    {
        this->UnlockMouse();
    }

    // 显示鼠标
    if (!m_ShowCursor)
    {
        this->ShowCursor();
    }

    m_hWnd = nullptr;
    m_hInstance = nullptr;
}

void CInputManager::Update()
{
    if (!m_InputEnabled)
        return;

    // 保存上一帧状态
    m_PreviousKeyState = m_CurrentKeyState;
    m_PreviousMouseButtons = m_CurrentMouseButtons;

    // 重置按键状态
    m_KeyPressed.fill(FALSE);
    m_KeyReleased.fill(FALSE);
    m_MouseButtonPressed.fill(FALSE);
    m_MouseButtonReleased.fill(FALSE);

    // 更新键盘状态
    UpdateKeyboard();

    // 更新鼠标状态
    for (int i = 0; i < MOUSE_BUTTON_COUNT; ++i)
    {
        m_MouseButtonPressed[i] = m_CurrentMouseButtons[i] && !m_PreviousMouseButtons[i];
        m_MouseButtonReleased[i] = !m_CurrentMouseButtons[i] && m_PreviousMouseButtons[i];
    }
}

// 在 Update 之后，或者在帧渲染开始前调用此方法来“清零”位移
void CInputManager::ClearDelta()
{
    m_MouseDelta = {0, 0};
    m_MouseWheelDelta = 0;
}

LRESULT CALLBACK CInputManager::InputWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // 获取输入管理器实例
    CInputManager *pInputManager = nullptr;

    // 从窗口额外数据获取输入管理器实例
    pInputManager = reinterpret_cast<CInputManager *>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (pInputManager)
    {
        LRESULT result = pInputManager->HandleMessage(hWnd, msg, wParam, lParam);
        if (result != 0)
        {
            return result;
        }
    }
    else
    {
        OutputDebugStringA("Warning: InputManager instance not found in UserData\n");
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CInputManager::UpdateKeyboard()
{
    // 1. 保存上一帧状态
    m_PreviousKeyState = m_CurrentKeyState;

    // 2. 获取当前键盘状态
    HRESULT hr = ::GetKeyboardState(m_CurrentKeyState.data());

    if (FAILED(hr))
    {
        std::cerr << "获取键盘状态失败！错误码: 0x" << std::hex << hr << std::dec << std::endl;
        return;
    }

    // 计算按键按下和释放状态
    for (INT i = 0; i < KEY_COUNT; ++i)
    {
        BYTE currentState = m_CurrentKeyState[i];
        BYTE previousState = m_PreviousKeyState[i];

        m_KeyPressed[i] = ((currentState & 0x80) && !(previousState & 0x80));
        m_KeyReleased[i] = (!(currentState & 0x80) && (previousState & 0x80));
    }
}

void CInputManager::UpdateMouse()
{
    // 获取当前鼠标位置
    POINT pt;
    if (GetCursorPos(&pt))
    {
        ScreenToClient(m_hWnd, &pt);

        m_PreviousMousePosition = m_MousePosition;
        m_MousePosition = pt;

        // 计算鼠标增量
        m_MouseDelta.x = m_MousePosition.x - m_PreviousMousePosition.x;
        m_MouseDelta.y = m_MousePosition.y - m_PreviousMousePosition.y;
    }

    // 计算鼠标按键按下和释放状态
    for (INT i = 0; i < MOUSE_BUTTON_COUNT; ++i)
    {
        m_MouseButtonPressed[i] = m_CurrentMouseButtons[i] && !m_PreviousMouseButtons[i];
        m_MouseButtonReleased[i] = !m_CurrentMouseButtons[i] && m_PreviousMouseButtons[i];
    }
}

LRESULT CInputManager::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!m_InputEnabled)
        return 0;

    switch (msg)
    {
    case WM_KILLFOCUS:
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            Clear();
            if (m_MouseLocked)
                UnlockMouse();
        }
        return 0;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        char buf[128];
        sprintf_s(buf, "Input: Key Down [VK: %u]\n", (UINT)wParam);
        OutputDebugStringA(buf);

        UINT vk = static_cast<UINT>(wParam);
        INT index = GetKeyIndex(vk);
        if (index >= 0 && index < KEY_COUNT)
        {
            m_CurrentKeyState[index] = 0x80; // 设置按下状态
        }
    }
    break;

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        UINT vk = static_cast<UINT>(wParam);
        INT index = GetKeyIndex(vk);
        if (index >= 0 && index < KEY_COUNT)
        {
            m_CurrentKeyState[index] = 0; // 清除按下状态
        }
    }
    break;

    // 鼠标按键
    case WM_LBUTTONDOWN:
        OutputDebugStringA("Input: Left Mouse Button Down\n");
        m_CurrentMouseButtons[(INT)MouseButton::Left] = TRUE;
        return 0;
    case WM_LBUTTONUP:
        m_CurrentMouseButtons[(INT)MouseButton::Left] = FALSE;
        return 0;
    case WM_RBUTTONDOWN:
        m_CurrentMouseButtons[(INT)MouseButton::Right] = TRUE;
        OutputDebugStringA("Input: Right Mouse Button Down\n");
        return 0;
    case WM_RBUTTONUP:
        m_CurrentMouseButtons[(INT)MouseButton::Right] = FALSE;
        return 0;
    case WM_MBUTTONDOWN:
        m_CurrentMouseButtons[(INT)MouseButton::Middle] = TRUE;
        OutputDebugStringA("Input: Middle Mouse Button Down\n");
        return 0;
    case WM_MBUTTONUP:
        m_CurrentMouseButtons[(INT)MouseButton::Middle] = FALSE;
        return 0;

    case WM_XBUTTONDOWN:
    {
        UINT button = GET_XBUTTON_WPARAM(wParam);
        if (button == XBUTTON1)
        {
            m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Button4)] = TRUE;
        }
        else if (button == XBUTTON2)
        {
            m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Button5)] = TRUE;
        }
    }
    break;

    case WM_XBUTTONUP:
    {
        UINT button = GET_XBUTTON_WPARAM(wParam);
        if (button == XBUTTON1)
        {
            m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Button4)] = FALSE;
        }
        else if (button == XBUTTON2)
        {
            m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Button5)] = FALSE;
        }
    }
    break;

    case WM_MOUSEMOVE:
        // 鼠标位置更新
        m_PreviousMousePosition = m_MousePosition;
        m_MousePosition.x = GET_X_LPARAM(lParam);
        m_MousePosition.y = GET_Y_LPARAM(lParam);
        break;

    case WM_INPUT:
        // INFO: 用于相机旋转
        {
            UINT dataSize = 0;
            GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &dataSize, sizeof(RAWINPUTHEADER));

            if (dataSize > 0)
            {
                UINT dwSize = sizeof(RAWINPUT);
                static BYTE lpb[sizeof(RAWINPUT)];
                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
                RAWINPUT *raw = (RAWINPUT *)lpb;

                if (raw->header.dwType == RIM_TYPEMOUSE)
                {
                    // 处理原始鼠标输入
                    if (raw->data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
                    {
                        m_MouseDelta.x += raw->data.mouse.lLastX;
                        m_MouseDelta.y += raw->data.mouse.lLastY;
                    }
                }
            }
        }
        break;

    case WM_MOUSEWHEEL:
        m_MouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
        break;
    }

    return 0;
}

BOOL CInputManager::IsKeyDown(UINT vk) const
{
    INT index = GetKeyIndex(vk);
    if (index >= 0 && index < KEY_COUNT)
    {
        // std::cout << "Key " << vk << " index " << index << " state: " << (int)m_CurrentKeyState[index] << std::endl;
        return (m_CurrentKeyState[index] & 0x80) != 0;
    }
    return FALSE;
}

BOOL CInputManager::IsKeyPressed(UINT vk) const
{
    INT index = GetKeyIndex(vk);
    if (index >= 0 && index < KEY_COUNT)
    {
        return m_KeyPressed[index];
    }
    return FALSE;
}

BOOL CInputManager::IsKeyReleased(UINT vk) const
{
    INT index = GetKeyIndex(vk);
    if (index >= 0 && index < KEY_COUNT)
    {
        return m_KeyReleased[index];
    }
    return FALSE;
}

BOOL CInputManager::IsKeyUp(UINT vk) const
{
    return !IsKeyDown(vk);
}

INT CInputManager::GetKeyIndex(UINT vk) const
{
    if (vk < KEY_COUNT)
    {
        return static_cast<INT>(vk);
    }
    return -1;
}

INT CInputManager::GetMouseButtonIndex(MouseButton button) const
{
    return static_cast<INT>(button);
}

BOOL CInputManager::IsMouseButtonDown(MouseButton button) const
{
    INT index = GetMouseButtonIndex(button);
    if (index >= 0 && index < MOUSE_BUTTON_COUNT)
    {
        return m_CurrentMouseButtons[index];
    }
    return FALSE;
}

BOOL CInputManager::IsMouseButtonPressed(MouseButton button) const
{
    INT index = GetMouseButtonIndex(button);
    if (index >= 0 && index < MOUSE_BUTTON_COUNT)
    {
        return m_MouseButtonPressed[index];
    }
    return FALSE;
}

BOOL CInputManager::IsMouseButtonReleased(MouseButton button) const
{
    INT index = GetMouseButtonIndex(button);
    if (index >= 0 && index < MOUSE_BUTTON_COUNT)
    {
        return m_MouseButtonReleased[index];
    }
    return FALSE;
}

BOOL CInputManager::IsMouseButtonUp(MouseButton button) const
{
    return !IsMouseButtonDown(button);
}

void CInputManager::SetMousePosition(INT x, INT y)
{
    POINT pt = {x, y};
    ClientToScreen(m_hWnd, &pt);
    SetCursorPos(pt.x, pt.y);

    // 更新内部状态
    m_MousePosition = {x, y};
    m_PreviousMousePosition = {x, y};
    m_MouseDelta = {0, 0};
}

void CInputManager::ShowCursor()
{
    if (!m_ShowCursor)
    {
        int count = ::ShowCursor(TRUE);
        // 如果计数器仍然小于 0，强制补足
        while (count < 0)
        {
            count = ::ShowCursor(TRUE);
        }
        m_ShowCursor = TRUE;
    }
}

void CInputManager::HideCursor()
{
    if (m_ShowCursor)
    {
        int count = ::ShowCursor(FALSE);
        // 如果计数器仍然大于等于 0，强制减小
        while (count >= 0)
        {
            count = ::ShowCursor(FALSE);
        }
        m_ShowCursor = FALSE;
    }
}

void CInputManager::ToggleCursor()
{
    if (m_ShowCursor)
    {
        HideCursor();
    }
    else
    {
        ShowCursor();
    }
}

void CInputManager::LockMouse()
{
    if (m_MouseLocked)
        return;

    RECT rect;
    GetClientRect(m_hWnd, &rect);
    POINT center = {(rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2};

    // 锁定前记录当前位置，解锁时可以还原（可选）
    SetMousePosition(center.x, center.y);
    m_MouseLocked = TRUE;
    HideCursor();

    // 限制鼠标在窗口内移动
    ClientToScreen(m_hWnd, (LPPOINT)&rect.left);
    ClientToScreen(m_hWnd, (LPPOINT)&rect.right);
    ClipCursor(&rect);
}

void CInputManager::UnlockMouse()
{
    if (!m_MouseLocked)
        return;
    m_MouseLocked = FALSE;
    ClipCursor(NULL); // 释放鼠标限制
    ShowCursor();
}

void CInputManager::ToggleMouseLock()
{
    if (m_MouseLocked)
    {
        UnlockMouse();
    }
    else
    {
        LockMouse();
    }
}

INT CInputManager::GetPressedKeys(std::vector<UINT> &keys) const
{
    keys.clear();

    for (INT i = 0; i < KEY_COUNT; ++i)
    {
        if (m_KeyPressed[i])
        {
            keys.push_back(static_cast<UINT>(i));
        }
    }

    return static_cast<INT>(keys.size());
}

void CInputManager::Clear()
{
    m_CurrentKeyState.fill(0);
    m_PreviousKeyState.fill(0);
    m_KeyPressed.fill(FALSE);
    m_KeyReleased.fill(FALSE);

    m_CurrentMouseButtons.fill(FALSE);
    m_PreviousMouseButtons.fill(FALSE);
    m_MouseButtonPressed.fill(FALSE);
    m_MouseButtonReleased.fill(FALSE);

    m_MouseDelta = {0, 0};
    m_MouseWheelDelta = 0;
    m_MouseWheelAccumulated = 0;
}

BOOL CInputManager::IsAnyKeyPressed() const
{
    for (BOOL pressed : m_KeyPressed)
    {
        if (pressed)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CInputManager::IsAnyMouseButtonPressed() const
{
    for (BOOL pressed : m_MouseButtonPressed)
    {
        if (pressed)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CInputManager::IsKeyComboDown(UINT vk1, UINT vk2) const
{
    return IsKeyDown(vk1) && IsKeyDown(vk2);
}

void CInputManager::GetKeyboardState(std::array<BYTE, KEY_COUNT> &state) const
{
    state = m_CurrentKeyState;
}