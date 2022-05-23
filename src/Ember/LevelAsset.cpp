#include "LevelAsset.hpp"

#include <Engine.Assets/Types/Level.hpp>

using namespace ember;

LevelAsset::LevelAsset(cref<asset_guid> guid_) noexcept :
    Asset(
        guid_,
        engine::assets::Level::typeId,
        nullptr
    ) {}

LevelAsset::~LevelAsset() noexcept = default;

bool LevelAsset::isValidType() const noexcept {
    return _typeId == engine::assets::Level::typeId;
}
