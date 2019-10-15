#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include <array>

#define GENERATE_BOILERPLATE(Struct, Binding)\
    using This = Binding;\
    Binding() = default;\
    Binding(Struct const& rhs)\
    {\
        *reinterpret_cast<Struct*>(this) = rhs;\
    }\
    Binding& operator=(Struct const& rhs)\
    {\
        *reinterpret_cast<Struct*>(this) = rhs;\
        return *this;\
    }\
    operator Struct const& ()\
    {\
        return *reinterpret_cast<const Struct*>(this);\
    }\
    operator Struct& ()\
    {\
        return *reinterpret_cast<Struct*>(this);\
    }\
    bool operator!=(Binding const& rhs) const\
    {\
        return !this->operator==(rhs);\
    }

#define PROPERTY(Type, name, setterName)\
    Type name;\
    This& setterName(Type& name)\
    {\
        this->name = name;\
        return *this;\
    }

#define ARRAY_PROPERTY(Type, size, name, setterName)\
    std::array<Type, size> name;\
    This& setterName(const std::array<Type, size>& name)\
    {\
        this->name = name;\
        return *this;\
    }

namespace vma
{
    // ----------------------------------------------------------------------------------------

    using Allocation = VmaAllocation;
    using Allocator = VmaAllocator;
    using Pool = VmaPool;

    // ----------------------------------------------------------------------------------------

    struct DeviceMemoryCallbacks
    {
        GENERATE_BOILERPLATE(VmaDeviceMemoryCallbacks, DeviceMemoryCallbacks)

        DeviceMemoryCallbacks(
            PFN_vmaAllocateDeviceMemoryFunction pfnAllocate,
            PFN_vmaFreeDeviceMemoryFunction pfnFree)
            : pfnAllocate(pfnAllocate)
            , pfnFree(pfnFree)
        {
        }

        PROPERTY(PFN_vmaAllocateDeviceMemoryFunction, pfnAllocate, setPfnAllocate)
        PROPERTY(PFN_vmaFreeDeviceMemoryFunction, pfnFree, setPfnFree)

        bool operator==(DeviceMemoryCallbacks const& rhs) const
        {
            return (this->pfnAllocate == rhs.pfnAllocate)
                && (this->pfnFree == rhs.pfnFree);
        }
    };
    static_assert(sizeof(DeviceMemoryCallbacks) == sizeof(VmaDeviceMemoryCallbacks),
        "DeviceMemoryCallbacks: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    struct VulkanFunctions
    {
        GENERATE_BOILERPLATE(VmaVulkanFunctions, VulkanFunctions)

        VulkanFunctions(
            PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties,
            PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties,
            PFN_vkAllocateMemory vkAllocateMemory,
            PFN_vkFreeMemory vkFreeMemory,
            PFN_vkMapMemory vkMapMemory,
            PFN_vkUnmapMemory vkUnmapMemory,
            PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges,
            PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges,
            PFN_vkBindBufferMemory vkBindBufferMemory,
            PFN_vkBindImageMemory vkBindImageMemory,
            PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements,
            PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements,
            PFN_vkCreateBuffer vkCreateBuffer,
            PFN_vkDestroyBuffer vkDestroyBuffer,
            PFN_vkCreateImage vkCreateImage,
            PFN_vkDestroyImage vkDestroyImage,
            PFN_vkCmdCopyBuffer vkCmdCopyBuffer
#if VMA_DEDICATED_ALLOCATION
            ,PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR,
            PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR
#endif
#if VMA_BIND_MEMORY2
            ,PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR,
            PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR
#endif
        )   : vkGetPhysicalDeviceProperties(vkGetPhysicalDeviceProperties)
            , vkGetPhysicalDeviceMemoryProperties(vkGetPhysicalDeviceMemoryProperties)
            , vkAllocateMemory(vkAllocateMemory)
            , vkFreeMemory(vkFreeMemory)
            , vkMapMemory(vkMapMemory)
            , vkUnmapMemory(vkUnmapMemory)
            , vkFlushMappedMemoryRanges(vkFlushMappedMemoryRanges)
            , vkInvalidateMappedMemoryRanges(vkInvalidateMappedMemoryRanges)
            , vkBindBufferMemory(vkBindBufferMemory)
            , vkBindImageMemory(vkBindImageMemory)
            , vkGetBufferMemoryRequirements(vkGetBufferMemoryRequirements)
            , vkGetImageMemoryRequirements(vkGetImageMemoryRequirements)
            , vkCreateBuffer(vkCreateBuffer)
            , vkDestroyBuffer(vkDestroyBuffer)
            , vkCreateImage(vkCreateImage)
            , vkDestroyImage(vkDestroyImage)
            , vkCmdCopyBuffer(vkCmdCopyBuffer)
#if VMA_DEDICATED_ALLOCATION
            , vkGetBufferMemoryRequirements2KHR(vkGetBufferMemoryRequirements2KHR)
            , vkGetImageMemoryRequirements2KHR(vkGetImageMemoryRequirements2KHR)
#endif
#if VMA_BIND_MEMORY2
            , vkBindBufferMemory2KHR(vkBindBufferMemory2KHR)
            , vkBindImageMemory2KHR(vkBindImageMemory2KHR)
#endif
        {
        }

        PROPERTY(PFN_vkGetPhysicalDeviceProperties, vkGetPhysicalDeviceProperties, setVkGetPhysicalDeviceProperties)
        PROPERTY(PFN_vkGetPhysicalDeviceMemoryProperties, vkGetPhysicalDeviceMemoryProperties, setVkGetPhysicalDeviceMemoryProperties)
        PROPERTY(PFN_vkAllocateMemory, vkAllocateMemory, setVkAllocateMemory)
        PROPERTY(PFN_vkFreeMemory, vkFreeMemory, setVkFreeMemory)
        PROPERTY(PFN_vkMapMemory, vkMapMemory, setVkMapMemory)
        PROPERTY(PFN_vkUnmapMemory, vkUnmapMemory, setVkUnmapMemory)
        PROPERTY(PFN_vkFlushMappedMemoryRanges, vkFlushMappedMemoryRanges, setVkFlushMappedMemoryRanges)
        PROPERTY(PFN_vkInvalidateMappedMemoryRanges, vkInvalidateMappedMemoryRanges, setVkInvalidateMappedMemoryRanges)
        PROPERTY(PFN_vkBindBufferMemory, vkBindBufferMemory, setVkBindBufferMemory)
        PROPERTY(PFN_vkBindImageMemory, vkBindImageMemory, setVkBindImageMemory)
        PROPERTY(PFN_vkGetBufferMemoryRequirements, vkGetBufferMemoryRequirements, setVkGetBufferMemoryRequirements)
        PROPERTY(PFN_vkGetImageMemoryRequirements, vkGetImageMemoryRequirements, setVkGetImageMemoryRequirements)
        PROPERTY(PFN_vkCreateBuffer, vkCreateBuffer, setVkCreateBuffer)
        PROPERTY(PFN_vkDestroyBuffer, vkDestroyBuffer, setVkDestroyBuffer)
        PROPERTY(PFN_vkCreateImage, vkCreateImage, setVkCreateImage)
        PROPERTY(PFN_vkDestroyImage, vkDestroyImage, setVkDestroyImage)
        PROPERTY(PFN_vkCmdCopyBuffer, vkCmdCopyBuffer, setVkCmdCopyBuffer)
 #if VMA_DEDICATED_ALLOCATION
        PROPERTY(PFN_vkGetBufferMemoryRequirements2KHR, vkGetBufferMemoryRequirements2KHR, setVkGetBufferMemoryRequirements2KHR)
        PROPERTY(PFN_vkGetImageMemoryRequirements2KHR, vkGetImageMemoryRequirements2KHR, setVkGetImageMemoryRequirements2KHR)
 #endif
 #if VMA_BIND_MEMORY2
        PROPERTY(PFN_vkBindBufferMemory2KHR, vkBindBufferMemory2KHR, setVkBindBufferMemory2KHR)
        PROPERTY(PFN_vkBindImageMemory2KHR, vkBindImageMemory2KHR, setVkBindImageMemory2KHR)
 #endif

        bool operator==(VulkanFunctions const& rhs) const
        {
            return (this->vkGetPhysicalDeviceProperties == rhs.vkGetPhysicalDeviceProperties)
                && (this->vkGetPhysicalDeviceMemoryProperties == rhs.vkGetPhysicalDeviceMemoryProperties)
                && (this->vkAllocateMemory == rhs.vkAllocateMemory)
                && (this->vkFreeMemory == rhs.vkFreeMemory)
                && (this->vkMapMemory == rhs.vkMapMemory)
                && (this->vkUnmapMemory == rhs.vkUnmapMemory)
                && (this->vkFlushMappedMemoryRanges == rhs.vkFlushMappedMemoryRanges)
                && (this->vkInvalidateMappedMemoryRanges == rhs.vkInvalidateMappedMemoryRanges)
                && (this->vkBindBufferMemory == rhs.vkBindBufferMemory)
                && (this->vkBindImageMemory == rhs.vkBindImageMemory)
                && (this->vkGetBufferMemoryRequirements == rhs.vkGetBufferMemoryRequirements)
                && (this->vkGetImageMemoryRequirements == rhs.vkGetImageMemoryRequirements)
                && (this->vkCreateBuffer == rhs.vkCreateBuffer)
                && (this->vkDestroyBuffer == rhs.vkDestroyBuffer)
                && (this->vkCreateImage == rhs.vkCreateImage)
                && (this->vkDestroyImage == rhs.vkDestroyImage)
                && (this->vkCmdCopyBuffer == rhs.vkCmdCopyBuffer)
#if VMA_DEDICATED_ALLOCATION
                && (this->vkGetBufferMemoryRequirements2KHR == rhs.vkGetBufferMemoryRequirements2KHR)
                && (this->vkGetImageMemoryRequirements2KHR == rhs.vkGetImageMemoryRequirements2KHR)
#endif
#if VMA_BIND_MEMORY2
                && (this->vkBindBufferMemory2KHR == rhs.vkBindBufferMemory2KHR)
                && (this->vkBindImageMemory2KHR == rhs.vkBindImageMemory2KHR)
#endif
                ;
        }
    };
    static_assert(sizeof(VulkanFunctions) == sizeof(VmaVulkanFunctions), "VulkanFunctions: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    enum class RecordFlagBits
    {
        eFlushAfterCall = VMA_RECORD_FLUSH_AFTER_CALL_BIT,
        eMax = VMA_RECORD_FLAG_BITS_MAX_ENUM
    };

    using RecordFlags = vk::Flags<RecordFlagBits>;

    struct RecordSettings
    {
        GENERATE_BOILERPLATE(VmaRecordSettings, RecordSettings)

        RecordSettings(
            RecordFlags flags,
            const char* pFilePath)
            : flags(flags)
            , pFilePath(pFilePath)
        {
        }

        PROPERTY(RecordFlags, flags, setFlags)
        PROPERTY(const char*, pFilePath, setPFilePath)

        bool operator==(RecordSettings const& rhs) const
        {
            return (this->flags == rhs.flags)
                && (this->pFilePath == rhs.pFilePath);
        }
    };
    static_assert(sizeof(RecordSettings) == sizeof(VmaRecordSettings),
        "RecordSettings: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    enum class AllocatorCreateFlagBits
    {
        eCreateExternallySynchronized = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
        eCreateKHRDedicatedAllocation = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT,
        eCreateKHRBindMemory2 = 0x00000004 /* VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT */,
        eMax = VMA_ALLOCATOR_CREATE_FLAG_BITS_MAX_ENUM
    };

    using AllocatorCreateFlags = vk::Flags<AllocatorCreateFlagBits>;

    struct AllocatorCreateInfo
    {
        GENERATE_BOILERPLATE(VmaAllocatorCreateInfo, AllocatorCreateInfo)

        AllocatorCreateInfo(
            AllocatorCreateFlags flags,
            vk::PhysicalDevice physicalDevice,
            vk::Device device,
            vk::DeviceSize preferredLargeHeapBlockSize,
            const vk::AllocationCallbacks* pAllocationCallbacks,
            const DeviceMemoryCallbacks* pDeviceMemoryCallbacks,
            uint32_t frameInUseCount,
            const vk::DeviceSize* pHeapSizeLimit,
            const VulkanFunctions* pVulkanFunctions,
            const RecordSettings* pRecordSettings)
            : flags(flags)
            , physicalDevice(physicalDevice)
            , device(device)
            , preferredLargeHeapBlockSize(preferredLargeHeapBlockSize)
            , pAllocationCallbacks(pAllocationCallbacks)
            , pDeviceMemoryCallbacks(pDeviceMemoryCallbacks)
            , frameInUseCount(frameInUseCount)
            , pHeapSizeLimit(pHeapSizeLimit)
            , pVulkanFunctions(pVulkanFunctions)
            , pRecordSettings(pRecordSettings)
        {
        }

        PROPERTY(AllocatorCreateFlags, flags, setFlags)
        PROPERTY(vk::PhysicalDevice, physicalDevice, setPhysicalDevice)
        PROPERTY(vk::Device, device, setDevices)
        PROPERTY(vk::DeviceSize, preferredLargeHeapBlockSize, setPreferredLargeHeapBlockSize)
        PROPERTY(const vk::AllocationCallbacks*, pAllocationCallbacks, setPAllocationCallbacks)
        PROPERTY(const DeviceMemoryCallbacks*, pDeviceMemoryCallbacks, setPDeviceMemoryCallbacks)
        PROPERTY(uint32_t, frameInUseCount, setFrameInUseCount)
        PROPERTY(const vk::DeviceSize*, pHeapSizeLimit, setPHeapSizeLimit)
        PROPERTY(const VulkanFunctions*, pVulkanFunctions, setPVulkanFunctions)
        PROPERTY(const RecordSettings*, pRecordSettings, setPRecordSettings)

        bool operator==(AllocatorCreateInfo const& rhs) const
        {
            return (this->flags == rhs.flags)
                && (this->physicalDevice == rhs.physicalDevice)
                && (this->device == rhs.device)
                && (this->preferredLargeHeapBlockSize == rhs.preferredLargeHeapBlockSize)
                && (this->pAllocationCallbacks == rhs.pAllocationCallbacks)
                && (this->pDeviceMemoryCallbacks == rhs.pDeviceMemoryCallbacks)
                && (this->frameInUseCount == rhs.frameInUseCount)
                && (this->pHeapSizeLimit == rhs.pHeapSizeLimit)
                && (this->pVulkanFunctions == rhs.pVulkanFunctions)
                && (this->pRecordSettings == rhs.pRecordSettings);
        }
    };
    static_assert(sizeof(AllocatorCreateInfo) == sizeof(VmaAllocatorCreateInfo),
        "AllocatorCreateInfo: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    struct StatInfo
    {
        GENERATE_BOILERPLATE(VmaStatInfo, StatInfo)

        StatInfo(
            uint32_t blockCount,
            uint32_t allocationCount,
            uint32_t unusedRangeCount,
            vk::DeviceSize usedBytes,
            vk::DeviceSize unusedBytes,
            vk::DeviceSize allocationSizeMin,
            vk::DeviceSize allocationSizeAvg,
            vk::DeviceSize allocationSizeMax,
            vk::DeviceSize unusedRangeSizeMin,
            vk::DeviceSize unusedRangeSizeAvg,
            vk::DeviceSize unusedRangeSizeMax)
            : blockCount(blockCount)
            , allocationCount(allocationCount)
            , unusedRangeCount(unusedRangeCount)
            , usedBytes(usedBytes)
            , unusedBytes(unusedBytes)
            , allocationSizeMin(allocationSizeMin)
            , allocationSizeAvg(allocationSizeAvg)
            , allocationSizeMax(allocationSizeMax)
            , unusedRangeSizeMin(unusedRangeSizeMin)
            , unusedRangeSizeAvg(unusedRangeSizeAvg)
            , unusedRangeSizeMax(unusedRangeSizeMax)
        {
        }

        PROPERTY(uint32_t, blockCount, setBlockCount)
        PROPERTY(uint32_t, allocationCount, setAllocationCount)
        PROPERTY(uint32_t, unusedRangeCount, setUnusedRangeCount)
        PROPERTY(vk::DeviceSize, usedBytes, setUsedBytes)
        PROPERTY(vk::DeviceSize, unusedBytes, setUnusedBytes)
        PROPERTY(vk::DeviceSize, allocationSizeMin, setAallocationSizeMin)
        PROPERTY(vk::DeviceSize, allocationSizeAvg, setAllocationSizeAvg)
        PROPERTY(vk::DeviceSize, allocationSizeMax, setAllocationSizeMax)
        PROPERTY(vk::DeviceSize, unusedRangeSizeMin, setUnusedRangeSizeMin)
        PROPERTY(vk::DeviceSize, unusedRangeSizeAvg, setUnusedRangeSizeAvg)
        PROPERTY(vk::DeviceSize, unusedRangeSizeMax, setUnusedRangeSizeMax)

        bool operator==(StatInfo const& rhs) const
        {
            return (this->blockCount == rhs.blockCount)
                && (this->allocationCount == rhs.allocationCount)
                && (this->unusedRangeCount == rhs.unusedRangeCount)
                && (this->usedBytes == rhs.usedBytes)
                && (this->unusedBytes == rhs.unusedBytes)
                && (this->allocationSizeMin == rhs.allocationSizeMin)
                && (this->allocationSizeAvg == rhs.allocationSizeAvg)
                && (this->allocationSizeMax == rhs.allocationSizeMax)
                && (this->unusedRangeSizeMin == rhs.unusedRangeSizeMin)
                && (this->unusedRangeSizeAvg == rhs.unusedRangeSizeAvg)
                && (this->unusedRangeSizeMax == rhs.unusedRangeSizeMax);
        }
    };
    static_assert(sizeof(StatInfo) == sizeof(VmaStatInfo), "StatInfo: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    struct Stats
    {
        GENERATE_BOILERPLATE(VmaStats, Stats)

        Stats(
            const std::array<StatInfo, VK_MAX_MEMORY_TYPES>& memoryType,
            const std::array<StatInfo, VK_MAX_MEMORY_HEAPS>& memoryHeap,
            const StatInfo& total)
            : memoryType(memoryType)
            , memoryHeap(memoryHeap)
            , total(total)
        {
        }
        
        ARRAY_PROPERTY(StatInfo, VK_MAX_MEMORY_TYPES, memoryType, setPMemoryType)
        ARRAY_PROPERTY(StatInfo, VK_MAX_MEMORY_HEAPS, memoryHeap, setPMemoryHeap)
        PROPERTY(StatInfo, total, setTotal)

        bool operator==(Stats const& rhs) const
        {
            return (this->memoryType == rhs.memoryType)
                && (this->memoryHeap == rhs.memoryHeap)
                && (this->total == rhs.total);
        }
    };
    static_assert(sizeof(Stats) == sizeof(VmaStats), "Stats: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    enum class AllocationCreateFlagBits
    {
        eDediactedMemory = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        eNeverAllocate = VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT,
        eMapped = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        eCanBecomeLost = VMA_ALLOCATION_CREATE_CAN_BECOME_LOST_BIT,
        eCanMakeOtherLost = VMA_ALLOCATION_CREATE_CAN_MAKE_OTHER_LOST_BIT,
        eUserDataCopyString = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT,
        eUpperAddress = VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT,
        eDontBind = 0x00000080 /* VMA_ALLOCATION_CREATE_DONT_BIND_BIT */,
        eStrategyBestFit = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT,
        eStrategyWorstFit = VMA_ALLOCATION_CREATE_STRATEGY_WORST_FIT_BIT,
        eStrategyFirstFit = VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT,
        eStrategyMinMemory = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
        eStrategyMinTime = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
        eStrategyMinFragmentation = VMA_ALLOCATION_CREATE_STRATEGY_MIN_FRAGMENTATION_BIT,
        eStrategyMask = VMA_ALLOCATION_CREATE_STRATEGY_MASK,
        eMax = VMA_ALLOCATION_CREATE_FLAG_BITS_MAX_ENUM
    };

    using AllocationCreateFlags = vk::Flags<AllocationCreateFlagBits>;

    enum class MemoryUsage
    {
        eUnknown = VMA_MEMORY_USAGE_UNKNOWN,
        eGpuOnly = VMA_MEMORY_USAGE_GPU_ONLY,
        eCpuOnly = VMA_MEMORY_USAGE_CPU_ONLY,
        eCpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
        eGpuToCpu = VMA_MEMORY_USAGE_GPU_TO_CPU,
        eMax = VMA_MEMORY_USAGE_MAX_ENUM
    };

    struct AllocationCreateInfo
    {
        GENERATE_BOILERPLATE(VmaAllocationCreateInfo, AllocationCreateInfo)

        AllocationCreateInfo(
            AllocationCreateFlags flags,
            MemoryUsage usage,
            vk::MemoryPropertyFlags requiredFlags,
            vk::MemoryPropertyFlags preferredFlags,
            uint32_t memoryTypeBits,
            Pool pool,
            void* pUserData)
            : flags(flags)
            , usage(usage)
            , requiredFlags(requiredFlags)
            , preferredFlags(preferredFlags)
            , memoryTypeBits(memoryTypeBits)
            , pool(pool)
            , pUserData(pUserData)
        {
        }

        PROPERTY(AllocationCreateFlags, flags, setFlags)
        PROPERTY(MemoryUsage, usage, setUsage)
        PROPERTY(vk::MemoryPropertyFlags, requiredFlags, setRequiredFlags)
        PROPERTY(vk::MemoryPropertyFlags, preferredFlags, setPreferredFlags)
        PROPERTY(uint32_t, memoryTypeBits, setMemoryTypeBits)
        PROPERTY(Pool, pool, setPool)
        PROPERTY(void*, pUserData, setPUserData)

        bool operator==(AllocationCreateInfo const& rhs) const
        {
            return (this->flags == rhs.flags)
                && (this->usage == rhs.usage)
                && (this->requiredFlags == rhs.requiredFlags)
                && (this->preferredFlags == rhs.preferredFlags)
                && (this->memoryTypeBits == rhs.memoryTypeBits)
                && (this->pool == rhs.pool)
                && (this->pUserData == rhs.pUserData);
        }
    };
    static_assert(sizeof(AllocationCreateInfo) == sizeof(VmaAllocationCreateInfo),
        "AllocationCreateInfo: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    enum class PoolCreateFlagBits
    {
        eIgnoreBufferImageGranularity = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT,
        eLinearAlgorithm = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT,
        eBuddyAlgorithm = VMA_POOL_CREATE_BUDDY_ALGORITHM_BIT,
        eAlgorithmMask = VMA_POOL_CREATE_ALGORITHM_MASK,
        eMax = VMA_POOL_CREATE_FLAG_BITS_MAX_ENUM
    };

    using PoolCreateFlags = vk::Flags<PoolCreateFlagBits>;

    struct PoolCreateInfo
    {
        GENERATE_BOILERPLATE(VmaPoolCreateInfo, PoolCreateInfo)

        PoolCreateInfo(
            uint32_t memoryTypeIndex,
            PoolCreateFlags flags,
            vk::DeviceSize blockSize,
            size_t minBlockCount,
            size_t maxBlockCount,
            uint32_t frameInUseCount)
            : memoryTypeIndex(memoryTypeIndex)
            , flags(flags)
            , blockSize(blockSize)
            , minBlockCount(minBlockCount)
            , maxBlockCount(maxBlockCount)
            , frameInUseCount(frameInUseCount)
        {
        }

        PROPERTY(uint32_t, memoryTypeIndex, setMemoryTypeIndex)
        PROPERTY(PoolCreateFlags, flags, setFlags)
        PROPERTY(vk::DeviceSize, blockSize, setBlockSize)
        PROPERTY(size_t, minBlockCount, setMinBlockCount)
        PROPERTY(size_t, maxBlockCount, setMaxBlockCount)
        PROPERTY(uint32_t, frameInUseCount, setFrameInUseCount)

        bool operator==(PoolCreateInfo const& rhs) const
        {
            return (this->memoryTypeIndex == rhs.memoryTypeIndex)
                && (this->flags == rhs.flags)
                && (this->blockSize == rhs.blockSize)
                && (this->minBlockCount == rhs.minBlockCount)
                && (this->maxBlockCount == rhs.maxBlockCount)
                && (this->frameInUseCount == rhs.frameInUseCount);
        }
    };
    static_assert(sizeof(PoolCreateInfo) == sizeof(VmaPoolCreateInfo),
        "PoolCreateInfo: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    struct PoolStats
    {
        GENERATE_BOILERPLATE(VmaPoolStats, PoolStats)

        PoolStats(
            vk::DeviceSize size,
            vk::DeviceSize unusedSize,
            size_t allocationCount,
            size_t unusedRangeCount,
            vk::DeviceSize unusedRangeSizeMax,
            size_t blockCount)
            : size(size)
            , unusedSize(unusedSize)
            , allocationCount(allocationCount)
            , unusedRangeCount(unusedRangeCount)
            , unusedRangeSizeMax(unusedRangeSizeMax)
            , blockCount(blockCount)
        {
        }

        PROPERTY(vk::DeviceSize, size, setSize)
        PROPERTY(vk::DeviceSize, unusedSize, setUnusedSize)
        PROPERTY(size_t, allocationCount, setAllocationCount)
        PROPERTY(size_t, unusedRangeCount, setUnusedRangeCount)
        PROPERTY(vk::DeviceSize, unusedRangeSizeMax, setUunusedRangeSizeMax)
        PROPERTY(size_t, blockCount, setBlockCount)

        bool operator==(PoolStats const& rhs) const
        {
            return (this->size == rhs.size)
                && (this->unusedSize == rhs.unusedSize)
                && (this->allocationCount == rhs.allocationCount)
                && (this->unusedRangeCount == rhs.unusedRangeCount)
                && (this->unusedRangeSizeMax == rhs.unusedRangeSizeMax)
                && (this->blockCount == rhs.blockCount);
        }
    };
    static_assert(sizeof(PoolStats) == sizeof(VmaPoolStats), "PoolStats: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    struct AllocationInfo
    {
        GENERATE_BOILERPLATE(VmaAllocationInfo, AllocationInfo)

        AllocationInfo(
            uint32_t memoryType,
            vk::DeviceMemory deviceMemory,
            vk::DeviceSize offset,
            vk::DeviceSize size,
            void* pMappedData,
            void* pUserData)
            : memoryType(memoryType)
            , deviceMemory(deviceMemory)
            , offset(offset)
            , size(size)
            , pMappedData(pMappedData)
            , pUserData(pUserData)
        {
        }

        PROPERTY(uint32_t, memoryType, setMemoryType)
        PROPERTY(vk::DeviceMemory, deviceMemory, setDeviceMemory)
        PROPERTY(vk::DeviceSize, offset, setOffset)
        PROPERTY(vk::DeviceSize, size, setSize)
        PROPERTY(void*, pMappedData, setPMappedData)
        PROPERTY(void*, pUserData, setPUserData)

        bool operator==(AllocationInfo const& rhs) const
        {
            return (this->memoryType == rhs.memoryType)
                && (this->deviceMemory == rhs.deviceMemory)
                && (this->offset == rhs.offset)
                && (this->size == rhs.size)
                && (this->pMappedData == rhs.pMappedData)
                && (this->pUserData == rhs.pUserData);
        }
    };
    static_assert(sizeof(AllocationInfo) == sizeof(VmaAllocationInfo),
        "AllocationInfo: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    enum class DefragmentationFlagBits
    {
        eMax = VMA_DEFRAGMENTATION_FLAG_BITS_MAX_ENUM
    };

    using DefragmentationFlags = vk::Flags<DefragmentationFlagBits>;

    struct DefragmentationInfo2
    {
        GENERATE_BOILERPLATE(VmaDefragmentationInfo2, DefragmentationInfo2)

        DefragmentationInfo2(
            DefragmentationFlags flags,
            uint32_t allocationCount,
            Allocation* pAllocations,
            vk::Bool32* pAllocationsChanged,
            uint32_t poolCount,
            Pool* pPools,
            vk::DeviceSize maxCpuBytesToMove,
            uint32_t maxCpuAllocationsToMove,
            vk::DeviceSize maxGpuBytesToMove,
            uint32_t maxGpuAllocationsToMove,
            vk::CommandBuffer commandBuffer)
            : flags(flags)
            , allocationCount(allocationCount)
            , pAllocations(pAllocations)
            , pAllocationsChanged(pAllocationsChanged)
            , poolCount(poolCount)
            , pPools(pPools)
            , maxCpuBytesToMove(maxCpuBytesToMove)
            , maxCpuAllocationsToMove(maxCpuAllocationsToMove)
            , maxGpuBytesToMove(maxGpuBytesToMove)
            , maxGpuAllocationsToMove(maxGpuAllocationsToMove)
            , commandBuffer(commandBuffer)
        {
        }

        PROPERTY(DefragmentationFlags, flags, setFlags)
        PROPERTY(uint32_t, allocationCount, setAllocationCount)
        PROPERTY(Allocation*, pAllocations, setPAllocations)
        PROPERTY(vk::Bool32*, pAllocationsChanged, setPAllocationsChanged)
        PROPERTY(uint32_t, poolCount, setPoolCount)
        PROPERTY(Pool*, pPools, setPPools)
        PROPERTY(vk::DeviceSize, maxCpuBytesToMove, setMaxCpuBytesToMove)
        PROPERTY(uint32_t, maxCpuAllocationsToMove, setMaxCpuAllocationsToMove)
        PROPERTY(vk::DeviceSize, maxGpuBytesToMove, setMaxGpuBytesToMove)
        PROPERTY(uint32_t, maxGpuAllocationsToMove, setMaxGpuAllocationsToMove)
        PROPERTY(vk::CommandBuffer, commandBuffer, setCommandBuffer)

        bool operator==(DefragmentationInfo2 const& rhs) const
        {
            return (this->flags == rhs.flags)
                && (this->allocationCount == rhs.allocationCount)
                && (this->pAllocations == rhs.pAllocations)
                && (this->pAllocationsChanged == rhs.pAllocationsChanged)
                && (this->poolCount == rhs.poolCount)
                && (this->pPools == rhs.pPools)
                && (this->maxCpuBytesToMove == rhs.maxCpuBytesToMove)
                && (this->maxCpuAllocationsToMove == rhs.maxCpuAllocationsToMove)
                && (this->maxGpuBytesToMove == rhs.maxGpuBytesToMove)
                && (this->maxGpuAllocationsToMove == rhs.maxGpuAllocationsToMove)
                && (this->commandBuffer == rhs.commandBuffer);
        }
    };
    static_assert(sizeof(DefragmentationInfo2) == sizeof(VmaDefragmentationInfo2),
        "DefragmentationInfo2: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    struct DefragmentationInfo
    {
        GENERATE_BOILERPLATE(VmaDefragmentationInfo, DefragmentationInfo)

        DefragmentationInfo(
            vk::DeviceSize maxBytesToMove,
            uint32_t maxAllocationsToMove)
            : maxBytesToMove(maxBytesToMove)
            , maxAllocationsToMove(maxAllocationsToMove)
        {
        }

        PROPERTY(vk::DeviceSize, maxBytesToMove, setMaxBytesToMove)
        PROPERTY(uint32_t, maxAllocationsToMove, setMaxAllocationsToMove)

        bool operator==(DefragmentationInfo const& rhs) const
        {
            return (this->maxBytesToMove == rhs.maxBytesToMove)
                && (this->maxAllocationsToMove == rhs.maxAllocationsToMove);
        }
    };
    static_assert(sizeof(DefragmentationInfo) == sizeof(VmaDefragmentationInfo),
        "DefragmentationInfo: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    struct DefragmentationStats
    {
        GENERATE_BOILERPLATE(VmaDefragmentationStats, DefragmentationStats)

        DefragmentationStats(
            vk::DeviceSize bytesMoved,
            vk::DeviceSize bytesFreed,
            uint32_t allocationsMoved,
            uint32_t deviceMemoryBlocksFreed)
            : bytesMoved(bytesMoved)
            , bytesFreed(bytesFreed)
            , allocationsMoved(allocationsMoved)
            , deviceMemoryBlocksFreed(deviceMemoryBlocksFreed)
        {
        }

        PROPERTY(vk::DeviceSize, bytesMoved, setBytesMoved)
        PROPERTY(vk::DeviceSize, bytesFreed, setBytesFreed)
        PROPERTY(uint32_t, allocationsMoved, setAllocationsMoved)
        PROPERTY(uint32_t, deviceMemoryBlocksFreed, setDeviceMemoryBlocksFreed)

        bool operator==(DefragmentationStats const& rhs) const
        {
            return (this->bytesMoved == rhs.bytesMoved)
                && (this->bytesFreed == rhs.bytesFreed)
                && (this->allocationsMoved == rhs.allocationsMoved)
                && (this->deviceMemoryBlocksFreed == rhs.deviceMemoryBlocksFreed);
        }
    };
    static_assert(sizeof(DefragmentationStats) == sizeof(VmaDefragmentationStats),
        "DefragmentationStats: struct and wrapper have different size!");

    // ----------------------------------------------------------------------------------------

    inline static vk::ResultValueType<Allocator>::type createAllocator(const AllocatorCreateInfo& createInfo)
    {
        Allocator allocator;
        vk::Result result = static_cast<vk::Result>(vmaCreateAllocator(reinterpret_cast<const VmaAllocatorCreateInfo*>(&createInfo), &allocator));
        return vk::createResultValue(result, allocator, "vma::createAllocator");
    }
}

#undef GENERATE_BOILERPLATE
#undef PROPERTY
#undef ARRAY_PROPERTY