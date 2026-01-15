
// ======================================================================
#ifndef __STDAFX_H__
#define __STDAFX_H__
// ======================================================================
// 系统宏定义
#define NOMINMAX // 禁用 min 和 max 宏
// ======================================================================

// ======================================================================
// Windows 头文件
#include <windows.h>

// ======================================================================
// C++标准库 头文件
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

// ======================================================================
// 项目宏定义
#define MYDEBUG

// ======================================================================
// 数学库
#include "Math/MathUtils.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"

// ======================================================================
// OpenGL 头文件
#ifdef _WIN32
#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glaux.h"
#include "GL/glext.h"
#else
// Linux/macOS 的包含方式
#include <GL/gl.h>
#include <GL/glu.h>
#endif

// ======================================================================
// 工具类
#include "Utils/LogUtils.h"
#include "Utils/PathUtils.h"

// ======================================================================
// 库链接指令
// #pragma comment(lib, "opengl32.lib")
// #pragma comment(lib, "glu32.lib")
// #pragma comment(lib, "glaux.lib")
// #pragma comment(lib, "glext.lib")

#endif // __STDAFX_H__