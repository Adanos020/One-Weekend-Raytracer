#pragma once

#include <camera.hpp>
#include <util/colors.hpp>
#include <util/sizes.hpp>
#include <util/vk_mem_alloc.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

class vulkan_renderer
{
    struct render_info
    {
        extent_2d<uint32_t> image_extent;
        uint32_t sample_count;
        camera cam;
    };

public:
    vulkan_renderer(const uint32_t sample_count);
    ~vulkan_renderer();

    std::vector<rgba> render_scene(const struct render_plan&);

private:
    void create_instance();
    void setup_devices();
    void setup_pipeline();
    void create_descriptor_sets();
    void create_command_pool();

private:
    vk::UniqueShaderModule load_shader_module(const std::string_view code_path) const;

private:
    const uint32_t sample_count;
    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation",
    };
    const std::vector<const char*> extensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    vk::UniqueInstance vulkan_instance;
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> debug_messenger;
    vk::DispatchLoaderDynamic dispatch;

    vk::PhysicalDevice physical_device;
    uint32_t compute_queue_index;
    vk::UniqueDevice device;
    vk::Queue compute_queue;

    vma::Allocator memory_allocator;

    vk::UniqueDescriptorSetLayout set_layout;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniqueDescriptorPool descriptor_pool;
    std::vector<vk::UniqueDescriptorSet> descriptor_sets;
    vk::UniquePipeline compute_pipeline;

    vk::UniqueCommandPool command_pool;
};