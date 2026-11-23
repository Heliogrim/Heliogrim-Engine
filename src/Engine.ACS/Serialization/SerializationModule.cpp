#include "SerializationModule.hpp"

using namespace hg;

ref<engine::serialization::MigrationRegistry> engine::SerializationModule::migrations() noexcept {
	return _migrations;
}

cref<engine::serialization::MigrationRegistry> engine::SerializationModule::migrations() const noexcept {
	return _migrations;
}
