
cmake_minimum_required(VERSION 3.26)
project(NoRenderEngine LANGUAGES CXX)

# 启用 C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 禁用 GNU 扩展，确保标准合规

# 禁用异常/RTTI（可选，适合渲染引擎）
# if(MSVC)
#     add_compile_options(/EHsc) # MSVC 需要这个才能用 STL
# else()
#     add_compile_options(-fno-exceptions -fno-rtti)
# endif()

# 添加源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE include)