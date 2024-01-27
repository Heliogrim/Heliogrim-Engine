#include "ImageAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace hg;

ImageAsset::ImageAsset(mref<asset_guid> guid_) noexcept :
    Asset(
        clone(guid_),
        engine::assets::Image::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createImageAsset(clone(guid_))
    ) {}

ImageAsset::ImageAsset(mref<asset_guid> guid_, cref<string> url_) noexcept :
    Asset(
        clone(guid_),
        engine::assets::Image::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createImageAsset(clone(guid_), url_)
    ) {}

ImageAsset::~ImageAsset() noexcept = default;

bool ImageAsset::isValidType() const noexcept {
    return _typeId == engine::assets::Image::typeId;
}
