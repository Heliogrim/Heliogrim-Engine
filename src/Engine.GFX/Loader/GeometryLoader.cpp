#include "GeometryLoader.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

GeometryLoader::GeometryLoader(cref<sptr<Device>> device_) :
    _device(device_) {}

GeometryLoader::future_type GeometryLoader::load(const Url& url_) const {
    const future_type::state_type state = nullptr;
    return future_type { state };
}

#if 0
Buffer StaticGeometryLoader::acquireDeviceBuffer(const u64 size_, const u64 stride_) {

    /**
     *
     */
    const vk::BufferCreateInfo bci {
        vk::BufferCreateFlags {},
        size_,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::SharingMode::eExclusive,
        NULL,
        NULL
    };

    auto rvkb { _device->vkDevice().createBuffer(bci) };
    auto bmr = _device->vkDevice().getBufferMemoryRequirements(rvkb);
    vk::MemoryAllocateInfo mai {
        bmr.size,
        get_memory_type(_device->vkPhysicalDevice(), bmr.memoryTypeBits,
            vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible)
    };

    const auto mem { _device->vkDevice().allocateMemory(mai) };

    /**
     * Bind Buffer and Memory
     */
    _device->vkDevice().bindBufferMemory(rvkb, mem, 0);

    /**
     *
     */
    Buffer buffer {
        rvkb,
        _device->vkDevice(),
        mem,
        bmr.size,
        bmr.alignment,
        nullptr,
        vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
        vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible
    };

    return buffer;
}
#endif
