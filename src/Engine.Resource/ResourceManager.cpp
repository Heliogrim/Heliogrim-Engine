#include "ResourceManager.hpp"

#ifdef _PROFILING
#endif

#include <Engine.Common/__macro.hpp>

import Engine.Common;
import Engine.Core;
import Engine.Serialization.Layouts;
import Engine.Storage;

using namespace hg::engine::resource;
using namespace hg::engine;
using namespace hg;

constinit std::atomic<ptr<engine::ResourceManager>> engine::ResourceManager::_moduleSlot { nullptr };
static_assert(engine::core::ModuleSlotAccess::published<engine::ResourceManager>);

ResourceManager::ResourceManager() noexcept :
	_importer(nullptr),
	_loader(nullptr) {
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
		_importer = make_uptr<res::ImporterManager>();
	}

	#ifdef _EDITOR
	if (!_indexer) {
		_indexer = make_uptr<res::Indexer>();
	}
	#endif

	if (!_loader) {
		auto engine = Engine::getEngine();
		_loader = make_uptr<LoaderManager>(*StorageModule::get());
	}
}

void ResourceManager::destroy() {
	_importer.reset();
	_indexer.reset();
	_loader.reset();
}

void ResourceManager::schedule() {}

cref<res::ImporterManager> ResourceManager::importer() const {
	return *_importer;
}

ptr<const res::ImporterManager> ResourceManager::importer(nothrow_t) const noexcept {
	return _importer.get();
}

ref<res::ImporterManager> ResourceManager::importer() {
	return *_importer;
}

ptr<res::ImporterManager> ResourceManager::importer(nothrow_t) noexcept {
	return _importer.get();
}

ptr<res::Indexer> ResourceManager::indexer() const noexcept {
	return _indexer.get();
}

cref<LoaderManager> ResourceManager::loader() const {
	return *_loader;
}

ptr<const LoaderManager> ResourceManager::loader(nothrow_t) const noexcept {
	return _loader.get();
}

ref<LoaderManager> ResourceManager::loader() {
	return *_loader;
}

ptr<LoaderManager> ResourceManager::loader(nothrow_t) noexcept {
	return _loader.get();
}
