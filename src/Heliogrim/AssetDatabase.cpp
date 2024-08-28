#include "AssetDatabase.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/AssetDescriptor.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg;

AssetDatabase::AssetDatabase(nmpt<::hg::engine::assets::IAssetRegistry> internal_) :
	_internal(std::move(internal_)) {}

AssetDatabase::~AssetDatabase() = default;

decltype(AssetDatabase::_internal) AssetDatabase::unwrap() const noexcept {
	return _internal;
}

bool AssetDatabase::contains(cref<asset_guid> guid_) const noexcept {

	const auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal.get()) };
	return idb.hasAsset(guid_);
}

AssetDatabaseResult<Asset> AssetDatabase::operator[](cref<asset_guid> guid_) const {

	const auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal.get()) };
	const auto asset = idb.findAssetByGuid(guid_);

	if (asset == nullptr) {
		return AssetDatabaseResult<Asset> {
			{ AssetDatabaseResultType::eFailed },
			hg::Asset { clone(invalid_asset_guid), asset_type_id { 0 }, nullptr }
		};
	}

	return AssetDatabaseResult<Asset> {
		{ AssetDatabaseResultType::eSuccess },
		{
			asset->get_guid(),
			asset->getTypeId(),
			// Warning: Reference out of Scope | Use-After-Free
			*asset.get()
		}
	};
}

bool AssetDatabase::insert(ref<Asset> asset_) noexcept {

	::hg::assertrt(asset_._internal != nullptr /* "Asset should have internal state representation." */);

	engine::assets::storeDefaultNameAndUrl(*asset_._internal, {});
	::hg::todo_panic();
	// TODO: return _internal->insert({ static_cast<ptr<engine::assets::Asset>>(asset_->_internal) });
}

bool AssetDatabase::erase(ref<Asset> asset_) noexcept {
	auto& idb { *static_cast<const non_owning_rptr<engine::assets::IAssetRegistry>>(_internal.get()) };
	return idb.removeAssetByGuid(asset_.guid());
}
