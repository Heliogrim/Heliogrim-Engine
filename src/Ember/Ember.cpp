#include "Ember.hpp"

#include <Engine.ACS/Registry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

#include "EmberStatic.hpp"

using namespace ember;

uptr<engine::Engine> Ember::_engine = nullptr;

static void startEngine(const non_owning_rptr<engine::Engine> engine_) {
    #ifdef _DEBUG
    assert(engine_->preInit());
    assert(engine_->init());
    assert(engine_->postInit());
    assert(engine_->start());
    #else
    engine_->preInit();
    engine_->init();
    engine_->postInit();
    engine_->start();
    #endif
}

void Ember::instantiate() {
    if (Ember::_engine) {
        throw _STD runtime_error("Ember engine already instantiated.");
    }

    Ember::_engine = EmberStatic::initialize();

    if (not Ember::_engine) {
        throw _STD runtime_error("Failed to initialize Ember Engine.");
    }
}

void Ember::destroy() {
    if (not Ember::_engine) {
        return;
    }

    Ember::_engine.reset();
}

const non_owning_rptr<engine::Engine> Ember::getEngine() noexcept {
    return Ember::_engine.get();
}

Session Ember::getSession() {
    const auto fnc { EmberStatic::sessionGetter() };
    const auto session = (Ember::_engine.get()->*fnc)();

    managed<void> dummy {};
    return Session {
        managed<void> { _STD move(dummy), session }
    };
}

Session Ember::getSession(std::nothrow_t) noexcept {
    if (not Ember::_engine || not EmberStatic::sessionGetter()) {
        return Session {};
    }

    return Ember::getSession();
}

AssetDatabase Ember::assets() noexcept {
    return AssetDatabase {
        _engine.get()->getAssets()->getDatabase()
    };
}
