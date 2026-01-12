/**
 * @file stdafx.h
 */

// ======================================================================
#ifndef __STDAFX_H__
#define __STDAFX_H__

#define NOMINMAX // 禁用 min 和 max 宏

#define MYDEDUG
// ======================================================================

// 常用的头文件
#include <windows.h> // Header File For Windows
#include <iostream>
#include <stdio.h> // Header File For Standard Input/Output
#include <cstdlib>
#include <string>
#include <cmath>
#include <ctime>
#include <math.h>
#include <time.h>
#include <vector>

// GL前缀的头文件
#include "GL/gl.h"  // Header File For The OpenGL32 Library
#include "GL/glu.h" // Header File For The GLu32 Library
#include "GL/glaux.h"
#include "GL/glext.h"

// 数学库
#include "Math/MathUtils.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"

// 手动包含OpenGL链接库文件
// #pragma comment(lib, "opengl32.lib")
// #pragma comment(lib, "glu32.lib")
// #pragma comment(lib, "glaux.lib")
// #pragma comment(lib, "glext.lib")

#endif // __STDAFX_H__