#include "StorageModule.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Storage.Boot/Action/ActionResolverInit.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage.System/Details/StorageActionResolver.hpp>

using namespace hg::engine;
using namespace hg;

StorageModule::StorageModule(ref<Engine> engine_) :
	_io(),
	_registry(make_uptr<storage::StorageRegistry>()),
	_resolver(make_uptr<storage::StorageActionResolver>()),
	_system(make_uptr<storage::StorageSystem>(*_registry, *_resolver)) {}

StorageModule::~StorageModule() = default;

void StorageModule::setup(cref<Config> config_) {
	static_cast<ptr<storage::StorageRegistry>>(_registry.get())->setup(config_);
	storage::initActionResolver(*_system, *_resolver);

	_io.start();
	_system->setIo(std::addressof(_io));
}

void StorageModule::stopIo() {
	// Note: Stop first, so queries starting after the stop resolve sync stages.
	// shortcut: a query already past `running()` may still cache an io-backed stage after the clear (its reads then come back empty), make `StackCache::clear` wait on the per-primary locks if storage is used across shutdown.
	_io.stop();
	_system->detachIo();
}

nmpt<storage::IStorageRegistry> StorageModule::getRegistry() const noexcept {
	return _registry.get();
}

nmpt<StorageModule::System> StorageModule::getSystem() const noexcept {
	return _system.get();
}

ref<io::IoSystem> StorageModule::io() noexcept {
	return _io;
}
