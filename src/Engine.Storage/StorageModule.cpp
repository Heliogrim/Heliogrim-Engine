#include "StorageModule.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Storage.Registry/Registry/StorageRegistry.hpp>
#include <Engine.Storage.System/StorageSystem.hpp>
#include <Engine.Storage.System/Details/StorageActionResolver.hpp>

using namespace hg::engine;
using namespace hg;

StorageModule::StorageModule(ref<Engine> engine_) :
	_registry(make_uptr<storage::StorageRegistry>()),
	_resolver(make_uptr<storage::StorageActionResolver>()),
	_system(make_uptr<storage::StorageSystem>(*_registry, *_resolver)) {}

StorageModule::~StorageModule() = default;

void StorageModule::setup(cref<Config> config_) {
	static_cast<ptr<storage::StorageRegistry>>(_registry.get())->setup(config_);
}

nmpt<storage::IStorageRegistry> StorageModule::getRegistry() const noexcept {
	return _registry.get();
}

nmpt<StorageModule::System> StorageModule::getSystem() const noexcept {
	return _system.get();
}
