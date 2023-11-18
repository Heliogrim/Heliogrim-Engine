#include "Buffer.hpp"

using namespace hg::engine::gfx;
using namespace hg;

void Buffer::bind(const u64 size_, const u64 offset_) {
    device.bindBufferMemory(
        buffer,
        memory->vkMemory,
        memory->offset + offset_
    );
}

void Buffer::destroy() {
    if (buffer) {
        device.destroyBuffer(buffer);
    }

    if (memory) {
        memory::AllocatedMemory::free(_STD move(memory));
        memory = nullptr;
    }
}

void Buffer::flushAligned(const u64 size_, const u64 offset_) {

    constexpr auto shift { 7ui64 };
    constexpr auto mask { 0b0111'1111ui64 };

    const auto range { (size_ >= VK_WHOLE_SIZE ? size : size_) - offset_ };

    const auto aligned {
        ((range >> shift) << shift) +
        ((range & mask) ? +1ui64 << shift : 0ui64)
    };

    memory->flush(
        aligned > memory->size ? VK_WHOLE_SIZE : aligned,
        offset_
    );
}

void Buffer::map(const u64 size_, const u64 offset_) {
    memory->map(
        size_,
        offset_
    );
}

void Buffer::mapAligned(const u64 size_, const u64 offset_) {

    constexpr auto shift { 7ui64 };
    constexpr auto mask { 0b0111'1111ui64 };

    const auto range { (size_ >= VK_WHOLE_SIZE ? size : size_) - offset_ };

    const auto aligned {
        ((range >> shift) << shift) +
        ((range & mask) ? +1ui64 << shift : 0ui64)
    };

    memory->map(
        aligned > memory->size ? VK_WHOLE_SIZE : aligned,
        offset_
    );
}

void Buffer::unmap() noexcept {
    memory->unmap();
}

void Buffer::write(const ptr<void> data_, const u64 size_) {
    memory->write(data_, size_);
}
