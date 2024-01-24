#include "TextureAsset.hpp"

using namespace hg::engine::assets;
using namespace hg;

TextureAsset::TextureAsset() :
    InheritMeta(invalid_asset_guid, typeId) {}

TextureAsset::TextureAsset(cref<asset_guid> guid_) :
    InheritMeta(guid_, typeId) {}

TextureAsset::TextureAsset(
    cref<asset_guid> guid_,
    cref<asset_guid> baseImage_,
    mref<Vector<asset_guid>> images_,
    cref<math::uivec3> extent_,
    cref<gfx::TextureFormat> format_,
    cref<u32> mipLevel_,
    cref<gfx::TextureType> type_
) :
    InheritMeta(guid_, typeId),
    _baseImage(baseImage_),
    _images(images_),
    _extent(extent_),
    _format(format_),
    _mipLevel(mipLevel_),
    _textureType(type_) {}

cref<asset_guid> TextureAsset::baseImage() const noexcept {
    return _baseImage;
}

void TextureAsset::setBaseImage(const asset_guid imageGuid_) {
    _baseImage = imageGuid_;
}

void TextureAsset::addSourceImage(const asset_guid imageGuid_) {
    _images.push_back(imageGuid_);
}

bool TextureAsset::hasSourceImage(const asset_guid imageGuid_) {
    return std::find(_images.begin(), _images.end(), imageGuid_) != _images.end();
}

void TextureAsset::removeSourceImage(const asset_guid imageGuid_) {
    const auto where { std::remove(_images.begin(), _images.end(), imageGuid_) };
    _images.erase(where, _images.end());
}

cref<math::vec3_t<unsigned>> TextureAsset::getExtent() const noexcept {
    return _extent;
}

void TextureAsset::setExtent(cref<math::uivec3> extent_) {
    _extent = extent_;
}

engine::gfx::TextureFormat TextureAsset::getTextureFormat() const noexcept {
    return _format;
}

void TextureAsset::setTextureFormat(const gfx::TextureFormat format_) {
    _format = format_;
}

u32 TextureAsset::getMipLevelCount() const noexcept {
    return _mipLevel;
}

void TextureAsset::setMipLevelCount(const u32 levels_) {
    _mipLevel = levels_;
}

engine::gfx::TextureType TextureAsset::getTextureType() const noexcept {
    return _textureType;
}

void TextureAsset::setTextureType(const gfx::TextureType type_) {
    _textureType = type_;
}
