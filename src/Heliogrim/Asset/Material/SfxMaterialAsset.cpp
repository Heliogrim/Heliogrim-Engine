#include <Engine.Assets/Types/SfxMaterial.hpp>
/**/
#include "SfxMaterialAsset.hpp"

#include <Engine.Reflect/IsType.hpp>

using namespace hg;

SfxMaterialAsset::SfxMaterialAsset(mref<asset_guid> guid_, ref<::hg::engine::assets::Asset> internal_) noexcept :
	Asset(
		std::move(guid_),
		engine::assets::SfxMaterial::typeId,
		internal_
	) {}

SfxMaterialAsset::~SfxMaterialAsset() noexcept = default;

bool SfxMaterialAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<engine::assets::SfxMaterial>(*internal());
}
