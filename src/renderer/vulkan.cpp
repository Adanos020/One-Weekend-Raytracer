#include <renderer/vulkan.hpp>
#include <render_plan.hpp>

#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std::string_literals;

#pragma clang optimize off

// Helpers

inline static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        std::cerr << std::endl << "[ERR] " << pCallbackData->pMessage << std::endl;
    }
    else if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << std::endl << "[!] " << pCallbackData->pMessage << std::endl;
    }
    else
    {
        std::cout << std::endl << pCallbackData->pMessage << std::endl;
    }
    return VK_FALSE;
}

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
    // Create instance and debug messenger

    using MsgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using MsgType = vk::DebugUtilsMessageTypeFlagBitsEXT;

    const auto application_info = vk::ApplicationInfo{}
        .setApiVersion(VK_MAKE_VERSION(1, 1, 0));
    const auto debug_messenger_info = vk::DebugUtilsMessengerCreateInfoEXT{}
        .setMessageSeverity(MsgSeverity::eVerbose | MsgSeverity::eWarning | MsgSeverity::eError)
        .setMessageType(MsgType::eGeneral | MsgType::eValidation | MsgType::ePerformance)
        .setPfnUserCallback(debug_callback);

    this->vulkan_instance = vk::createInstanceUnique(vk::InstanceCreateInfo{}
        .setPApplicationInfo(&application_info)
        .setPNext(&debug_messenger_info)
        .setPpEnabledLayerNames(this->validation_layers.data())
        .setEnabledLayerCount(this->validation_layers.size())
        .setEnabledExtensionCount(this->extensions.size())
        .setPpEnabledExtensionNames(this->extensions.data()));
    this->dispatch.init(*this->vulkan_instance);
    this->debug_messenger = this->vulkan_instance->createDebugUtilsMessengerEXTUnique(
        debug_messenger_info, nullptr, this->dispatch);

    // Pick physical device and create logical device

    std::cout << "Picking physical device... ";
    
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
        .setEnabledLayerCount(this->validation_layers.size())
        .setPpEnabledLayerNames(this->validation_layers.data())
        .setQueueCreateInfoCount(1)
        .setPQueueCreateInfos(&compute_queue_info));

    this->compute_queue = this->device->getQueue(this->compute_queue_index, 0);

    this->memory_allocator = vma::createAllocator(vma::AllocatorCreateInfo{}
        .setPhysicalDevice(this->physical_device)
        .setDevice(*this->device));

    std::cout << "Done." << std::endl;

    // Setup pipeline

    std::cout << "Setting up ";

    const std::vector<vk::DescriptorSetLayoutBinding> bindings = {
         vk::DescriptorSetLayoutBinding{} // scene
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute),
         vk::DescriptorSetLayoutBinding{} // textureImages
            .setBinding(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute),
        vk::DescriptorSetLayoutBinding{} // outImage
            .setBinding(2)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute),
    };

    std::cout << "set layout... ";

    this->set_layout = this->device->createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo{}
        .setBindingCount(bindings.size())
        .setPBindings(bindings.data()));

    std::cout << "pipeline layout... ";

    const std::vector<vk::PushConstantRange> push_constant_ranges = {
        vk::PushConstantRange{}
            .setStageFlags(vk::ShaderStageFlagBits::eCompute)
            .setOffset(0)
            .setSize(sizeof(render_info)),
    };

    this->pipeline_layout = this->device->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo{}
        .setSetLayoutCount(1)
        .setPSetLayouts(&this->set_layout.get())
        .setPushConstantRangeCount(push_constant_ranges.size())
        .setPPushConstantRanges(push_constant_ranges.data()));

    std::cout << "compute pipeline... ";

    this->compute_pipeline = this->device->createComputePipelineUnique(nullptr, vk::ComputePipelineCreateInfo{}
        .setStage(vk::PipelineShaderStageCreateInfo{}
            .setStage(vk::ShaderStageFlagBits::eCompute)
            .setModule(*this->load_shader_module("shaders/raytracer.comp.spv"))
            .setPName("main"))
        .setLayout(*this->pipeline_layout));

    std::cout << "Done." << std::endl;

    // Create descriptor sets

    std::cout << "Creating descriptor pool... ";

    const std::vector<vk::DescriptorPoolSize> descriptor_pool_sizes = {
        vk::DescriptorPoolSize{} // scene, textureImages
            .setType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(2),
        vk::DescriptorPoolSize{} // outImage
            .setType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1),
    };

    this->descriptor_pool = this->device->createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{}
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
        .setPoolSizeCount(descriptor_pool_sizes.size())
        .setPPoolSizes(descriptor_pool_sizes.data())
        .setMaxSets(1));

    std::cout << "Done." << std::endl;
    std::cout << "Creating descriptor sets... ";

    this->descriptor_sets = this->device->allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo{}
        .setDescriptorPool(*this->descriptor_pool)
        .setDescriptorSetCount(1)
        .setPSetLayouts(&*this->set_layout));

    std::cout << "Done." << std::endl;

    // Create command pool

    std::cout << "Creating command pool... ";

    this->command_pool = this->device->createCommandPoolUnique(vk::CommandPoolCreateInfo{}
        .setQueueFamilyIndex(this->compute_queue_index));

    std::cout << "Done." << std::endl;
}

vulkan_renderer::~vulkan_renderer()
{
    this->memory_allocator.destroy();
}

std::vector<rgba> vulkan_renderer::render_scene(const render_plan& plan)
{
    std::vector<rgba> image{ plan.image_size.width * plan.image_size.height, rgba{ 1 } };

    // Create buffers

    std::cout << "Creating buffers for ";

    const vk::DeviceSize scene_buffer_size = 1;
    const vk::DeviceSize texture_images_buffer_size = 1;
    const vk::DeviceSize image_buffer_size = image.size() * sizeof(rgba);

    std::cout << "scene... ";

    auto [scene_buffer, scene_allocation] = this->memory_allocator.createBuffer(
        vk::BufferCreateInfo{}
            .setSize(scene_buffer_size)
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
            .setSharingMode(vk::SharingMode::eExclusive),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eGpuOnly)
            .setRequiredFlags(vk::MemoryPropertyFlagBits::eHostVisible)
            .setPreferredFlags(vk::MemoryPropertyFlagBits::eHostCoherent));

    std::cout << "texture images... ";

    auto [texture_images_buffer, texture_images_allocation] = this->memory_allocator.createBuffer(
        vk::BufferCreateInfo{}
            .setSize(texture_images_buffer_size)
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
            .setSharingMode(vk::SharingMode::eExclusive),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eGpuOnly)
            .setRequiredFlags(vk::MemoryPropertyFlagBits::eHostVisible)
            .setPreferredFlags(vk::MemoryPropertyFlagBits::eHostCoherent));

    std::cout << "output image... ";

    auto [image_buffer, image_allocation] = this->memory_allocator.createBuffer(
        vk::BufferCreateInfo{}
            .setSize(image_buffer_size)
            .setUsage(vk::BufferUsageFlagBits::eStorageBuffer)
            .setSharingMode(vk::SharingMode::eExclusive),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eGpuToCpu)
            .setRequiredFlags(vk::MemoryPropertyFlagBits::eHostVisible)
            .setPreferredFlags(vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostCoherent)
            .setFlags(vma::AllocationCreateFlagBits::eMapped));

    // Create descriptor sets

    struct descriptor_set_update_info
    {
        std::vector<vk::DescriptorBufferInfo> descriptor_buffer_infos;
        vk::DescriptorType descriptor_type;
    };
    const std::vector<descriptor_set_update_info> descriptor_set_update_infos = {
        descriptor_set_update_info{
            {
                vk::DescriptorBufferInfo{}
                    .setBuffer(scene_buffer)
                    .setRange(VK_WHOLE_SIZE),
                vk::DescriptorBufferInfo{}
                    .setBuffer(texture_images_buffer)
                    .setRange(VK_WHOLE_SIZE),
            }, vk::DescriptorType::eUniformBuffer
        },
        descriptor_set_update_info{
            {
                vk::DescriptorBufferInfo{}
                    .setBuffer(image_buffer)
                    .setRange(VK_WHOLE_SIZE),
            }, vk::DescriptorType::eStorageBuffer
        }
    };

    for (size_t i = 0; i < this->descriptor_sets.size(); ++i)
    {
        const descriptor_set_update_info& dsui = descriptor_set_update_infos[i];
        this->device->updateDescriptorSets(vk::WriteDescriptorSet{}
            .setDstSet(*this->descriptor_sets[i])
            .setDescriptorType(dsui.descriptor_type)
            .setDescriptorCount(dsui.descriptor_buffer_infos.size())
            .setPBufferInfo(dsui.descriptor_buffer_infos.data()), nullptr);
    }

    std::cout << "Done." << std::endl;

    // Record commands to buffers and execute

    std::cout << "Rendering scene... ";

    std::vector<vk::UniqueCommandBuffer> command_buffers = this->device->allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo{}
            .setCommandPool(*this->command_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1));

    const render_info info = { plan.image_size, this->sample_count, plan.cam };
    for (size_t i = 0; i < command_buffers.size(); ++i)
    {
        vk::UniqueCommandBuffer& command_buffer = command_buffers[i];

        command_buffer->begin(vk::CommandBufferBeginInfo{});
        command_buffer->bindPipeline(vk::PipelineBindPoint::eCompute, *this->compute_pipeline);

        command_buffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, *this->pipeline_layout, 0,
            1, &*this->descriptor_sets[i],
            0, {});
        
        command_buffer->pushConstants(*this->pipeline_layout, vk::ShaderStageFlagBits::eCompute,
            0, sizeof(render_info), &info);
        
        command_buffer->dispatch(plan.image_size.width, plan.image_size.height, 1);
        command_buffer->end();

        this->compute_queue.submit(vk::SubmitInfo{}
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*command_buffer), nullptr);
    }

    this->compute_queue.waitIdle();

    { // Retrieve image
        void* data = this->memory_allocator.mapMemory(image_allocation);
        std::memcpy(image.data(), data, image_buffer_size);
        this->memory_allocator.unmapMemory(image_allocation);
    }
    
    std::cout << "Done." << std::endl;

    // Cleanup
    this->memory_allocator.destroyBuffer(scene_buffer, scene_allocation);
    this->memory_allocator.destroyBuffer(texture_images_buffer, texture_images_allocation);
    this->memory_allocator.destroyBuffer(image_buffer, image_allocation);

    return image;
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