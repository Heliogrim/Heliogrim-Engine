#include <Engine.Assets.Type/Texture/TextureAsset.hpp>
/**/
#include "TextureAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/IsType.hpp>

#include "Engine.Asserts/Todo.hpp"

using namespace hg;

TextureAsset::TextureAsset(mref<AssetGuid> guid_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::TextureAsset::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createTextureAsset(clone(guid_))
	) {}

TextureAsset::TextureAsset(
	mref<AssetGuid> guid_,
	cref<AssetGuid> baseImage_,
	mref<Vector<AssetGuid>> images_,
	cref<math::uivec3> extent_,
	cref<TextureFormat> format_,
	cref<u32> mipLevel_,
	cref<TextureType> type_
) noexcept :
	Asset(
		clone(guid_),
		engine::assets::TextureAsset::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createTextureAsset(
			clone(guid_),
			baseImage_,
			std::forward<Vector<AssetGuid>>(images_),
			extent_,
			format_,
			mipLevel_,
			type_
		)
	) {}

TextureAsset::~TextureAsset() noexcept = default;

bool TextureAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::TextureAsset>(*internal());
}

nmpt<const ImageAsset> TextureAsset::baseImage() const noexcept {
	::hg::todo_panic();
}

cref<AssetGuid> TextureAsset::baseImageGuid() const noexcept {
	::hg::todo_panic();
}

Vector<ptr<const ImageAsset>> TextureAsset::images() const noexcept {
	::hg::todo_panic();
}

Vector<AssetGuid> TextureAsset::imagesGuids() const noexcept {
	::hg::todo_panic();
}

cref<math::uivec3> TextureAsset::extent() const noexcept {
	::hg::todo_panic();
}

cref<TextureFormat> TextureAsset::format() const noexcept {
	::hg::todo_panic();
}

cref<u32> TextureAsset::mipLevel() const noexcept {
	::hg::todo_panic();
}

cref<TextureType> TextureAsset::type() const noexcept {
	::hg::todo_panic();
}
