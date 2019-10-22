#define VMA_IMPLEMENTATION

#include <renderer/vulkan.hpp>
#include <scene_definitions_for_vulkan/render_plan.hpp>
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

#ifdef VK_ENABLE_VALIDATION_LAYERS
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
#endif

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
}

vulkan_renderer::~vulkan_renderer()
{
    if (unclean)
    {
        this->memory_allocator.destroyBuffer(this->scene_memory.first, this->scene_memory.second);
        this->memory_allocator.destroyBuffer(this->texture_images_memory.first, this->texture_images_memory.second);
        this->memory_allocator.destroyImage(this->output_image_memory.first, this->output_image_memory.second);
    }
    this->memory_allocator.destroy();
}

std::vector<rgba> vulkan_renderer::render_scene(const render_plan& plan)
{
    this->setup_pipeline(plan);
    this->create_descriptor_sets();
    this->create_command_pool();

    const vk::Extent3D image_extent = { plan.image_size.width, plan.image_size.height, 1 };
    std::vector<rgba> image{ image_extent.width * image_extent.height, rgba{ 1 } };

    const vk::DeviceSize scene_buffer_size = plan.world.size();
    const vk::DeviceSize texture_images_buffer_size = 1;

    std::cout << "Allocating memory for scene... ";

    this->scene_memory = this->memory_allocator.createBuffer(
        vk::BufferCreateInfo{}
            .setSize(scene_buffer_size)
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eGpuOnly));

    std::cout << "texture images... ";

    this->texture_images_memory = this->memory_allocator.createBuffer(
        vk::BufferCreateInfo{}
            .setSize(texture_images_buffer_size)
            .setUsage(vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst)
            .setSharingMode(vk::SharingMode::eExclusive),
        vma::AllocationCreateInfo{}
            .setUsage(vma::MemoryUsage::eGpuOnly));

    std::cout << "output image... ";

    this->output_image_memory = this->memory_allocator.createImage(
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
        .setImage(this->output_image_memory.first)
        .setViewType(vk::ImageViewType::e2D)
        .setFormat(vk::Format::eR8G8B8A8Uint)
        .setSubresourceRange(vk::ImageSubresourceRange{}
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1)));

    this->transfer_image(this->output_image_memory.first, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
        vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

    this->unclean = true;

    std::cout << "Done." << std::endl;
    std::cout << "Copying input data... ";

    { // scene
        auto [staging_buffer, staging_allocation] = this->memory_allocator.createBuffer(
            vk::BufferCreateInfo{}
                .setSize(scene_buffer_size)
                .setUsage(vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc)
                .setSharingMode(vk::SharingMode::eExclusive),
            vma::AllocationCreateInfo{}
                .setUsage(vma::MemoryUsage::eCpuOnly));
        this->copy_to_memory(staging_allocation, plan.world.to_bytes().data(), scene_buffer_size);
        this->copy_buffer(staging_buffer, this->scene_memory.first, scene_buffer_size);
        this->memory_allocator.destroyBuffer(staging_buffer, staging_allocation);
    }

    std::cout << "Done." << std::endl;
    std::cout << "Updating descriptor sets... ";

    const auto scene_descriptor_info = vk::DescriptorBufferInfo{}
        .setBuffer(this->scene_memory.first)
        .setRange(VK_WHOLE_SIZE);
    const auto texture_images_descriptor_info = vk::DescriptorBufferInfo{}
        .setBuffer(this->texture_images_memory.first)
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

    this->transfer_image(this->output_image_memory.first, vk::ImageLayout::eGeneral, vk::ImageLayout::eTransferSrcOptimal,
        vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eHost);

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
        retrieve_image.command_buffer.copyImageToBuffer(this->output_image_memory.first, vk::ImageLayout::eTransferSrcOptimal,
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
    this->copy_from_memory(staging_allocation, image.data(), output_image_size);
    this->memory_allocator.destroyBuffer(staging_buffer, staging_allocation);
    
    std::cout << "Done." << std::endl;

    // Cleanup
    this->memory_allocator.destroyBuffer(this->scene_memory.first, this->scene_memory.second);
    this->memory_allocator.destroyBuffer(this->texture_images_memory.first, this->texture_images_memory.second);
    this->memory_allocator.destroyImage(this->output_image_memory.first, this->output_image_memory.second);

    this->unclean = false;

    return image;
}

void vulkan_renderer::create_instance()
{
    using MsgSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
    using MsgType = vk::DebugUtilsMessageTypeFlagBitsEXT;

    std::cout << "Creating Vulkan instance... ";

    const auto application_info = vk::ApplicationInfo{}
        .setApiVersion(VK_MAKE_VERSION(1, 1, 0));
#ifdef VK_ENABLE_VALIDATION_LAYERS
    const auto debug_messenger_info = vk::DebugUtilsMessengerCreateInfoEXT{}
        .setMessageSeverity(MsgSeverity::eVerbose | MsgSeverity::eWarning | MsgSeverity::eError)
        .setMessageType(MsgType::eGeneral | MsgType::eValidation | MsgType::ePerformance)
        .setPfnUserCallback(debug_callback);
#endif
    this->vulkan_instance = vk::createInstanceUnique(vk::InstanceCreateInfo{}
#ifdef VK_ENABLE_VALIDATION_LAYERS
        .setPNext(&debug_messenger_info)
        .setPpEnabledLayerNames(this->validation_layers.data())
        .setEnabledLayerCount(this->validation_layers.size())
        .setEnabledExtensionCount(this->extensions.size())
        .setPpEnabledExtensionNames(this->extensions.data())
#endif
        .setPApplicationInfo(&application_info));
#ifdef VK_ENABLE_VALIDATION_LAYERS
    this->dispatch.init(*this->vulkan_instance);
    this->debug_messenger = this->vulkan_instance->createDebugUtilsMessengerEXTUnique(
        debug_messenger_info, nullptr, this->dispatch);
#endif

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
#ifdef VK_ENABLE_VALIDATION_LAYERS
        .setEnabledLayerCount(this->validation_layers.size())
        .setPpEnabledLayerNames(this->validation_layers.data())
#endif
        .setQueueCreateInfoCount(1)
        .setPQueueCreateInfos(&compute_queue_info));

    this->compute_queue = this->device->getQueue(this->compute_queue_index, 0);

    this->memory_allocator = vma::createAllocator(vma::AllocatorCreateInfo{}
        .setPhysicalDevice(this->physical_device)
        .setDevice(*this->device));

    std::cout << "Done, picked: " << this->physical_device.getProperties().deviceName << std::endl;
}

void vulkan_renderer::setup_pipeline(const render_plan& plan)
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
            .setModule(*this->load_shader_module("shaders/raytracer.comp", plan))
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

vk::UniqueShaderModule vulkan_renderer::load_shader_module(const std::string_view code_path, const render_plan& plan) const
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

    string_replace_all(code, "@SCENE_SHAPES_COUNT@", std::to_string(plan.world.shapes.size()));
    string_replace_all(code, "@SCENE_SPHERE_SHAPES_COUNT@", std::to_string(plan.world.sphere_shapes.size()));
    string_replace_all(code, "@SCENE_DIELECTRIC_MATERIALS_COUNT@", std::to_string(plan.world.dielectric_materials.size()));
    string_replace_all(code, "@SCENE_DIFFUSE_LIGHT_MATERIALS_COUNT@", std::to_string(plan.world.diffuse_light_materials.size()));
    string_replace_all(code, "@SCENE_LAMBERTIAN_MATERIALS_COUNT@", std::to_string(plan.world.lambertian_materials.size()));
    string_replace_all(code, "@SCENE_METAL_MATERIALS_COUNT@", std::to_string(plan.world.metal_materials.size()));
    string_replace_all(code, "@SCENE_CHECKER_TEXTURES_COUNT@", std::to_string(plan.world.checker_textures.size()));
    string_replace_all(code, "@SCENE_CONSTANT_TEXTURES_COUNT@", std::to_string(plan.world.constant_textures.size()));
    string_replace_all(code, "@SCENE_IMAGE_TEXTURES_COUNT@", std::to_string(plan.world.image_textures.size()));
    string_replace_all(code, "@SCENE_NOISE_TEXTURES_COUNT@", std::to_string(plan.world.noise_textures.size()));

    std::cout << "Compiling shader... ";

    shaderc::Compiler compiler;
    const shaderc::SpvCompilationResult compilation_result = compiler.CompileGlslToSpv(
        code, shaderc_shader_kind::shaderc_compute_shader, code_path.data());
    if (compilation_result.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        std::cout << "Error." << std::endl;
        throw std::runtime_error(std::string(compilation_result.GetErrorMessage()));
    }
    std::vector<uint32_t> spv(compilation_result.cbegin(), compilation_result.cend());

    std::cout << "Done." << std::endl;
    std::cout << "Creating shader module... ";

    return this->device->createShaderModuleUnique(vk::ShaderModuleCreateInfo{}
        .setCodeSize(spv.size() * sizeof(uint32_t))
        .setPCode(spv.data()));
}

void vulkan_renderer::copy_to_memory(const vma::Allocation& allocation, const void* data, const size_t size) const
{
    void* memory = this->memory_allocator.mapMemory(allocation);
    std::memcpy(memory, data, size);
    this->memory_allocator.unmapMemory(allocation);
}

void vulkan_renderer::copy_from_memory(const vma::Allocation& allocation, void* data, const size_t size) const
{
    void* memory = this->memory_allocator.mapMemory(allocation);
    std::memcpy(data, memory, size);
    this->memory_allocator.unmapMemory(allocation);
}

void vulkan_renderer::copy_buffer(const vk::Buffer& source, vk::Buffer& destination, const vk::DeviceSize size)
{
    vk_single_time_commands retrieve_image{ *this->device, this->compute_queue, *this->command_pool };
    retrieve_image.command_buffer.copyBuffer(source, destination, vk::BufferCopy{}.setSize(size));
}

void vulkan_renderer::transfer_image(vk::Image& image, const vk::ImageLayout old_layout,
    const vk::ImageLayout new_layout, const vk::PipelineStageFlagBits src_stage,
    const vk::PipelineStageFlagBits dst_stage)
{
    vk_single_time_commands transition_layout{ *this->device, this->compute_queue, *this->command_pool };
    transition_layout.command_buffer.pipelineBarrier(
        src_stage, dst_stage,
        vk::DependencyFlags{}, {}, {}, vk::ImageMemoryBarrier{}
        .setOldLayout(old_layout)
        .setNewLayout(new_layout)
        .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
        .setImage(image)
        .setSubresourceRange(vk::ImageSubresourceRange{}
            .setAspectMask(vk::ImageAspectFlagBits::eColor)
            .setBaseMipLevel(0)
            .setLevelCount(1)
            .setBaseArrayLayer(0)
            .setLayerCount(1)));
}