#include <Engine.Assets/Types/Sound.hpp>
/**/
#include "SoundAsset.hpp"

#include <Engine.Reflect/IsType.hpp>

using namespace hg;

SoundAsset::SoundAsset(mref<asset_guid> guid_, ref<::hg::engine::assets::Asset> internal_) noexcept :
	Asset(std::move(guid_), engine::assets::Sound::typeId, internal_) {}

SoundAsset::~SoundAsset() noexcept = default;

bool SoundAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::Sound>(*internal());
}
