#include "SerializationModule.hpp"

using namespace hg;

constinit std::atomic<ptr<const engine::SerializationModule>> engine::SerializationModule::_moduleSlot { nullptr };
static_assert(engine::core::ModuleSlotAccess::published<engine::SerializationModule>);
static_assert(std::is_same_v<decltype(engine::SerializationModule::get()), Opt<ref<const engine::SerializationModule>>>);

ref<engine::serialization::MigrationRegistry> engine::SerializationModule::migrations() noexcept {
	return _migrations;
}

cref<engine::serialization::MigrationRegistry> engine::SerializationModule::migrations() const noexcept {
	return _migrations;
}
