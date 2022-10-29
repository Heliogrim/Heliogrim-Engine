#include "Texture.hpp"

#include "TextureView.hpp"

using namespace ember::engine::gfx;

Texture::Texture() noexcept :
    _buffer(),
    _extent(0ui32),
    _format(TextureFormat::eUndefined),
    _mipLevels(0),
    _type(TextureType::eUndefined),
    _view(vk::ImageView {}) {}

Texture::Texture(value_type&& other_) noexcept :
    _buffer(_STD exchange(other_._buffer, {})),
    _extent(_STD exchange(other_._extent, math::uivec3 { 0ui32 })),
    _format(_STD exchange(other_._format, TextureFormat::eUndefined)),
    _mipLevels(_STD exchange(other_._mipLevels, 0)),
    _type(_STD exchange(other_._type, TextureType::eUndefined)),
    _view(_STD exchange(other_._view, {})) { }

Texture::~Texture() noexcept {
    destroy();
}

Texture& Texture::operator=(value_type&& other_) noexcept {
    if (this != &other_) {
        _buffer = _STD exchange(other_._buffer, {});
        _extent = _STD exchange(other_._extent, math::uivec3 { 0ui32 });
        _format = _STD exchange(other_._format, TextureFormat::eUndefined);
        _mipLevels = _STD exchange(other_._mipLevels, 0);
        _type = _STD exchange(other_._type, TextureType::eUndefined);
        _view = _STD exchange(other_._view, {});
    }

    return *this;
}

Texture::operator bool() const noexcept {
    return _buffer && /* Should be at least one */_mipLevels;
}

void Texture::destroy() {

    // We expect vkDevice to be defined, otherwise it might be a swapchain image -> texture
    // TODO: make identifier for swapchain images
    if (!_buffer.vkDevice()) {
        return;
    }

    if (_view) {
        _buffer.vkDevice().destroy(_view);
        _view = VK_NULL_HANDLE;
    }

    _buffer.destroy();
}

ember::uptr<TextureView> Texture::makeView(math::uivec2 layers_, math::uExtent3D extent_, math::uivec2 mipLevels_) {

    const math::uivec2 mipLevels {
        MAX(_mipLevels, mipLevels_.min),
        MIN(_mipLevels, mipLevels_.max)
    };

    const math::uivec2 layers {
        MAX(_extent.z, layers_.min),
        MIN(_extent.z, layers_.max)
    };

    const math::uExtent3D extent {
        MIN(_extent.x - extent_.offsetX, extent_.width),
        MIN(_extent.y - extent_.offsetY, extent_.height),
        MIN(/*_extent.z*/1 - extent_.offsetZ, extent_.depth),
        MIN(_extent.x, extent_.offsetX),
        MIN(_extent.y, extent_.offsetY),
        MIN(/*_extent.z*/1, extent_.offsetZ)
    };

    /**/

    const auto view {
        new TextureView(this, layers, extent, mipLevels)
    };
    return _STD unique_ptr<TextureView>(view);
}

TextureBuffer& Texture::buffer() noexcept {
    return _buffer;
}

const TextureBuffer& Texture::buffer() const noexcept {
    return _buffer;
}

ember::math::uivec3& Texture::extent() noexcept {
    return _extent;
}

ember::math::uivec3 Texture::extent() const noexcept {
    return _extent;
}

ember::math::uivec3::value_type Texture::depth() const noexcept {
    if (_type != TextureType::e2dArray) {
        return _extent.z;
    }

    return 1ui32;
}

ember::math::uivec3::value_type Texture::height() const noexcept {
    return _extent.y;
}

ember::math::uivec3::value_type Texture::width() const noexcept {
    return _extent.x;
}

ember::math::vec3_t<unsigned>::value_type Texture::layer() const noexcept {
    if (_type == TextureType::e2dArray) {
        return _extent.z;
    }

    if (_type == TextureType::eCube) {
        return 6ui32;
    }

    return 1ui32;
}

TextureFormat& Texture::format() noexcept {
    return _format;
}

TextureFormat Texture::format() const noexcept {
    return _format;
}

ember::u32& Texture::mipLevels() noexcept {
    return _mipLevels;
}

ember::u32 Texture::mipLevels() const noexcept {
    return _mipLevels;
}

TextureType& Texture::type() noexcept {
    return _type;
}

TextureType Texture::type() const noexcept {
    return _type;
}

vk::ImageView& Texture::vkView() noexcept {
    return _view;
}

const vk::ImageView& Texture::vkView() const noexcept {
    return _view;
}
