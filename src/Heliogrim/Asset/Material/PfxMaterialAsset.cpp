#include <Engine.Assets.Type/Material/PfxMaterial.hpp>
/**/
#include "PfxMaterialAsset.hpp"

#include <Engine.Reflect/IsType.hpp>

using namespace hg;

PfxMaterialAsset::PfxMaterialAsset(mref<asset_guid> guid_, ref<::hg::engine::assets::Asset> internal_) noexcept :
	Asset(
		std::move(guid_),
		engine::assets::PfxMaterial::typeId,
		internal_
	) {}

PfxMaterialAsset::~PfxMaterialAsset() noexcept = default;

bool PfxMaterialAsset::isValidType() const noexcept {
	return internal() != nullptr && IsType<::hg::engine::assets::PfxMaterial>(*internal());
}
