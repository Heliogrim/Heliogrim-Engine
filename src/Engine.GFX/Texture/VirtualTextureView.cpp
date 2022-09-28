#include "VirtualTextureView.hpp"

#include "VirtualTexture.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualTextureView::VirtualTextureView(
    const non_owning_rptr<VirtualTexture> owner_,
    mref<Vector<non_owning_rptr<VirtualTexturePage>>> pages_,
    const u32 layer_,
    const math::uivec3 extent_, const TextureFormat format_,
    const math::uivec2 mipLevels_, const TextureType type_
) :
    _owner(owner_),
    _pages(_STD move(pages_)),
    _baseLayer(layer_),
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

u32 VirtualTextureView::baseLayer() const noexcept {
    return _baseLayer;
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
