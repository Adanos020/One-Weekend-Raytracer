#define VULKAN_TEST 1

#if VULKAN_TEST
#   include <renderer/vulkan.hpp>
#   include <scene_definitions_for_vulkan/render_plan.hpp>
#else
#   include <render_plan.hpp>
#   include <renderer/cpu.hpp>
#endif
#include <util/string.hpp>
#include <util/sizes.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <iostream>
#include <string>

using namespace std::string_literals;

void export_image(const std::vector<rgba>& image, const extent_2d<uint32_t> image_size,
    const std::string_view path)
{
    std::cout << "Writing to file... ";

    static const uint32_t channels = 4;
    if (string_ends_with(path, ".png"))
    {
        stbi_write_png(path.data(), image_size.width, image_size.height, channels,
            image.data(), image_size.width * channels);
    }
    else if (string_ends_with(path, ".jpg"))
    {
        static const int32_t quality = 100;
        stbi_write_jpg(path.data(), image_size.width, image_size.height, channels,
            image.data(), quality);
    }
    else
    {
        const size_t last_dot_pos = path.find_last_of('.');
        const std::string_view format = path.substr(last_dot_pos + 1, path.size() - last_dot_pos - 1);
        throw std::runtime_error("Unsupported image format: "s + format.data());
    }

    std::cout << "Done." << std::endl;
}

int main()
{
    try
    {
        const extent_2d<uint32_t> image_size = { 1600, 900 };
#if VULKAN_TEST
        const render_plan plan = render_plan::hello_ball(image_size);
        const std::vector<rgba> image = vulkan_renderer{ 1000 }.render_scene(plan);
#else
        const render_plan plan = render_plan::random_balls(image_size);
        const std::vector<rgba> image = cpu_renderer{ 1000, 20 }.render_scene(plan);
#endif
        export_image(image, image_size, "test.png");
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const vk::Error& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}