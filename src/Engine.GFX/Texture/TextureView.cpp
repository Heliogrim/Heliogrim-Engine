#include "TextureView.hpp"

#include "Texture.hpp"

using namespace ember::engine::gfx;
using namespace ember;

TextureView::TextureView(
    non_owning_rptr<Texture> owner_,
    math::uivec2 layers_,
    math::uExtent3D extent_,
    math::uivec2 mipLevels_
) :
    _owner(owner_),
    _layers(layers_),
    _extent(extent_),
    _mipLevels(mipLevels_) {}

TextureView::~TextureView() = default;

const non_owning_rptr<const Texture> TextureView::owner() const noexcept {
    return _owner;
}

const non_owning_rptr<Texture> TextureView::owner() noexcept {
    return _owner;
}

math::uivec2::value_type TextureView::baseLayer() const noexcept {
    return _layers.min;
}

math::uivec2::value_type TextureView::layers() const noexcept {
    return (_layers.max - _layers.min) + 1ui32;
}

math::uExtent3D::off_type TextureView::offsetX() const noexcept {
    return _extent.offsetX;
}

math::uExtent3D::off_type TextureView::offsetY() const noexcept {
    return _extent.offsetY;
}

math::uExtent3D::off_type TextureView::offsetZ() const noexcept {
    return _extent.offsetZ;
}

math::uExtent3D::value_type TextureView::width() const noexcept {
    return _extent.width;
}

math::uExtent3D::value_type TextureView::height() const noexcept {
    return _extent.height;
}

math::uExtent3D::value_type TextureView::depth() const noexcept {
    return _extent.depth;
}

math::vec2 TextureView::minSt() const noexcept {
    return math::vec2 {
        static_cast<float>(_extent.offsetX) / static_cast<float>(_owner->extent().x),
        static_cast<float>(_extent.offsetY) / static_cast<float>(_owner->extent().y)
    };
}

math::vec2 TextureView::maxSt() const noexcept {
    return math::vec2 {
        static_cast<float>(_extent.offsetX + _extent.width) / static_cast<float>(_owner->extent().x),
        static_cast<float>(_extent.offsetY + _extent.height) / static_cast<float>(_owner->extent().y)
    };
}

math::uivec2::value_type TextureView::minMipLevel() const noexcept {
    return _mipLevels.min;
}

math::uivec2::value_type TextureView::mipLevels() const noexcept {
    return (_mipLevels.max - _mipLevels.min) + 1ui32;
}

math::uivec2::value_type TextureView::maxMipLevel() const noexcept {
    return _mipLevels.max;
}
