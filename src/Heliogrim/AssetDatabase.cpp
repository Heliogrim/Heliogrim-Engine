#include "AssetDatabase.hpp"

#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>

import Engine.Asserts;
import Engine.Common;

using namespace hg;

AssetDatabase::AssetDatabase(nmpt<::hg::engine::assets::IAssetRegistry> internal_) noexcept :
	_internal(std::move(internal_)) {}

decltype(AssetDatabase::_internal) AssetDatabase::unwrap() const noexcept {
	return _internal;
}

bool AssetDatabase::contains(cref<AssetGuid> guid_) const noexcept {

	const auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal.get()) };
	return idb.hasAsset(guid_);
}

AssetDatabaseResult<AssetHandle> AssetDatabase::operator[](ref<const AssetGuid> guid_) const {

	const auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal.get()) };
	auto asset = idb.findAssetByGuid(guid_);

	if (asset == None || asset.value() == nullptr) {
		return AssetDatabaseResult<AssetHandle> {
			{ AssetDatabaseResultType::eFailed },
			{}
		};
	}

	return AssetDatabaseResult<AssetHandle> {
		{ AssetDatabaseResultType::eSuccess },
		{
			(*asset)->getAssetGuid(),
			(*asset)->getTypeId(),
			// Warning: Reference out of Scope | Use-After-Free
			Some(**asset)
		}
	};
}

/**/

bool hg::IsAssetsAvailable() noexcept {
	return engine::Engine::getEngine() != nullptr
		&& engine::Engine::getEngine()->getAssets() != nullptr
		&& engine::Engine::getEngine()->getAssets()->getRegistry() != nullptr;
}

AssetDatabase hg::GetAssets() {
	return AssetDatabase {
		engine::Engine::getEngine()->getAssets()->getRegistry().get()
	};
}
