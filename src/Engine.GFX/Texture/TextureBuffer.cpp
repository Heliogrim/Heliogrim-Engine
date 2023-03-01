#include "TextureBuffer.hpp"

using namespace hg::engine::gfx;
using namespace hg;

TextureBuffer::TextureBuffer() :
    _image(nullptr),
    _memory(nullptr),
    _memorySize(0),
    _memoryAlign(0),
    _memoryMapped(nullptr),
    _sharing(),
    _tiling(),
    _vkLayout() {}

TextureBuffer::~TextureBuffer() noexcept {}

TextureBuffer::operator bool() const noexcept {
    return _image && _memory;
}

void TextureBuffer::destroy() {
    if (_memory && _memoryMapped) {
        _vkDevice.unmapMemory(_memory);
        _memoryMapped = nullptr;
    }

    if (_memory) {
        _vkDevice.free(_memory);
        _memory = VK_NULL_HANDLE;
    }

    if (_image) {
        _vkDevice.destroyImage(_image);
        _image = VK_NULL_HANDLE;
    }
}

vk::Device& TextureBuffer::vkDevice() noexcept {
    return _vkDevice;
}

vk::Image& TextureBuffer::image() noexcept {
    return _image;
}

vk::Image TextureBuffer::image() const noexcept {
    return _image;
}

vk::DeviceMemory& TextureBuffer::memory() noexcept {
    return _memory;
}

vk::DeviceMemory TextureBuffer::memory() const noexcept {
    return _memory;
}

u64& TextureBuffer::memorySize() noexcept {
    return _memorySize;
}

u64 TextureBuffer::memorySize() const noexcept {
    return _memorySize;
}

u64& TextureBuffer::memoryAlign() noexcept {
    return _memoryAlign;
}

u64 TextureBuffer::memoryAlign() const noexcept {
    return _memoryAlign;
}

void* TextureBuffer::memoryMapped() const noexcept {
    return _memoryMapped;
}

void*& TextureBuffer::memoryMapped() noexcept {
    return _memoryMapped;
}

vk::SharingMode& TextureBuffer::sharingMode() noexcept {
    return _sharing;
}

vk::SharingMode TextureBuffer::sharingMode() const noexcept {
    return _sharing;
}

vk::ImageTiling& TextureBuffer::tiling() noexcept {
    return _tiling;
}

vk::ImageTiling TextureBuffer::tiling() const noexcept {
    return _tiling;
}
