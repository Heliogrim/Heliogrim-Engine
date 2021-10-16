#include "ResourceManager.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include <Engine.Session/Session.hpp>

using namespace ember::engine::res;
using namespace ember::engine;
using namespace ember;

engine::ResourceManager::ResourceManager(cref<ptr<Session>> session_) noexcept :
    _session(session_),
    _importer(nullptr),
    _loader(nullptr),
    _locator(nullptr) {}

engine::ResourceManager::~ResourceManager() noexcept = default;

void engine::ResourceManager::setup() {

    SCOPED_STOPWATCH

    if (!_importer) {
        _importer = _STD move(make_uptr<ImporterManager>());
    }

    #ifdef _EDITOR
    if (!_indexer) {
        _indexer = _STD move(make_uptr<Indexer>());
    }
    #endif

    if (!_loader) {
        _loader = _STD move(make_uptr<LoaderManager>());
    }

    if (!_locator) {
        _locator = _STD move(make_uptr<Locator>());
    }
}

void engine::ResourceManager::schedule() {}

ptr<engine::Session> engine::ResourceManager::session() const noexcept {
    return _session;
}

cref<ImporterManager> engine::ResourceManager::importer() const {
    return *_importer;
}

ptr<const ImporterManager> engine::ResourceManager::importer(std::nothrow_t) const noexcept {
    return _importer.get();
}

ref<ImporterManager> engine::ResourceManager::importer() {
    return *_importer;
}

ptr<ImporterManager> engine::ResourceManager::importer(std::nothrow_t) noexcept {
    return _importer.get();
}

ptr<Indexer> engine::ResourceManager::indexer() const noexcept {
    return _indexer.get();
}
