#include "LoaderManager.hpp"

using namespace ember::engine::res;
using namespace ember;

LoaderManager::LoaderManager() noexcept = default;

LoaderManager::~LoaderManager() noexcept = default;

sptr<LoaderBase> LoaderManager::selectLoader(cref<asset_type_id> typeId_, ptr<void> options_) const noexcept {

    const auto result { _loader.find(typeId_) };
    if (result == _loader.end()) {
        return nullptr;
    }

    // TODO: evaluate correct loader (requires multiple mapped loaders per type)

    return result->second;
}

bool LoaderManager::registerLoader(cref<asset_type_id> typeId_, cref<sptr<LoaderBase>> loader_) noexcept {
    const auto result { _loader.insert({ typeId_, loader_ }) };
    return result.second;
}

LoaderManager::load_type LoaderManager::preload(const ptr<assets::Asset> asset_, ptr<void> options_) {
    // TODO:
    return load(asset_, options_);
}

LoaderManager::load_type LoaderManager::load(const ptr<assets::Asset> asset_, ptr<void> options_) {
    /**
     *
     */
    const auto loader { selectLoader(asset_->getTypeId(), options_) };

    if (loader == nullptr) {
        return nullptr;
    }

    /**
     *
     */
    return loader->operator()(asset_, options_);
}

LoaderManager::load_type LoaderManager::loadImmediatly(const ptr<assets::Asset> asset_, ptr<void> options_) {
    // TODO:
    return load(asset_, options_);
}
