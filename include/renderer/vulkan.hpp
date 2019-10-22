#pragma once

#ifndef NDEBUG
#   define VK_ENABLE_VALIDATION_LAYERS
#endif

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
    void setup_pipeline(const struct render_plan&);
    void create_descriptor_sets();
    void create_command_pool();

private:
    vk::UniqueShaderModule load_shader_module(const std::string_view code_path, const struct render_plan&) const;
    
    void copy_to_memory(const vma::Allocation&, const void* data, const size_t size) const;
    void copy_from_memory(const vma::Allocation&, void* data, const size_t size) const;

    void copy_buffer(const vk::Buffer& source, vk::Buffer& destination, const vk::DeviceSize size);
    
    void transfer_image(vk::Image&, const vk::ImageLayout old_layout, const vk::ImageLayout new_layout,
        const vk::PipelineStageFlagBits src_stage, const vk::PipelineStageFlagBits dst_stage);

private:
    const uint32_t sample_count;

#ifdef VK_ENABLE_VALIDATION_LAYERS
    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation",
    };
    const std::vector<const char*> extensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };
#endif

    vk::UniqueInstance vulkan_instance;
#ifdef VK_ENABLE_VALIDATION_LAYERS
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> debug_messenger;
    vk::DispatchLoaderDynamic dispatch;
#endif

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

    std::pair<vk::Buffer, vma::Allocation> scene_memory;
    std::pair<vk::Buffer, vma::Allocation> texture_images_memory;
    std::pair<vk::Image, vma::Allocation> output_image_memory;

    bool unclean = false;

    vk::UniqueCommandPool command_pool;
};