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
        memory::AllocatedMemory::free(std::move(memory));
        memory = nullptr;
    }
}

void Buffer::flushAligned(const u64 size_, const u64 offset_) {

    constexpr auto shift { 7uLL };
    constexpr auto mask { 0b0111'1111uLL };

    const auto range { (size_ >= VK_WHOLE_SIZE ? size : size_) - offset_ };

    const auto aligned {
        ((range >> shift) << shift) +
        ((range & mask) ? +1uLL << shift : 0uLL)
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

    constexpr auto shift { 7uLL };
    constexpr auto mask { 0b0111'1111uLL };

    const auto range { (size_ >= VK_WHOLE_SIZE ? size : size_) - offset_ };

    const auto aligned {
        ((range >> shift) << shift) +
        ((range & mask) ? +1uLL << shift : 0uLL)
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
