// main.cpp - 用于测试 clangd 配置（C++23）

#include <expected> // C++23
#include <iostream>
#include <mdspan> // C++23
#include <ranges> // C++20/23
#include <span>   // C++20
#include <string>
#include <vector>

// 测试 ShowAKA: typedef / using 别名
using VertexID = int;
using Color = unsigned int;

// 测试 unused-variable / unused-parameter 抑制
void dummy_func(int unused_param)
{
    int unused_var = 42;
    (void)unused_var; // 手动抑制，但 clangd 应因配置忽略警告
}

// 测试 std::expected (C++23)
auto load_texture(const std::string& path) -> std::expected<std::vector<unsigned char>, std::string>
{
    if (path.empty()) {
        return std::unexpected("Path is empty");
    }
    return std::vector<unsigned char>{0xFF, 0x00, 0xFF, 0x00}; // mock RGBA
}

// 测试 constexpr + consteval
consteval int square(int x)
{
    return x * x;
}

constexpr auto compute_matrix_size()
{
    return square(4); // 编译期计算
}

// 测试 mdspan (C++23) - 多维数据视图
void process_image(std::span<unsigned char> pixels, int width, int height)
{
    auto img_view = std::mdspan(pixels.data(), height, width, 4); // H x W x RGBA
    // clangd 应能正确推导 img_view 类型（配合 InlayHints.DeducedTypes）
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            img_view[y, x, 0] = 255; // R
        }
    }
}

int main()
{
    // 测试 DeducedTypes + ParameterNames inlay hints
    auto data = std::vector{1, 2, 3, 4, 5};
    auto doubled = data | std::views::transform([](int x) { return x * 2; });

    // 测试 Hover.ShowAKA: 悬停 VertexID 应显示 "int (aka VertexID)"
    VertexID vid = 1001;
    Color c = 0xFF00FF00;

    // 测试 std::expected 使用
    auto tex = load_texture("grass.png");
    if (!tex) {
        std::cerr << "Error: " << tex.error() << "\n";
    }

    // 测试 constexpr 值
    constexpr int mat_size = compute_matrix_size(); // 应为 16

    // 测试 mdspan 调用
    std::vector<unsigned char> buffer(100 * 100 * 4);
    process_image(buffer, 100, 100);

    // 故意留下未使用变量（应被 Suppress）
    int debug_flag = 1;

    std::cout << "Clangd config test passed! Matrix size: " << mat_size << "\n";
    return 0;
}