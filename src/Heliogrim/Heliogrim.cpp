#include "Heliogrim.hpp"

#include <Engine.ACS/Registry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

#include "HeliogrimStatic.hpp"

using namespace hg;

uptr<engine::Engine> Heliogrim::_engine = nullptr;

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

void Heliogrim::instantiate() {
    if (Heliogrim::_engine) {
        throw _STD runtime_error("Heliogrim engine already instantiated.");
    }

    Heliogrim::_engine = HeliogrimStatic::initialize();

    if (not Heliogrim::_engine) {
        throw _STD runtime_error("Failed to initialize Heliogrim Engine.");
    }
}

void Heliogrim::destroy() {
    if (not Heliogrim::_engine) {
        return;
    }

    Heliogrim::_engine.reset();
}

const non_owning_rptr<engine::Engine> Heliogrim::getEngine() noexcept {
    return Heliogrim::_engine.get();
}

Session Heliogrim::getSession() {
    const auto fnc { HeliogrimStatic::sessionGetter() };
    const auto session = (Heliogrim::_engine.get()->*fnc)();

    managed<void> dummy {};
    return Session {
        managed<void> { _STD move(dummy), session }
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
        _engine.get()->getAssets()->getRegistry()
    };
}
