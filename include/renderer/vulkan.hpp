#pragma once

#include <util/colors.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

class vulkan_renderer
{
public:
    vulkan_renderer(const uint32_t sample_count);
    std::vector<rgba> render_scene(const struct render_plan&);

private:
    vk::UniqueShaderModule load_shader_module(const std::string_view code_path);

private:
    const uint32_t sample_count;
    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation",
    };

    vk::UniqueInstance vulkan_instance;
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic> debug_messenger;
    vk::DispatchLoaderDynamic dispatch;

    vk::PhysicalDevice physical_device;
    uint32_t compute_queue_index;
    vk::UniqueDevice device;
    vk::Queue compute_queue;

    vk::UniqueDescriptorSetLayout set_layout;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniqueDescriptorPool descriptor_pool;
    std::vector<vk::UniqueDescriptorSet> descriptor_sets;
    vk::UniquePipeline compute_pipeline;

    vk::UniqueCommandPool command_pool;
};