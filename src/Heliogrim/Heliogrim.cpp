#include "Heliogrim.hpp"

#include <Engine.ACS/Registry.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

#include "HeliogrimStatic.hpp"

using namespace hg;

uptr<engine::Engine> Heliogrim::_engine = nullptr;

static void startEngine(const non_owning_rptr<engine::Engine> engine_) {
	assertrt(engine_->preInit());
	assertrt(engine_->init());
	assertrt(engine_->postInit());
	assertrt(engine_->start());
}

void Heliogrim::instantiate() {
	if (Heliogrim::_engine) {
		throw std::runtime_error("Heliogrim engine already instantiated.");
	}

	Heliogrim::_engine = HeliogrimStatic::initialize();

	if (not Heliogrim::_engine) {
		throw std::runtime_error("Failed to initialize Heliogrim Engine.");
	}
}

void Heliogrim::destroy() {
	if (not Heliogrim::_engine) {
		return;
	}

	Heliogrim::_engine.reset();
}

nmpt<engine::Engine> Heliogrim::getEngine() noexcept {
	return Heliogrim::_engine.get();
}

Session Heliogrim::getSession() {
	const auto fnc { HeliogrimStatic::sessionGetter() };
	const auto session = (Heliogrim::_engine.get()->*fnc)();

	SharedPtr<void> dummy {};
	return Session {
		SharedPtr<::hg::engine::core::Session> { std::move(dummy), session.get() }
	};
}

Session Heliogrim::getSession(std::nothrow_t) noexcept {
	if (not Heliogrim::_engine || not HeliogrimStatic::sessionGetter()) {
		return Session {};
	}

	return Heliogrim::getSession();
}

AssetDatabase Heliogrim::assets() noexcept {
	return AssetDatabase {
		_engine.get()->getAssets()->getRegistry().get()
	};
}
