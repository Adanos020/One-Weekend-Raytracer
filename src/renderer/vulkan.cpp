#include <renderer/vulkan.hpp>
#include <render_plan.hpp>
#include <util/vk_mem_alloc.hpp>

#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std::string_literals;

// Helpers

inline static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "[ERR] " << pCallbackData->pMessage << std::endl;
    }
    else
    {
        std::cout << pCallbackData->pMessage << std::endl;
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

    const auto debug_messenger_info = vk::DebugUtilsMessengerCreateInfoEXT{}
        .setMessageSeverity(MsgSeverity::eVerbose | MsgSeverity::eWarning | MsgSeverity::eError)
        .setMessageType(MsgType::eGeneral | MsgType::eValidation | MsgType::ePerformance)
        .setPfnUserCallback(debug_callback);

    const std::vector<const char*> extensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    };

    const auto application_info = vk::ApplicationInfo{}
        .setApiVersion(VK_MAKE_VERSION(1, 1, 0));
    this->vulkan_instance = vk::createInstanceUnique(vk::InstanceCreateInfo{}
        .setPApplicationInfo(&application_info)
        .setPNext(&debug_messenger_info)
        .setPpEnabledLayerNames(this->validation_layers.data())
        .setEnabledLayerCount(this->validation_layers.size())
        .setEnabledExtensionCount(extensions.size())
        .setPpEnabledExtensionNames(extensions.data()));
    this->dispatch.init(*this->vulkan_instance);

    this->debug_messenger = this->vulkan_instance->createDebugUtilsMessengerEXTUnique(
        debug_messenger_info, nullptr, this->dispatch);

    // Pick physical device and create logical device

    std::cout << "Initializing Vulkan... ";
    
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

    // Setup pipeline

    const std::vector<vk::DescriptorSetLayoutBinding> bindings = {
        vk::DescriptorSetLayoutBinding{} // outImage
            .setBinding(0)
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute),
//         vk::DescriptorSetLayoutBinding{} // camera
//             .setBinding(1)
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
        vk::DescriptorPoolSize{} // outImage
            .setType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1),
//         vk::DescriptorPoolSize{} // camera
//             .setType(vk::DescriptorType::eUniformBuffer)
//             .setDescriptorCount(1),
    };

    this->descriptor_pool = this->device->createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{}
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
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

    std::vector<rgba> image{ plan.image_size.width * plan.image_size.height, rgba{ 1 } };

    // Determine buffer and memory sizes

    const vk::DeviceSize image_buffer_memory_size = sizeof(plan.image_size) + sizeof(this->sample_count) + (image.size() * sizeof(rgba));
//    const vk::DeviceSize camera_buffer_memory_size = sizeof(plan.cam);
    
    const vk::DeviceSize render_plan_memory_size = image_buffer_memory_size
//        + camera_buffer_memory_size
    ;

    // Create buffers

    vk::UniqueBuffer image_buffer = this->device->createBufferUnique(vk::BufferCreateInfo{}
        .setSize(image_buffer_memory_size)
        .setUsage(vk::BufferUsageFlagBits::eStorageBuffer)
        .setSharingMode(vk::SharingMode::eExclusive));

    const vk::MemoryRequirements requirements = this->device->getBufferMemoryRequirements(*image_buffer);
    vk::UniqueDeviceMemory render_plan_memory = this->device->allocateMemoryUnique(vk::MemoryAllocateInfo{}
        .setAllocationSize(requirements.size)
        .setMemoryTypeIndex(find_memory_type(this->physical_device, requirements.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)));

    this->device->bindBufferMemory(*image_buffer, *render_plan_memory, 0);

//     vk::UniqueBuffer camera_buffer = this->device->createBufferUnique(vk::BufferCreateInfo{}
//         .setSize(camera_buffer_memory_size)
//         .setUsage(vk::BufferUsageFlagBits::eStorageBuffer)
//         .setSharingMode(vk::SharingMode::eExclusive));
//     this->device->getBufferMemoryRequirements(*camera_buffer);
//     this->device->bindBufferMemory(*camera_buffer, *render_plan_memory, image_buffer_memory_size);

    // Copy data over

    { // image
        void* image_data = this->device->mapMemory(*render_plan_memory, 0, sizeof(plan.image_size) + sizeof(this->sample_count));
        std::memcpy(image_data, &plan.image_size, sizeof(plan.image_size));
        std::memcpy(static_cast<char*>(image_data) + sizeof(plan.image_size), &this->sample_count, sizeof(this->sample_count));
        this->device->unmapMemory(*render_plan_memory);
    }

//     { // camera
//         void* camera_data = this->device->mapMemory(*render_plan_memory, image_buffer_memory_size, camera_buffer_memory_size);
//         std::memcpy(camera_data, &plan.cam, sizeof(plan.cam));
//         this->device->unmapMemory(*render_plan_memory);
//     }

    // Create descriptor sets

    const std::vector<vk::DescriptorBufferInfo> descriptor_buffers_info = {
        vk::DescriptorBufferInfo{}
            .setBuffer(*image_buffer)
            .setRange(VK_WHOLE_SIZE),
//         vk::DescriptorBufferInfo{}
//             .setBuffer(*camera_buffer)
//             .setRange(VK_WHOLE_SIZE),
    };

    for (size_t i = 0; i < this->descriptor_sets.size(); ++i)
    {
        this->device->updateDescriptorSets(vk::WriteDescriptorSet{}
            .setDstSet(*this->descriptor_sets[i])
            .setDescriptorType(vk::DescriptorType::eStorageBuffer)
            .setDescriptorCount(1)
            .setPBufferInfo(&descriptor_buffers_info[i]), nullptr);
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
        command_buffer->dispatch(plan.image_size.width, plan.image_size.height, 1);
        command_buffer->end();

        this->compute_queue.submit(vk::SubmitInfo{}
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*command_buffer), nullptr);
    }

    this->compute_queue.waitIdle();

    void* image_data = this->device->mapMemory(*render_plan_memory, 0, image_buffer_memory_size);
    std::memcpy(image.data(), image_data, image_buffer_memory_size);
    this->device->unmapMemory(*render_plan_memory);
    
    std::cout << "Done." << std::endl;

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