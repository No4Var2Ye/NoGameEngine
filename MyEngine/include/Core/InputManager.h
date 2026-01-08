
// ======================================================================
#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

// ======================================================================
#include <Windows.h>
#include <array>
#include <vector>
// ======================================================================

// 鼠标按键枚举
enum class MouseButton
{
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Count = 5
};

// ======================================================================

/**
 * @brief 输入管理器类
 * @details 处理键盘和鼠标输入，提供输入状态查询
 */
class CInputManager
{
private:
    static const INT KEY_COUNT = 256;        // 键盘按键数量
    static const INT MOUSE_BUTTON_COUNT = 5; // 鼠标按键数量

    HWND m_hWnd;           // 窗口句柄
    HINSTANCE m_hInstance; // 应用程序实例

    // 键盘状态
    std::array<BYTE, KEY_COUNT> m_CurrentKeyState;  // 当前帧按键状态
    std::array<BYTE, KEY_COUNT> m_PreviousKeyState; // 上一帧按键状态
    std::array<BOOL, KEY_COUNT> m_KeyPressed;       // 按键刚按下
    std::array<BOOL, KEY_COUNT> m_KeyReleased;      // 按键刚释放

    // 鼠标状态
    POINT m_MousePosition;         // 鼠标当前位置
    POINT m_PreviousMousePosition; // 上一帧鼠标位置
    POINT m_MouseDelta;            // 鼠标移动增量

    INT m_MouseWheelDelta = 0;                                   // 鼠标滚轮增量
    std::array<BOOL, MOUSE_BUTTON_COUNT> m_CurrentMouseButtons;  // 当前帧鼠标按键状态
    std::array<BOOL, MOUSE_BUTTON_COUNT> m_PreviousMouseButtons; // 上一帧鼠标按键状态
    std::array<BOOL, MOUSE_BUTTON_COUNT> m_MouseButtonPressed;   // 鼠标按键刚按下
    std::array<BOOL, MOUSE_BUTTON_COUNT> m_MouseButtonReleased;  // 鼠标按键刚释放

    BOOL m_InputEnabled = TRUE; // 输入是否启用
    BOOL m_MouseLocked = FALSE; // 鼠标是否锁定
    POINT m_MouseLockPosition;  // 鼠标锁定位置
    BOOL m_ShowCursor = TRUE;   // 是否显示鼠标指针

    void UpdateKeyboard(); // 更新键盘状态
    void UpdateMouse();    // 更新鼠标状态

    /**
     * @brief 处理Windows消息
     * @param hWnd 窗口句柄
     * @param msg 消息
     * @param wParam 参数1
     * @param lParam 参数2
     * @return 是否处理了消息
     */
    LRESULT HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 获取虚拟键对应的数组索引
     * @param vk 虚拟键码
     * @return 数组索引
     */
    INT GetKeyIndex(UINT vk) const;

    /**
     * @brief 获取鼠标按键索引
     * @param button 鼠标按键
     * @return 数组索引
     */
    INT GetMouseButtonIndex(MouseButton button) const;

public:
    CInputManager();
    ~CInputManager();

    // 禁止拷贝构造和赋值
    CInputManager(const CInputManager &) = delete;
    CInputManager &operator=(const CInputManager &) = delete;

    /**
     * @brief 初始化输入管理器
     * @param hWnd 窗口句柄
     * @param hInstance 应用程序实例
     * @return 初始化成功返回TRUE，失败返回FALSE
     */
    BOOL Initialize(HWND hWnd, HINSTANCE hInstance);

    void Shutdown(); // 关闭输入管理器

    void Update();     // 更新输入状态（每帧调用）
    void ClearDelta(); // 清零位移

    /**
     * @brief 处理窗口消息
     * @param hWnd 窗口句柄
     * @param msg 消息
     * @param wParam 参数1
     * @param lParam 参数2
     * @return 是否处理了消息
     */
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // ==================================================================
    // 键盘输入查询
    // ==================================================================

    /**
     * @brief 检查按键是否按下
     * @param vk 虚拟键码
     * @return 按下返回TRUE，否则返回FALSE
     */
    BOOL IsKeyDown(UINT vk) const;

    /**
     * @brief 检查按键是否刚按下
     * @param vk 虚拟键码
     * @return 刚按下返回TRUE，否则返回FALSE
     */
    BOOL IsKeyPressed(UINT vk) const;

    /**
     * @brief 检查按键是否刚释放
     * @param vk 虚拟键码
     * @return 刚释放返回TRUE，否则返回FALSE
     */
    BOOL IsKeyReleased(UINT vk) const;

    /**
     * @brief 检查按键是否弹起
     * @param vk 虚拟键码
     * @return 弹起返回TRUE，否则返回FALSE
     */
    BOOL IsKeyUp(UINT vk) const;

    /**
     * @brief 获取所有按下的按键
     * @param keys 输出按下的按键数组
     * @return 按下的按键数量
     */
    INT GetPressedKeys(std::vector<UINT> &keys) const;

    // ==================================================================
    // 鼠标输入查询
    // ==================================================================

    /**
     * @brief 检查鼠标按键是否按下
     * @param button 鼠标按键
     * @return 按下返回TRUE，否则返回FALSE
     */
    BOOL IsMouseButtonDown(MouseButton button) const;

    /**
     * @brief 检查鼠标按键是否刚按下
     * @param button 鼠标按键
     * @return 刚按下返回TRUE，否则返回FALSE
     */
    BOOL IsMouseButtonPressed(MouseButton button) const;

    /**
     * @brief 检查鼠标按键是否刚释放
     * @param button 鼠标按键
     * @return 刚释放返回TRUE，否则返回FALSE
     */
    BOOL IsMouseButtonReleased(MouseButton button) const;

    /**
     * @brief 检查鼠标按键是否弹起
     * @param button 鼠标按键
     * @return 弹起返回TRUE，否则返回FALSE
     */
    BOOL IsMouseButtonUp(MouseButton button) const;

    POINT GetMousePosition() const { return m_MousePosition; } // 获取鼠标位置
    INT GetMouseX() const { return m_MousePosition.x; }        // 获取鼠标X坐标
    INT GetMouseY() const { return m_MousePosition.y; }        // 获取鼠标Y坐标

    POINT GetMouseDelta() const { return m_MouseDelta; }                       // 获取鼠标移动增量
    INT GetMouseDeltaX() const { return m_MouseDelta.x; }                      // 获取鼠标X方向移动增量
    INT GetMouseDeltaY() const { return m_MouseDelta.y; }                      // 获取鼠标Y方向移动增量
    INT GetMouseWheelDelta() const { return m_MouseWheelDelta; }               // 获取鼠标滚轮增量
    int GetMouseWheelSteps() const { return m_MouseWheelDelta / WHEEL_DELTA; } // 获取标准化的滚轮滚动量

    // ==================================================================
    // 鼠标控制
    // ==================================================================

    /**
     * @brief 设置鼠标位置
     * @param x X坐标
     * @param y Y坐标
     */
    void SetMousePosition(INT x, INT y);

    void ShowCursor();   // 显示鼠标指针
    void HideCursor();   // 隐藏鼠标指针
    void ToggleCursor(); // 切换鼠标指针显示状态
    void LockMouse();    // 锁定鼠标到窗口中心
    void UnlockMouse();  // 解锁鼠标

    void ToggleMouseLock(); // 切换鼠标锁定状态

    // ==================================================================
    // 输入启用/禁用
    void EnableInput() { m_InputEnabled = TRUE; }          // 启用输入
    void DisableInput() { m_InputEnabled = FALSE; }        // 禁用输入
    BOOL IsInputEnabled() const { return m_InputEnabled; } // 检查输入是否启用

    void Clear(); // 清除所有输入状态

    BOOL IsAnyKeyPressed() const;         // 检查是否按下了任意键
    BOOL IsAnyMouseButtonPressed() const; // 检查是否按下了任意鼠标按键

    // ==================================================================
    // 辅助功能

    /**
     * @brief 检查是否按下了组合键
     * @param vk1 键1
     * @param vk2 键2
     * @return 同时按下返回TRUE，否则返回FALSE
     */
    BOOL IsKeyComboDown(UINT vk1, UINT vk2) const;

    /**
     * @brief 获取键盘状态快照
     * @param state 输出键盘状态数组
     */
    void GetKeyboardState(std::array<BYTE, KEY_COUNT> &state) const;
};

#endif // __INPUTMANAGER_H__