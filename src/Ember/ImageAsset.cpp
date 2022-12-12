#include "ImageAsset.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Core/Engine.hpp>

#include "Engine.Assets/Assets.hpp"

using namespace ember;

ImageAsset::ImageAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::Image::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createImageAsset(guid_)
    ) {}

ImageAsset::ImageAsset(cref<asset_guid> guid_, cref<string> url_) noexcept :
    Asset(guid_,
        engine::assets::Image::typeId,
        engine::Engine::getEngine()->getAssets()->getFactory()->createImageAsset(guid_, url_)
    ) {}

ImageAsset::~ImageAsset() noexcept = default;

bool ImageAsset::isValidType() const noexcept {
    return _typeId == engine::assets::Image::typeId;
}
