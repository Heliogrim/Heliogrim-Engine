#include "VkBufferFactory.hpp"

#include "Buffer.hpp"

using namespace hg::engine::gfx;
using namespace hg;

nmpt<VkBufferFactory> VkBufferFactory::make(cref<sptr<Device>> device_) {
    _instance = uptr<VkBufferFactory>(new VkBufferFactory(device_));
    return static_cast<ptr<VkBufferFactory>>(_instance.get());
}

VkBufferFactory::VkBufferFactory(cref<sptr<Device>> device_) :
    BufferFactory(device_) {}

VkBufferFactory::~VkBufferFactory() noexcept = default;

Buffer VkBufferFactory::build(mref<BufferBuildPayload> payload_) const {

    Buffer obj {};

    /**/

    vk::BufferCreateInfo bci {
        payload_.vkFlags,
        payload_.byteSize,
        payload_.vkUsage,
        payload_.vkSharing,
        0uL,
        nullptr
    };

    obj.buffer = _device->vkDevice().createBuffer(bci);
    obj.device = _device->vkDevice();
    assert(obj.buffer);

    /**/

    const auto result = allocate(_device->allocator(), _device, payload_.memoryProperties, obj);
    assert(obj.memory);

    obj.bind();

    /**/

    obj.size = payload_.byteSize;
    obj.usageFlags = payload_.vkUsage;

    return obj;
}
