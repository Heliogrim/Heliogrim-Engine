#include "ResourceManager.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::res;
using namespace ember;

ptr<engine::ResourceManager> engine::ResourceManager::_instance = nullptr;

engine::ResourceManager::ResourceManager() noexcept :
    _importer(nullptr),
    _loader(nullptr),
    _locator(nullptr) {}

engine::ResourceManager::~ResourceManager() noexcept = default;

ptr<engine::ResourceManager> engine::ResourceManager::get() noexcept {
    return _instance;
}

ptr<engine::ResourceManager> engine::ResourceManager::make() {
    if (_instance == nullptr) {
        _instance = new ResourceManager();
    }

    return _instance;
}

void engine::ResourceManager::destroy() noexcept {
    delete _instance;
    _instance = nullptr;
}

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
