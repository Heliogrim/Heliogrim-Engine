#include "SoundAsset.hpp"

#include <Engine.Assets/Types/Sound.hpp>

using namespace ember;

SoundAsset::SoundAsset(cref<asset_guid> guid_) noexcept :
    StreamableAsset(guid_, engine::assets::Sound::typeId) {}

SoundAsset::~SoundAsset() noexcept = default;

bool SoundAsset::isValidType() const noexcept {
    return _typeId == engine::assets::Sound::typeId;
}
