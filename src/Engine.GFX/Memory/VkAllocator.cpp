#include "VkAllocator.hpp"

#include "AllocatedMemory.hpp"
#include "AllocationResult.hpp"
#include "../vkinc.hpp"
#include "../API/VkTranslate.hpp"
#include "../Device/Device.hpp"

using namespace hg::engine::gfx::memory;
using namespace hg::engine::gfx;
using namespace hg;

VkAllocator::VkAllocator(cref<sptr<Device>> device_) noexcept :
    _device(device_) {}

VkAllocator::~VkAllocator() noexcept = default;

AllocationResult VkAllocator::allocate(
    cref<MemoryLayout> layout_,
    const u64 size_,
    ref<uptr<AllocatedMemory>> dst_
) {

    const vk::MemoryAllocateInfo mai {
        size_,
        get_memory_type(
            _device->vkPhysicalDevice(),
            layout_.typeBits,
            api::vkTranslateMemoryProperties(layout_.props)
        )
    };

    VkDevice vkd { *reinterpret_cast<VkDevice*>(reinterpret_cast<void*>(&_device->vkDevice())) };
    VkDeviceMemory mem {};

    const auto vkMemResult {
        vk::getDispatchLoaderStatic().vkAllocateMemory(
            vkd,
            reinterpret_cast<const VkMemoryAllocateInfo*>(&mai),
            nullptr,
            &mem
        )
    };

    // Handle failed vulkan allocation
    if (vkMemResult != VkResult::VK_SUCCESS) {
        switch (vkMemResult) {
            case VkResult::VK_ERROR_OUT_OF_POOL_MEMORY:
            case VkResult::VK_ERROR_OUT_OF_HOST_MEMORY:
            case VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY: {
                return AllocationResult::eOutOfMemory;
            }
            default: {
                return AllocationResult::eFailed;
            }
        }
    }

    dst_ = make_uptr<AllocatedMemory>(
        this,
        nullptr,
        layout_,
        size_,
        0ui64,
        _device->vkDevice(),
        vk::DeviceMemory { mem }
    );

    return AllocationResult::eSuccess;
}

void VkAllocator::free(mref<uptr<AllocatedMemory>> mem_) {
    _device->vkDevice().freeMemory(mem_->vkMemory);
    mem_->vkMemory = nullptr;
    mem_.reset();
}

cref<sptr<Device>> VkAllocator::device() const noexcept {
    return _device;
}
