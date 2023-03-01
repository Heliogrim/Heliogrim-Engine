#include "VirtualTextureView.hpp"

#include "VirtualTexture.hpp"

using namespace hg::engine::gfx;
using namespace hg;

VirtualTextureView::VirtualTextureView(
    const non_owning_rptr<VirtualTexture> owner_,
    mref<Vector<non_owning_rptr<VirtualTexturePage>>> pages_,
    const math::uivec2 layers_,
    const math::uivec3 extent_,
    const TextureFormat format_,
    const math::uivec2 mipLevels_,
    const TextureType type_
) :
    _owner(owner_),
    _pages(_STD move(pages_)),
    _layers(layers_),
    _extent(extent_),
    _format(format_),
    _mipLevels(mipLevels_),
    _type(type_) {}

VirtualTextureView::~VirtualTextureView() = default;

const non_owning_rptr<const VirtualTexture> VirtualTextureView::owner() const noexcept {
    return _owner;
}

const non_owning_rptr<VirtualTexture> VirtualTextureView::owner() noexcept {
    return _owner;
}

cref<Vector<non_owning_rptr<VirtualTexturePage>>> VirtualTextureView::pages() const noexcept {
    return _pages;
}

math::uivec2::value_type VirtualTextureView::baseLayer() const noexcept {
    return _layers.min;
}

math::uivec2::value_type VirtualTextureView::layers() const noexcept {
    return (_layers.max - _layers.min) + 1ui32;
}

math::uivec3 VirtualTextureView::extent() const noexcept {
    return _extent;
}

math::uivec3::value_type VirtualTextureView::width() const noexcept {
    return _extent.x;
}

math::uivec3::value_type VirtualTextureView::height() const noexcept {
    return _extent.y;
}

math::uivec3::value_type VirtualTextureView::depth() const noexcept {
    return _extent.z;
}

TextureFormat VirtualTextureView::format() const noexcept {
    return _format;
}

math::uivec2::value_type VirtualTextureView::minMipLevel() const noexcept {
    return _mipLevels.min;
}

math::uivec2::value_type VirtualTextureView::mipLevels() const noexcept {
    return (_mipLevels.max - _mipLevels.min) + 1ui32;
}

math::uivec2::value_type VirtualTextureView::maxMipLevel() const noexcept {
    return _mipLevels.max;
}

TextureType VirtualTextureView::type() const noexcept {
    return _type;
}

_::VkImageView VirtualTextureView::vkImageView() const noexcept {
    return _vkImageView;
}

_::VkImageView VirtualTextureView::vkImageView(mref<_::VkImageView> nextView_) noexcept {
    return _STD exchange(_vkImageView, nextView_);
}
