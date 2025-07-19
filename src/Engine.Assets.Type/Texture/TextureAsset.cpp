#include "TextureAsset.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::assets;
using namespace hg;

TextureAsset::TextureAsset() :
	InheritMeta(clone(invalid_asset_guid), TextureAsset::typeId, StringView {}, AssetReferenceUrl {}, AssetUrl {}),
	_baseImage(invalid_asset_guid),
	_images(),
	_extent(math::uivec3 { 0uL }),
	_format(gfx::TextureFormat::eUndefined),
	_mipLevel(0uL),
	_textureType(gfx::TextureType::eUndefined) {}

TextureAsset::TextureAsset(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_,
	cref<AssetGuid> baseImage_,
	mref<Vector<AssetGuid>> images_,
	cref<math::uivec3> extent_,
	cref<gfx::TextureFormat> format_,
	cref<u32> mipLevel_,
	cref<gfx::TextureType> type_
) :
	InheritMeta(std::move(guid_), typeId, ::hg::move(name_), ::hg::move(storageUrl_), ::hg::move(vfsUrl_)),
	_baseImage(baseImage_),
	_images(images_),
	_extent(extent_),
	_format(format_),
	_mipLevel(mipLevel_),
	_textureType(type_) {}

cref<AssetGuid> TextureAsset::baseImage() const noexcept {
	return _baseImage;
}

void TextureAsset::setBaseImage(const AssetGuid imageGuid_) {
	_baseImage = imageGuid_;
}

void TextureAsset::addSourceImage(const AssetGuid imageGuid_) {
	_images.push_back(imageGuid_);
}

bool TextureAsset::hasSourceImage(const AssetGuid imageGuid_) {
	return std::find(_images.begin(), _images.end(), imageGuid_) != _images.end();
}

void TextureAsset::removeSourceImage(const AssetGuid imageGuid_) {
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
