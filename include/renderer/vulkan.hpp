#pragma once

#include <render_plan.hpp>
#include <util/types.hpp>

#include <vulkan/vulkan.hpp>

#include <vector>

class vulkan_renderer
{
public:
    vulkan_renderer(const uint32_t sample_count);
    std::vector<rgba> render_scene(const render_plan&);

private:
    void create_buffer_and_memory(const vk::DeviceSize, const vk::BufferUsageFlags,
        const vk::MemoryPropertyFlags, vk::UniqueBuffer&, vk::UniqueDeviceMemory&) const;
    vk::UniqueShaderModule load_shader_module(const std::string_view code_path);

private:
    const uint32_t sample_count;

    vk::UniqueInstance vulkan_instance;

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