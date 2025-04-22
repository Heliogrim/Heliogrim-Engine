#include <Engine.Assets.Type/Texture/Image.hpp>
/**/
#include "ImageAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg;

ImageAsset::ImageAsset(mref<AssetGuid> guid_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::Image::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createImageAsset(clone(guid_))
	) {}

ImageAsset::ImageAsset(mref<AssetGuid> guid_, cref<string> url_) noexcept :
	Asset(
		clone(guid_),
		engine::assets::Image::typeId,
		// Warning: Reference out of Scope | Use-After-Free
		*engine::Engine::getEngine()->getAssets()->getFactory()->createImageAsset(clone(guid_), url_)
	) {}

ImageAsset::~ImageAsset() noexcept = default;

bool ImageAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::Image>(*internal());
}
