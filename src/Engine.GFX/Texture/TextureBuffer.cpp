#include "TextureBuffer.hpp"

using namespace ember::engine::gfx;
using namespace ember;

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
    }

    if (_memory) {
        _vkDevice.free(_memory);
    }

    if (_image) {
        _vkDevice.destroyImage(_image);
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
