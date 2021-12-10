#include "LoaderManager.hpp"

using namespace ember::engine::res;
using namespace ember;

LoaderManager::LoaderManager() noexcept = default;

LoaderManager::~LoaderManager() noexcept = default;

ptr<Resource> LoaderManager::preload(const ptr<assets::Asset> asset_, ptr<void> options) {
    return nullptr;
}

ptr<Resource> LoaderManager::load(const ptr<assets::Asset> asset_, ptr<void> options) {
    return nullptr;
}

ptr<Resource> LoaderManager::loadImmediatly(const ptr<assets::Asset> asset_, ptr<void> options) {
    return nullptr;
}
