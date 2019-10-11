#include <renderer/vulkan.hpp>

#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std::string_literals;

// Helpers

std::optional<uint32_t> find_compute_queue_family(const vk::PhysicalDevice& device)
{
    const std::vector<vk::QueueFamilyProperties> queue_families = device.getQueueFamilyProperties();
    for (uint32_t i = 0; i < queue_families.size(); ++i)
    {
        if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags == vk::QueueFlagBits::eCompute)
        {
            return i;
        }
    }
    return {};
}

uint32_t find_memory_type(const vk::PhysicalDevice& physical_device, const uint32_t type_filter,
    const vk::MemoryPropertyFlags properties)
{
    const vk::PhysicalDeviceMemoryProperties memory_properties = physical_device.getMemoryProperties();

    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
        const vk::MemoryPropertyFlags memory_type_flags = memory_properties.memoryTypes[i].propertyFlags;
        if ((type_filter & (1 << i)) && (memory_type_flags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find a sufficient memory type.");
}

// Implementation

vulkan_renderer::vulkan_renderer(const uint32_t sample_count)
    : sample_count(sample_count)
{
    // Create instance

    std::cout << "Initializing Vulkan... ";

    const auto application_info = vk::ApplicationInfo{}
        .setApiVersion(VK_MAKE_VERSION(1, 1, 0));
    this->vulkan_instance = vk::createInstanceUnique(vk::InstanceCreateInfo{}
        .setPApplicationInfo(&application_info));

    // Pick physical device and create logical device
    
    const std::vector<vk::PhysicalDevice> devices = this->vulkan_instance->enumeratePhysicalDevices();
    for (const vk::PhysicalDevice& device : devices)
    {
        if (std::optional<uint32_t> compute_queue = find_compute_queue_family(device))
        {
            this->physical_device = device;
            this->compute_queue_index = *compute_queue;
            break;
        }
    }

    if (!this->physical_device)
    {
        throw std::runtime_error("Failed to find a suitable device.");
    }

    const float queue_priority = 1.f;
    const auto compute_queue_info = vk::DeviceQueueCreateInfo{}
        .setQueueFamilyIndex(this->compute_queue_index)
        .setQueueCount(1)
        .setPQueuePriorities(&queue_priority);

    this->device = this->physical_device.createDeviceUnique(vk::DeviceCreateInfo{}
        .setQueueCreateInfoCount(1)
        .setPQueueCreateInfos(&compute_queue_info));

    this->compute_queue = this->device->getQueue(this->compute_queue_index, 0);

    // Setup pipeline

    const std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        vk::DescriptorSetLayoutBinding{} // outImage
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute),
//         vk::DescriptorSetLayoutBinding{} // scene
//             .setBinding(1)
//             .setDescriptorType(vk::DescriptorType::eUniformBuffer)
//             .setDescriptorCount(1)
//             .setStageFlags(vk::ShaderStageFlagBits::eCompute),
//         vk::DescriptorSetLayoutBinding{} // textureImages
//             .setBinding(2)
//             .setDescriptorType(vk::DescriptorType::eUniformBuffer)
//             .setDescriptorCount(1)
//             .setStageFlags(vk::ShaderStageFlagBits::eCompute),
    };

    this->set_layout = this->device->createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo{}
        .setBindingCount(bindings.size())
        .setPBindings(bindings.data()));

    this->pipeline_layout = this->device->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo{}
        .setSetLayoutCount(1)
        .setPSetLayouts(&this->set_layout.get()));

    this->compute_pipeline = this->device->createComputePipelineUnique(nullptr, vk::ComputePipelineCreateInfo{}
        .setStage(vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(*this->load_shader_module("shaders/raytracer.comp.spv"))
            .setPName("main"))
        .setLayout(*this->pipeline_layout));

    // Create descriptor sets

    std::vector<vk::DescriptorPoolSize> descriptor_pool_sizes = {
        vk::DescriptorPoolSize{}
            .setType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1),
    };

    this->descriptor_pool = this->device->createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{}
        .setPoolSizeCount(descriptor_pool_sizes.size())
        .setPPoolSizes(descriptor_pool_sizes.data())
        .setMaxSets(1));

    this->descriptor_sets = this->device->allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo{}
        .setDescriptorPool(*this->descriptor_pool)
        .setDescriptorSetCount(1)
        .setPSetLayouts(&*this->set_layout));

    // Create command pool

    this->command_pool = this->device->createCommandPoolUnique(vk::CommandPoolCreateInfo{}
        .setQueueFamilyIndex(this->compute_queue_index));

    std::cout << "Done." << std::endl;
}

std::vector<rgba> vulkan_renderer::render_scene(const render_plan& plan)
{
    std::cout << "Creating buffers... ";

    std::vector<rgba> image{ plan.image_size.width * plan.image_size.height, rgba{ 0 } };

    // Create image buffer

    vk::UniqueBuffer image_buffer;
    vk::UniqueDeviceMemory image_buffer_memory;
    const vk::DeviceSize image_buffer_memory_size = image.size() * sizeof(rgba);
    this->create_buffer_and_memory(image_buffer_memory_size, vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
        image_buffer, image_buffer_memory);

    const auto descriptor_buffer_info = vk::DescriptorBufferInfo{}
        .setBuffer(*image_buffer)
        .setRange(VK_WHOLE_SIZE);

    for (vk::UniqueDescriptorSet& descriptor_set : this->descriptor_sets)
    {
        this->device->updateDescriptorSets(vk::WriteDescriptorSet{}
            .setDstSet(*descriptor_set)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setPBufferInfo(&descriptor_buffer_info), nullptr);
    }

    std::cout << "Done." << std::endl;

    // Record commands to buffers and execute

    std::cout << "Rendering scene... ";

    std::vector<vk::UniqueCommandBuffer> command_buffers = this->device->allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo{}
            .setCommandPool(*this->command_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1));

    for (size_t i = 0; i < command_buffers.size(); ++i)
    {
        vk::UniqueCommandBuffer& command_buffer = command_buffers[i];

        command_buffer->begin(vk::CommandBufferBeginInfo{});
        command_buffer->bindPipeline(vk::PipelineBindPoint::eCompute, *this->compute_pipeline);
        command_buffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, *this->pipeline_layout, 0,
            1, &*this->descriptor_sets[i],
            0, {});
        command_buffer->dispatch(plan.image_size.width * plan.image_size.height, 1, 1);
        command_buffer->end();

        this->compute_queue.submit(vk::SubmitInfo{}
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*command_buffer), nullptr);
    }

    this->compute_queue.waitIdle();

    void* image_data = this->device->mapMemory(*image_buffer_memory, 0, image_buffer_memory_size);
    std::memcpy(image.data(), image_data, image_buffer_memory_size);
    this->device->unmapMemory(*image_buffer_memory);
    
    std::cout << "Done." << std::endl;

    return image;
}

void vulkan_renderer::create_buffer_and_memory(const vk::DeviceSize size,
    const vk::BufferUsageFlags usage, const vk::MemoryPropertyFlags properties,
    vk::UniqueBuffer& out_buffer, vk::UniqueDeviceMemory& out_memory) const
{
    out_buffer = this->device->createBufferUnique(vk::BufferCreateInfo{}
        .setSize(size)
        .setUsage(usage)
        .setSharingMode(vk::SharingMode::eExclusive));

    const vk::MemoryRequirements requirements = this->device->getBufferMemoryRequirements(*out_buffer);
    out_memory = this->device->allocateMemoryUnique(vk::MemoryAllocateInfo{}
        .setAllocationSize(requirements.size)
        .setMemoryTypeIndex(find_memory_type(this->physical_device, requirements.memoryTypeBits, properties)));

    this->device->bindBufferMemory(*out_buffer, *out_memory, 0);
}

vk::UniqueShaderModule vulkan_renderer::load_shader_module(const std::string_view code_path)
{
    std::ifstream file{ code_path.data(), std::ios::ate | std::ios::binary };
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file "s + code_path.data());
    }

    const size_t file_size = file.tellg();
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);
    file.close();

    return this->device->createShaderModuleUnique(vk::ShaderModuleCreateInfo{}
        .setCodeSize(buffer.size())
        .setPCode(reinterpret_cast<const uint32_t*>(buffer.data())));
}