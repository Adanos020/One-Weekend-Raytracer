#include <renderer/vulkan.hpp>
#include <render_plan.hpp>
#include <util/string.hpp>
#include <util/vk_single_time_commands.hpp>

#include <shaderc/shaderc.hpp>

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

// Implementation

vulkan_renderer::vulkan_renderer(const uint32_t sample_count)
    : sample_count(sample_count)
{
    this->create_instance();
    this->setup_devices();
    this->setup_pipeline();
    this->create_descriptor_sets();
    this->create_command_pool();
}

vulkan_renderer::~vulkan_renderer()
{
    this->memory_allocator.destroy();
}

std::vector<rgba> vulkan_renderer::render_scene(const render_plan& plan)
{
    const vk::Extent3D image_extent = { plan.image_size.width, plan.image_size.height, 1 };
    std::vector<rgba> image{ image_extent.width * image_extent.height, rgba{ 1 } };

    const vk::DeviceSize scene_buffer_size = 1;
    const vk::DeviceSize texture_images_buffer_size = 1;

    std::cout << "Allocating memory for scene... ";

    auto [scene_buffer, scene_allocation] = this->memory_allocator.createBuffer(
        vk::BufferCreateInfo{}
            .setSize(scene_buffer_size)
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eGpuOnly));

    std::cout << "texture images... ";

    auto [texture_images_buffer, texture_images_allocation] = this->memory_allocator.createBuffer(
        vk::BufferCreateInfo{}
            .setSize(texture_images_buffer_size)
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eGpuOnly));

    std::cout << "output image... ";

    auto [output_image, image_allocation] = this->memory_allocator.createImage(
        vk::ImageCreateInfo{}
            .setImageType(vk::ImageType::e2D)
            .setExtent(image_extent)
            .setMipLevels(1)
            .setArrayLayers(1)
            .setFormat(vk::Format::eR8G8B8A8Uint)
            .setTiling(vk::ImageTiling::eOptimal)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setUsage(vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setSamples(vk::SampleCountFlagBits::e1),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eGpuOnly));
    vk::UniqueImageView output_image_view = this->device->createImageViewUnique(vk::ImageViewCreateInfo{}
        .setImage(output_image)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(vk::Format::eR8G8B8A8Uint)
        .setSubresourceRange(vk::ImageSubresourceRange{}
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1)));
    {
        vk_single_time_commands transition_layout{ *this->device, this->compute_queue, *this->command_pool };
        transition_layout.command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlags{}, {}, {}, vk::ImageMemoryBarrier{}
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eGeneral)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setImage(output_image)
                .setSubresourceRange(vk::ImageSubresourceRange{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1)));
    }

    std::cout << "Done." << std::endl;
    std::cout << "Updating descriptor sets... ";

    const auto scene_descriptor_info = vk::DescriptorBufferInfo{}
        .setBuffer(scene_buffer)
        .setRange(VK_WHOLE_SIZE);
    const auto texture_images_descriptor_info = vk::DescriptorBufferInfo{}
        .setBuffer(texture_images_buffer)
        .setRange(VK_WHOLE_SIZE);
    const auto image_descriptor_info = vk::DescriptorImageInfo{}
        .setImageLayout(vk::ImageLayout::eGeneral)
        .setImageView(*output_image_view);

    for (size_t i = 0; i < this->descriptor_sets.size(); ++i)
    {
        const std::vector<vk::WriteDescriptorSet> descriptor_writes = {
            vk::WriteDescriptorSet{}
                .setDstBinding(0)
                .setDstSet(*this->descriptor_sets[i])
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setPBufferInfo(&scene_descriptor_info),
            vk::WriteDescriptorSet{}
                .setDstBinding(1)
                .setDstSet(*this->descriptor_sets[i])
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDescriptorCount(1)
                .setPBufferInfo(&texture_images_descriptor_info),
            vk::WriteDescriptorSet{}
                .setDstBinding(2)
                .setDstSet(*this->descriptor_sets[i])
                .setDescriptorType(vk::DescriptorType::eStorageImage)
                .setDescriptorCount(1)
                .setPImageInfo(&image_descriptor_info),
        };
        this->device->updateDescriptorSets(descriptor_writes, nullptr);
    }

    std::cout << "Done." << std::endl;
    std::cout << "Rendering scene... ";

    std::vector<vk::UniqueCommandBuffer> command_buffers = this->device->allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo{}
            .setCommandPool(*this->command_pool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(this->descriptor_sets.size()));

    for (size_t i = 0; i < command_buffers.size(); ++i)
    {
        vk::UniqueCommandBuffer& command_buffer = command_buffers[i];

        command_buffer->begin(vk::CommandBufferBeginInfo{});
        command_buffer->bindPipeline(vk::PipelineBindPoint::eCompute, *this->compute_pipeline);

        command_buffer->bindDescriptorSets(vk::PipelineBindPoint::eCompute, *this->pipeline_layout, 0,
            1, &*this->descriptor_sets[i],
            0, {});

        const render_info render_meta = { plan.image_size, this->sample_count, plan.cam };
        command_buffer->pushConstants(*this->pipeline_layout, vk::ShaderStageFlagBits::eCompute,
            0, sizeof(render_info), &render_meta);
        
        command_buffer->dispatch(plan.image_size.width, plan.image_size.height, 1);
        command_buffer->end();

        this->compute_queue.submit(vk::SubmitInfo{}
            .setCommandBufferCount(1)
            .setPCommandBuffers(&*command_buffer), nullptr);
    }

    this->compute_queue.waitIdle();

    std::cout << "Done." << std::endl;
    std::cout << "Retrieving image... ";

    { // Transfer image layout
        vk_single_time_commands transition_layout{ *this->device, this->compute_queue, *this->command_pool };
        transition_layout.command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eHost,
            vk::DependencyFlags{}, {}, {}, vk::ImageMemoryBarrier{}
            .setOldLayout(vk::ImageLayout::eGeneral)
            .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setImage(output_image)
            .setSubresourceRange(vk::ImageSubresourceRange{}
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseMipLevel(0)
                .setLevelCount(1)
                .setBaseArrayLayer(0)
                .setLayerCount(1)));
    }

    // Retrieve image
    const vk::DeviceSize output_image_size = image.size() * sizeof(rgba);
    auto [staging_buffer, staging_allocation] = this->memory_allocator.createBuffer(
        vk::BufferCreateInfo{}
            .setSize(output_image_size)
            .setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eCpuOnly));
    {
        vk_single_time_commands retrieve_image{ *this->device, this->compute_queue, *this->command_pool };
        retrieve_image.command_buffer.copyImageToBuffer(output_image, vk::ImageLayout::eTransferSrcOptimal,
            staging_buffer, vk::BufferImageCopy{}
                .setBufferOffset(0)
                .setBufferRowLength(0)
                .setBufferImageHeight(0)
                .setImageSubresource(vk::ImageSubresourceLayers{}
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setMipLevel(0)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1))
                .setImageOffset(vk::Offset3D{ 0, 0, 0 })
                .setImageExtent(image_extent));
    }
    void* output_data = this->memory_allocator.mapMemory(staging_allocation);
    std::memcpy(image.data(), output_data, output_image_size);
    this->memory_allocator.unmapMemory(staging_allocation);
    this->memory_allocator.destroyBuffer(staging_buffer, staging_allocation);
    
    std::cout << "Done." << std::endl;

    // Cleanup
    this->memory_allocator.destroyBuffer(scene_buffer, scene_allocation);
    this->memory_allocator.destroyBuffer(texture_images_buffer, texture_images_allocation);
    this->memory_allocator.destroyImage(output_image, image_allocation);

    return image;
}

void vulkan_renderer::create_instance()
{
    using MsgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using MsgType = vk::DebugUtilsMessageTypeFlagBitsEXT;

    std::cout << "Creating Vulkan instance... ";

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

    std::cout << "Done." << std::endl;
}

void vulkan_renderer::setup_devices()
{
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
}

void vulkan_renderer::setup_pipeline()
{
    std::cout << "Setting up descriptor set layout... ";

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
            .setDescriptorType(vk::DescriptorType::eStorageImage)
            .setDescriptorCount(1)
            .setStageFlags(vk::ShaderStageFlagBits::eCompute),
    };

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
            .setModule(*this->load_shader_module("shaders/raytracer.comp"))
            .setPName("main"))
        .setLayout(*this->pipeline_layout));

    std::cout << "Done." << std::endl;
}

void vulkan_renderer::create_descriptor_sets()
{
    std::cout << "Creating descriptor pool... ";

    const std::vector<vk::DescriptorPoolSize> descriptor_pool_sizes = {
        vk::DescriptorPoolSize{} // scene, textureImages
            .setType(vk::DescriptorType::eUniformBuffer)
            .setDescriptorCount(2),
        vk::DescriptorPoolSize{} // outImage
            .setType(vk::DescriptorType::eStorageImage)
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
}

void vulkan_renderer::create_command_pool()
{
    std::cout << "Creating command pool... ";

    this->command_pool = this->device->createCommandPoolUnique(vk::CommandPoolCreateInfo{}
        .setQueueFamilyIndex(this->compute_queue_index));

    std::cout << "Done." << std::endl;
}

vk::UniqueShaderModule vulkan_renderer::load_shader_module(const std::string_view code_path) const
{
    std::ifstream file{ code_path.data(), std::ios::ate };
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file "s + code_path.data());
    }

    const size_t file_size = file.tellg();
    file.seekg(0);

    std::string code;
    code.reserve(file_size);
    code.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    file.close();

    string_replace_all(code, "@SCENE_SHAPES_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_SPHERE_SHAPES_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_DIELECTRIC_MATERIALS_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_DIFFUSE_LIGHT_MATERIALS_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_LAMBERTIAN_MATERIALS_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_METAL_MATERIALS_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_CHECKER_TEXTURES_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_CONSTANT_TEXTURES_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_IMAGE_TEXTURES_COUNT@", std::to_string(1));
    string_replace_all(code, "@SCENE_NOISE_TEXTURES_COUNT@", std::to_string(1));

    std::cout << std::endl << "Compiling shaders... ";

    shaderc::Compiler compiler;
    const shaderc::SpvCompilationResult compilation_result = compiler.CompileGlslToSpv(
        code, shaderc_shader_kind::shaderc_compute_shader, code_path.data());
    if (compilation_result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        throw std::runtime_error(compilation_result.GetErrorMessage());
    }
    std::vector<uint32_t> spv(compilation_result.cbegin(), compilation_result.cend());

    std::cout << "Done." << std::endl;

    return this->device->createShaderModuleUnique(vk::ShaderModuleCreateInfo{}
        .setCodeSize(spv.size() * sizeof(uint32_t))
        .setPCode(spv.data()));
}