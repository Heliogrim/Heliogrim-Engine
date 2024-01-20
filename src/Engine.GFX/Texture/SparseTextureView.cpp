#include "SparseTextureView.hpp"

#include "SparseTexture.hpp"

using namespace hg::engine::gfx;
using namespace hg;

SparseTextureView::SparseTextureView(
    nmpt<SparseTexture> owner_,
    mref<Vector<nmpt<SparseTexturePage>>> pages_,
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
    _type(type_),
    _vkImageView(nullptr) {}

SparseTextureView::~SparseTextureView() = default;

const nmpt<const SparseTexture> SparseTextureView::owner() const noexcept {
    return _owner;
}

const nmpt<SparseTexture> SparseTextureView::owner() noexcept {
    return _owner;
}

cref<Vector<nmpt<SparseTexturePage>>> SparseTextureView::pages() const noexcept {
    return _pages;
}

math::uivec2::value_type SparseTextureView::baseLayer() const noexcept {
    return _layers.min;
}

math::uivec2::value_type SparseTextureView::layers() const noexcept {
    return (_layers.max - _layers.min) + 1ui32;
}

math::uivec3 SparseTextureView::extent() const noexcept {
    return _extent;
}

math::uivec3::value_type SparseTextureView::width() const noexcept {
    return _extent.x;
}

math::uivec3::value_type SparseTextureView::height() const noexcept {
    return _extent.y;
}

math::uivec3::value_type SparseTextureView::depth() const noexcept {
    return _extent.z;
}

TextureFormat SparseTextureView::format() const noexcept {
    return _format;
}

math::uivec2::value_type SparseTextureView::minMipLevel() const noexcept {
    return _mipLevels.min;
}

math::uivec2::value_type SparseTextureView::mipLevels() const noexcept {
    return (_mipLevels.max - _mipLevels.min) + 1ui32;
}

math::uivec2::value_type SparseTextureView::maxMipLevel() const noexcept {
    return _mipLevels.max;
}

TextureType SparseTextureView::type() const noexcept {
    return _type;
}

_::VkImageView SparseTextureView::vkImageView() const noexcept {
    return _vkImageView;
}

_::VkImageView SparseTextureView::vkImageView(mref<_::VkImageView> nextView_) noexcept {
    return _STD exchange(_vkImageView, nextView_);
}
