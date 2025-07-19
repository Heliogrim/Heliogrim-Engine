#include "LoaderManager.hpp"

#include <Engine.Common/Make.hpp>

#include "Loader/SourceLoader/SourceLoader.hpp"

using namespace hg::engine::resource;
using namespace hg;

LoaderManager::LoaderManager(cref<StorageModule> storageModule_) :
	_sharedSourceLoader(make_sptr<loader::SourceLoader>(storageModule_)),
	_loader() {}

LoaderManager::~LoaderManager() = default;

sptr<loader::SourceLoader> LoaderManager::sharedSourceLoader() const {
	return _sharedSourceLoader;
}

sptr<loader::LoaderBase> LoaderManager::selectLoader(cref<AssetTypeId> typeId_, ptr<void> options_) const noexcept {

	const auto result { _loader.find(typeId_) };
	if (result == _loader.end()) {
		return nullptr;
	}

	// TODO: evaluate correct loader (requires multiple mapped loaders per type)

	return result->second;
}

bool LoaderManager::hasLoader(cref<AssetTypeId> typeId_) const noexcept {
	const auto iter = _loader.find(typeId_);
	return iter != _loader.end() && iter->second.get() != nullptr;
}

bool LoaderManager::registerLoader(cref<AssetTypeId> typeId_, cref<sptr<loader::LoaderBase>> loader_) noexcept {
	const auto result { _loader.insert({ typeId_, loader_ }) };
	return result.second;
}

bool LoaderManager::unregisterLoader(mref<sptr<loader::LoaderBase>> loader_) noexcept {

	const auto iter = std::ranges::find_if(
		_loader,
		[loader_](const auto& pair_) {
			return pair_.second == loader_;
		}
	);

	if (iter == _loader.cend()) {
		return false;
	}

	_loader.erase(iter);
	return true;
}

bool LoaderManager::unregisterLoader(cref<AssetTypeId> typeId_) noexcept {

	const auto iter = _loader.find(typeId_);
	if (iter == _loader.cend()) {
		return false;
	}

	_loader.erase(iter);
	return true;
}

LoaderManager::response_base_type LoaderManager::preload(const ptr<const assets::Asset> asset_, ptr<void> options_) const {
	// TODO:
	return load(asset_, options_);
}

LoaderManager::response_base_type LoaderManager::load(const ptr<const assets::Asset> asset_, ptr<void> options_) const {
	/**
	 *
	 */
	const auto loader { selectLoader(asset_->getTypeId(), options_) };

	if (loader == nullptr) {
		return {};
	}

	/**/
	return loader->operator()(asset_, options_);
}

LoaderManager::response_base_type LoaderManager::loadImmediately(
	const ptr<const assets::Asset> asset_,
	ptr<void> options_
) const {
	// TODO:
	return load(asset_, options_);
}
