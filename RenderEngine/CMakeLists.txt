# ===============================================================
# Specify the minimum required version of CMake
cmake_minimum_required(VERSION 3.26)

# Define the project name and specify that it use C++ 
project(NoRenderEngine LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# ===============================================================

# set(CMAKE_CXX_EXTENSIONS OFF) # 禁用 GNU 扩展，确保标准合规

# 禁用异常/RTTI（可选，适合渲染引擎）
# if(MSVC)
#     add_compile_options(/EHsc) # MSVC 需要这个才能用 STL
# else()
#     add_compile_options(-fno-exceptions -fno-rtti)
# endif()

# 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# file(GLOB_RECURSE SOURCES "src/*.cpp")
set(SOURCES
    src/main.cpp
)

if(NOT SOURCES)
    message(FATAL_ERROR "No source files  found in src/")
endif()
# Create an executable target named "${PROJECT_NAME}" from main.cpp
add_executable(${PROJECT_NAME} ${SOURCES})

# Included Directories
# target_include_directories(${PROJECT_NAME} PRIVATE include)