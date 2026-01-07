
#include "stdafx.h"
#include "Core/InputManager.h"
#include <windowsx.h>
#include <cassert>

CInputManager::CInputManager()
    : m_hWnd(nullptr), m_hInstance(nullptr), m_MouseWheelDelta(0), m_InputEnabled(TRUE), m_MouseLocked(FALSE), m_ShowCursor(TRUE)
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

    if (!RegisterRawInputDevices(rid, 2, sizeof(rid[0])))
    {
        OutputDebugStringA("注册原始输入设备失败\n");
        return FALSE;
    }

    return TRUE;
}

void CInputManager::Shutdown()
{
    // 解锁鼠标
    if (m_MouseLocked)
    {
        UnlockMouse();
    }

    // 显示鼠标
    if (!m_ShowCursor)
    {
        ShowCursor();
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

    // 重置鼠标增量
    m_MouseDelta = {0, 0};
    m_MouseWheelDelta = 0;

    // 更新键盘状态
    UpdateKeyboard();

    // 更新鼠标状态
    UpdateMouse();
}

LRESULT CALLBACK CInputManager::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
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

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void CInputManager::UpdateKeyboard()
{

    // 获取当前键盘状态
    // 使用 :: 表示全局命名空间，调用 Windows API
    if (!::GetKeyboardState(m_CurrentKeyState.data()))
    {
        return;
    }

    // 计算按键按下和释放状态
    for (int i = 0; i < KEY_COUNT; ++i)
    {
        BOOL currentState = (m_CurrentKeyState[i] & 0x80) != 0;
        BOOL previousState = (m_PreviousKeyState[i] & 0x80) != 0;

        m_KeyPressed[i] = currentState && !previousState;
        m_KeyReleased[i] = !currentState && previousState;
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
    for (int i = 0; i < MOUSE_BUTTON_COUNT; ++i)
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
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        UINT vk = static_cast<UINT>(wParam);
        int index = GetKeyIndex(vk);
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
        int index = GetKeyIndex(vk);
        if (index >= 0 && index < KEY_COUNT)
        {
            m_CurrentKeyState[index] = 0; // 清除按下状态
        }
    }
    break;

    case WM_LBUTTONDOWN:
        m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Left)] = TRUE;
        break;

    case WM_LBUTTONUP:
        m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Left)] = FALSE;
        break;

    case WM_RBUTTONDOWN:
        m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Right)] = TRUE;
        break;

    case WM_RBUTTONUP:
        m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Right)] = FALSE;
        break;

    case WM_MBUTTONDOWN:
        m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Middle)] = TRUE;
        break;

    case WM_MBUTTONUP:
        m_CurrentMouseButtons[GetMouseButtonIndex(MouseButton::Middle)] = FALSE;
        break;

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

    case WM_MOUSEWHEEL:
        m_MouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
        break;

    case WM_MOUSEMOVE:
        // 鼠标移动已经在UpdateMouse中处理
        break;

    case WM_INPUT:
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
    break;
    }

    return 0;
}

BOOL CInputManager::IsKeyDown(UINT vk) const
{
    int index = GetKeyIndex(vk);
    if (index >= 0 && index < KEY_COUNT)
    {
        return (m_CurrentKeyState[index] & 0x80) != 0;
    }
    return FALSE;
}

BOOL CInputManager::IsKeyPressed(UINT vk) const
{
    int index = GetKeyIndex(vk);
    if (index >= 0 && index < KEY_COUNT)
    {
        return m_KeyPressed[index];
    }
    return FALSE;
}

BOOL CInputManager::IsKeyReleased(UINT vk) const
{
    int index = GetKeyIndex(vk);
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

int CInputManager::GetKeyIndex(UINT vk) const
{
    if (vk >= 0 && vk < KEY_COUNT)
    {
        return static_cast<int>(vk);
    }
    return -1;
}

int CInputManager::GetMouseButtonIndex(MouseButton button) const
{
    return static_cast<int>(button);
}

BOOL CInputManager::IsMouseButtonDown(MouseButton button) const
{
    int index = GetMouseButtonIndex(button);
    if (index >= 0 && index < MOUSE_BUTTON_COUNT)
    {
        return m_CurrentMouseButtons[index];
    }
    return FALSE;
}

BOOL CInputManager::IsMouseButtonPressed(MouseButton button) const
{
    int index = GetMouseButtonIndex(button);
    if (index >= 0 && index < MOUSE_BUTTON_COUNT)
    {
        return m_MouseButtonPressed[index];
    }
    return FALSE;
}

BOOL CInputManager::IsMouseButtonReleased(MouseButton button) const
{
    int index = GetMouseButtonIndex(button);
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

void CInputManager::SetMousePosition(int x, int y)
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
    ::ShowCursor(TRUE);
    m_ShowCursor = TRUE;
}

void CInputManager::HideCursor()
{
    ::ShowCursor(FALSE);
    m_ShowCursor = FALSE;
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

    int centerX = rect.left + (rect.right - rect.left) / 2;
    int centerY = rect.top + (rect.bottom - rect.top) / 2;

    SetMousePosition(centerX, centerY);
    m_MouseLockPosition = {centerX, centerY};
    m_MouseLocked = TRUE;

    HideCursor();
}

void CInputManager::UnlockMouse()
{
    if (!m_MouseLocked)
        return;

    m_MouseLocked = FALSE;
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

int CInputManager::GetPressedKeys(std::vector<UINT> &keys) const
{
    keys.clear();

    for (int i = 0; i < KEY_COUNT; ++i)
    {
        if (m_KeyPressed[i])
        {
            keys.push_back(static_cast<UINT>(i));
        }
    }

    return static_cast<int>(keys.size());
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