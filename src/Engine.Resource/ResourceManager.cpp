#include "ResourceManager.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Session/Session.hpp>

using namespace ember::engine::res;
using namespace ember::engine;
using namespace ember;

ResourceManager::ResourceManager(cref<sptr<Session>> session_) noexcept :
    _session(session_),
    _importer(nullptr),
    _loader(nullptr),
    _locator(nullptr) {}

ResourceManager::~ResourceManager() noexcept = default;

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
        _loader = make_uptr<LoaderManager>();
    }

    if (!_locator) {
        _locator = make_uptr<Locator>();
    }
}

void ResourceManager::schedule() {}

sptr<Session> ResourceManager::session() const noexcept {
    return _session;
}

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

cref<res::LoaderManager> ResourceManager::loader() const {
    return *_loader;
}

ptr<const res::LoaderManager> ResourceManager::loader(std::nothrow_t) const noexcept {
    return _loader.get();
}

ref<res::LoaderManager> ResourceManager::loader() {
    return *_loader;
}

ptr<res::LoaderManager> ResourceManager::loader(std::nothrow_t) noexcept {
    return _loader.get();
}
