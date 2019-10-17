#pragma once

#include <vulkan/vulkan.hpp>

class vk_single_time_commands
{
public:
    vk::CommandBuffer command_buffer;

public:
    vk_single_time_commands(vk::Device&, vk::Queue&, vk::CommandPool&);
    ~vk_single_time_commands();

private:
    vk::Device& device;
    vk::Queue& queue;
    vk::CommandPool& command_pool;
};