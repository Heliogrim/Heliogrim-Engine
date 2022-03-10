#include "VkAllocator.hpp"

#include "../vkinc.hpp"
#include "../API/VkTranslate.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VkAllocator::VkAllocator(cref<sptr<Device>> device_) noexcept :
    _device(device_),
    _memoryTypeBits(0) {}

VkAllocator::~VkAllocator() noexcept = default;

ptr<AllocatedMemory> VkAllocator::allocate(const u64 size_) {

    const vk::MemoryAllocateInfo mai {
        size_,
        get_memory_type(
            _device->vkPhysicalDevice(),
            _memoryTypeBits,
            _memoryProperties
        )
    };

    const auto vkMemResult { _device->vkDevice().allocateMemory(mai) };
    assert(vkMemResult);

    auto mem = new AllocatedMemory {
        api::vkTranslateMemoryProperties(_memoryProperties),
        _align,
        size_,
        nullptr,
        _device->vkDevice(),
        vkMemResult
    };

    return mem;
}

void VkAllocator::free(mref<ptr<AllocatedMemory>> memory_) {

    _device->vkDevice().freeMemory(memory_->vkMemory);
    memory_->vkMemory = nullptr;

    delete memory_;
    memory_ = nullptr;
}

cref<sptr<Device>> VkAllocator::device() const noexcept {
    return _device;
}

ptr<VkAllocator> VkAllocator::makeForBuffer(cref<sptr<Device>> device_, cref<vk::Buffer> buffer_,
    cref<vk::MemoryPropertyFlags> properties_) {

    const vk::Device vkDevice { device_->vkDevice() };
    const vk::MemoryRequirements req { vkDevice.getBufferMemoryRequirements(buffer_) };

    auto alloc = new VkAllocator { device_ };
    alloc->_align = req.alignment;
    alloc->_memoryTypeBits = req.memoryTypeBits;
    alloc->_memoryProperties = properties_;

    return alloc;
}

ptr<VkAllocator> VkAllocator::makeForImage(cref<sptr<Device>> device_, cref<vk::Image> image_,
    cref<vk::MemoryPropertyFlags> properties_) {

    const vk::Device vkDevice { device_->vkDevice() };
    const vk::MemoryRequirements req { vkDevice.getImageMemoryRequirements(image_) };

    auto alloc = new VkAllocator { device_ };
    alloc->_align = req.alignment;
    alloc->_memoryTypeBits = req.memoryTypeBits;
    alloc->_memoryProperties = properties_;

    return alloc;
}
