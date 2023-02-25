#include "Texture.hpp"

using namespace ember::engine::assets;
using namespace ember;

Texture::Texture() :
    Asset(invalid_asset_guid, typeId) {}

Texture::Texture(cref<asset_guid> guid_) :
    Asset(guid_, typeId) {}

Texture::Texture(
    cref<asset_guid> guid_,
    cref<asset_guid> baseImage_,
    mref<Vector<asset_guid>> images_,
    cref<math::uivec3> extent_,
    cref<gfx::TextureFormat> format_,
    cref<u32> mipLevel_,
    cref<gfx::TextureType> type_
) :
    Asset(guid_, typeId),
    _baseImage(baseImage_),
    _images(images_),
    _extent(extent_),
    _format(format_),
    _mipLevel(mipLevel_),
    _textureType(type_) {}

cref<asset_guid> Texture::baseImage() const noexcept {
    return _baseImage;
}

void Texture::setBaseImage(const asset_guid imageGuid_) {
    _baseImage = imageGuid_;
}

void Texture::addSourceImage(const asset_guid imageGuid_) {
    _images.push_back(imageGuid_);
}

bool Texture::hasSourceImage(const asset_guid imageGuid_) {
    return _STD find(_images.begin(), _images.end(), imageGuid_) != _images.end();
}

void Texture::removeSourceImage(const asset_guid imageGuid_) {
    const auto where { _STD remove(_images.begin(), _images.end(), imageGuid_) };
    _images.erase(where, _images.end());
}

cref<math::vec3_t<unsigned>> Texture::getExtent() const noexcept {
    return _extent;
}

void Texture::setExtent(cref<math::uivec3> extent_) {
    _extent = extent_;
}

engine::gfx::TextureFormat Texture::getTextureFormat() const noexcept {
    return _format;
}

void Texture::setTextureFormat(const gfx::TextureFormat format_) {
    _format = format_;
}

u32 Texture::getMipLevelCount() const noexcept {
    return _mipLevel;
}

void Texture::setMipLevelCount(const u32 levels_) {
    _mipLevel = levels_;
}

engine::gfx::TextureType Texture::getTextureType() const noexcept {
    return _textureType;
}

void Texture::setTextureType(const gfx::TextureType type_) {
    _textureType = type_;
}
