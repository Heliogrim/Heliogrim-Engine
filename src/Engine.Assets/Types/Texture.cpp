#include "Texture.hpp"

using namespace ember::engine::assets;
using namespace ember;

Texture::Texture(cref<asset_guid> guid_) :
    Data(guid_, typeId) {}

Texture::Texture(cref<asset_guid> guid_, cref<asset_guid> baseImage_, mref<Vector<asset_guid>> images_,
    cref<math::uivec3> extent_, cref<gfx::TextureFormat> format_, cref<u32> mipLevel_, cref<gfx::TextureType> type_) :
    Data(guid_, typeId),
    _baseImage(baseImage_),
    _images(images_),
    _extent(extent_),
    _format(format_),
    _mipLevel(mipLevel_),
    _textureType(type_) {}

cref<asset_guid> Texture::baseImage() const noexcept {
    return _baseImage;
}

cref<math::vec3_t<unsigned>> Texture::getExtent() const noexcept {
    return _extent;
}

engine::gfx::TextureFormat Texture::getTextureFormat() const noexcept {
    return _format;
}

u32 Texture::getMipLevelCount() const noexcept {
    return _mipLevel;
}

engine::gfx::TextureType Texture::getTextureType() const noexcept {
    return _textureType;
}
