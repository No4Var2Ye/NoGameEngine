# ===============================================================
# X TODO:
# 1. install 

# ===============================================================
# Configuration | CMake 配置
# ===============================================================
# Specify the minimum required version of CMake
cmake_minimum_required(VERSION 3.26)

# Define the project name and specify that it use C++ 
project(NoRenderEngine LANGUAGES C CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF) # 禁用 GNU 扩展，确保标准合规
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 性能优化: 启用多核编译
if(MSVC)
    add_compile_options(/MP)
endif()

# ===============================================================
# Dependencies | 环境依赖
# ===============================================================

# --- Vulkan ---
find_package(Vulkan REQUIRED)

# --- GLFW ---
include(FetchContent)

# Ensure GLFW is built with Vulkan support
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "Enable Vulkan support in GLFW" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4  # stable version
)

# Disable unnecessary parts of GLFW
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE) # 避免安装到系统目录
set(GLFW_BUILD_WIN32    ON  CACHE BOOL "" FORCE)
set(GLFW_BUILD_COCOA    OFF CACHE BOOL "" FORCE) # macOS
set(GLFW_BUILD_WAYLAND  OFF CACHE BOOL "" FORCE) # Linux

FetchContent_MakeAvailable(glfw)

# --- GLM ---
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.3
)
FetchContent_MakeAvailable(glm)
# set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "" FORCE)

# ===============================================================
# Platform-specific Definitions | 平台宏定义
# ===============================================================
if(WIN32)
    add_compile_definitions(VK_USE_PLATFORM_WIN32_KHR)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(VK_USE_PLATFORM_XCB_KHR)  # or Xlib, Wayland
endif()

# ===============================================================
# Shader Compilation (自动化 SPIR-V 转换)
# ===============================================================
# find_program(VULKAN_GLSLC_EXECUTABLE NAMES glslc HINTS ${Vulkan_BIN_DIR})
#
# function(compile_shaders TARGET_NAME SHADER_DIR)
#     file(GLOB SHADERS "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag" "${SHADER_DIR}/*.comp")
#     foreach(SHADER ${SHADERS})
#         get_filename_component(SHADER_NAME ${SHADER} NAME)
#         set(SPIRV_OUT "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
#
#         add_custom_command(
#             OUTPUT ${SPIRV_OUT}
#             COMMAND ${VULKAN_GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV_OUT}
#             DEPENDS ${SHADER}
#             COMMENT "Compiling shader: ${SHADER_NAME}"
#         )
#         list(APPEND SPIRV_BINARY_FILES ${SPIRV_OUT})
#     endforeach()
#     add_custom_target(${TARGET_NAME}_SHADERS DEPENDS ${SPIRV_BINARY_FILES})
#     add_dependencies(${TARGET_NAME} ${TARGET_NAME}_SHADERS)
# endfunction()

# ===============================================================
# Output Directories | 输出目录
# ===============================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
if(WIN32)
    # Windows
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
else()
    # Linux/macOS
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
endif()

# ===============================================================
# Source Files | 源代码文件
# ===============================================================
# file(GLOB_RECURSE SOURCES "src/*.cpp")
set(SOURCES
    src/main.cpp
    # TODO: Add more as needed: src/core/myengine.cpp, etc.
    third_party/volk/volk.c
)

if(NOT SOURCES)
    message(FATAL_ERROR "No source files  found in src/")
endif()

# ===============================================================
# Executable | 可执行文件
# ===============================================================
# Create an executable target named "${PROJECT_NAME}" from main.cpp
add_executable(${PROJECT_NAME} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "nre"
)

# ===============================================================
# Included Directories | 包含头文件
# TODO: update included directories
# ===============================================================
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${PROJECT_NAME}
    SYSTEM PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${Vulkan_INCLUDE_DIRS}
)

# ===============================================================
# Linking Libraries | 链接库文件
# TODO: update ,linked libraries
# ===============================================================
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Vulkan::Vulkan
        glfw
)
# 或者更规范地，创建一个 interface target
# add_library(json INTERFACE)
# target_include_directories(json INTERFACE ${CMAKE_SOURCE_DIR}/third_party)
# target_link_libraries(${PROJECT_NAME} PRIVATE json)
# target_link_libraries(${PROJECT_NAME} PRIVATE EngineCore)

# # 触发 Shader 编译 (假设你的 Shader 放在 /shaders 目录下)
# if(VULKAN_GLSLC_EXECUTABLE)
#    compile_shaders(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
# endif()

# ===============================================================
# Compiler Warnings & Options | 编译器选项
# ===============================================================
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

if(MSVC)
    # MSVC: Enable standard-conforming mode and warnings
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
    # GCC / Clang
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -g)
endif()

# Development helper — enable validation layers in debug builds
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_BUILD)
endif()