#include "ActorModule.hpp"

#include <Engine.Common/Make.hpp>

#include "Registry.hpp"

using namespace hg::engine::acs;
using namespace hg::engine;
using namespace hg;

ActorModule::ActorModule(ref<Engine> engine_) :
	CoreModule(engine_),
	_registry(nullptr) {}

ActorModule::~ActorModule() {
	destroy();
}

void ActorModule::setup() {
	_registry = make_uptr<ActorRegistry>();
}

void ActorModule::start() {}

void ActorModule::stop() {}

void ActorModule::destroy() {
	_registry.reset();
}

nmpt<ActorModule::ActorRegistry> ActorModule::getRegistry() const noexcept {
	return _registry.get();
}
