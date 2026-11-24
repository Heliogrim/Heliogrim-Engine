#include "StorageModule.hpp"

import Engine.Common;
import Engine.Config;
import Engine.Storage.Boot;
import Engine.Storage.Registry;
import Engine.Storage.System;

using namespace hg::engine;
using namespace hg;

constinit std::atomic<ptr<const engine::StorageModule>> engine::StorageModule::_moduleSlot { nullptr };
static_assert(engine::core::ModuleSlotAccess::published<engine::StorageModule>);
static_assert(std::is_same_v<decltype(engine::StorageModule::get()), Opt<ref<const engine::StorageModule>>>);

StorageModule::StorageModule(ref<Engine> engine_) :
	_io(),
	_registry(make_uptr<storage::StorageRegistry>()),
	_resolver(make_uptr<storage::StorageActionResolver>()),
	_system(make_uptr<storage::StorageSystem>(*_registry, *_resolver)) {}

StorageModule::~StorageModule() = default;

void StorageModule::setup(cref<Config> config_) {
	static_cast<ptr<storage::StorageRegistry>>(_registry.get())->setup(config_);
	storage::initActionResolver(*_system, *_resolver);

	// A missing, mistyped or zero entry keeps the default.
	constexpr auto fallback = io::IoLimits {};
	const auto limit = [&config_](const cfg::StorageConfigProperty key_, const u32 default_) {
		const auto value = config_.getOrDefault<u32>(key_, default_).value_or(default_);
		return value != 0u ? value : default_;
	};
	_io.setLimits(
		io::IoLimits {
			.maxInFlight = limit(cfg::StorageConfigProperty::eIoMaxInFlight, fallback.maxInFlight),
			.maxInFlightBytes = limit(cfg::StorageConfigProperty::eIoMaxInFlightBytes, fallback.maxInFlightBytes)
		}
	);
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
