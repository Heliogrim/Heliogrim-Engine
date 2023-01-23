#include "ResourceManager.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Serialization.Layouts/LayoutManager.hpp>

using namespace ember::engine::res;
using namespace ember::engine;
using namespace ember;

ResourceManager::ResourceManager() noexcept :
    _importer(nullptr),
    _loader(nullptr),
    _locator(nullptr) {
    /**/
    serialization::LayoutManager::make();
    /**/
}

ResourceManager::~ResourceManager() noexcept {
    /**/
    serialization::LayoutManager::destroy();
    /**/
}

void ResourceManager::setup() {
    SCOPED_STOPWATCH

    if (!_importer) {
        _importer = make_uptr<ImporterManager>();
    }

    #ifdef _EDITOR
    if (!_indexer) {
        _indexer = make_uptr<Indexer>();
    }
    #endif

    if (!_loader) {
        _loader = make_uptr<resource::LoaderManager>();
    }

    if (!_locator) {
        _locator = make_uptr<Locator>();
    }
}

void ResourceManager::destroy() {
    _importer.reset();
    _indexer.reset();
    _loader.reset();
    _locator.reset();
}

void ResourceManager::schedule() {}

cref<ImporterManager> ResourceManager::importer() const {
    return *_importer;
}

ptr<const ImporterManager> ResourceManager::importer(std::nothrow_t) const noexcept {
    return _importer.get();
}

ref<ImporterManager> ResourceManager::importer() {
    return *_importer;
}

ptr<ImporterManager> ResourceManager::importer(std::nothrow_t) noexcept {
    return _importer.get();
}

ptr<Indexer> ResourceManager::indexer() const noexcept {
    return _indexer.get();
}

cref<resource::LoaderManager> ResourceManager::loader() const {
    return *_loader;
}

ptr<const resource::LoaderManager> ResourceManager::loader(std::nothrow_t) const noexcept {
    return _loader.get();
}

ref<resource::LoaderManager> ResourceManager::loader() {
    return *_loader;
}

ptr<resource::LoaderManager> ResourceManager::loader(std::nothrow_t) noexcept {
    return _loader.get();
}
