#include "StorageModule.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Storage.IO/StorageIo.hpp>

#include "Registry/StorageRegistry.hpp"

using namespace hg::engine;
using namespace hg;

StorageModule::StorageModule(ref<Engine> engine_) :
	CoreModule(engine_),
	_registry(make_uptr<storage::StorageRegistry>()),
	_io(make_uptr<StorageIo>(engine_)) {}

StorageModule::~StorageModule() = default;

void StorageModule::setup() {}

void StorageModule::start() {}

void StorageModule::stop() {}

void StorageModule::destroy() {}

nmpt<storage::IStorageRegistry> StorageModule::getRegistry() const noexcept {
	return _registry.get();
}

nmpt<storage::StorageIo> StorageModule::getIo() const noexcept {
	return _io.get();
}
