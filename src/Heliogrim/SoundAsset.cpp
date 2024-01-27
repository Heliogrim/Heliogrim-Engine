#include "SoundAsset.hpp"

#include <Engine.Assets/Types/Sound.hpp>

using namespace hg;

SoundAsset::SoundAsset(mref<asset_guid> guid_) noexcept :
    StreamableAsset(std::move(guid_), engine::assets::Sound::typeId) {}

SoundAsset::~SoundAsset() noexcept = default;

bool SoundAsset::isValidType() const noexcept {
    return _typeId == engine::assets::Sound::typeId;
}
