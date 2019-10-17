#include <util/vk_single_time_commands.hpp>

vk_single_time_commands::vk_single_time_commands(vk::Device& device, vk::Queue& queue, vk::CommandPool& command_pool)
    : device(device), queue(queue), command_pool(command_pool)
{
    this->command_buffer = this->device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{}
        .setLevel(vk::CommandBufferLevel::ePrimary)
        .setCommandPool(this->command_pool)
        .setCommandBufferCount(1)).front();
    this->command_buffer.begin(vk::CommandBufferBeginInfo{}.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
}

vk_single_time_commands::~vk_single_time_commands()
{
    this->command_buffer.end();

    this->queue.submit(vk::SubmitInfo{}
        .setCommandBufferCount(1)
        .setPCommandBuffers(&this->command_buffer), nullptr);
    this->queue.waitIdle();

    this->device.freeCommandBuffers(this->command_pool, this->command_buffer);
}