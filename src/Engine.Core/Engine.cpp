#include "Engine.hpp"

#include <cassert>
#include <Engine.Logging/Logger.hpp>

#include "EngineState.hpp"

using namespace hg::engine::core;
using namespace hg::engine;
using namespace hg;

non_owning_rptr<Engine> Engine::_cached = nullptr;

Engine::Engine() {
    Engine::_cached = this;
    Logger::setup();
}

Engine::~Engine() {
    tidy();
    Logger::destroy();
    Engine::_cached = nullptr;
}

non_owning_rptr<Engine> Engine::getEngine() noexcept {
    return _cached;
}

void Engine::tidy() {
    if (getEngineState() == core::EngineState::eStarted) {
        stop();
        assert(getEngineState() < core::EngineState::eStarted);
    }

    if (getEngineState() == core::EngineState::eStopped || getEngineState() == core::EngineState::eInitialized) {
        shutdown();
        assert(getEngineState() < core::EngineState::eInitialized);
    }

    if (getEngineState() == core::EngineState::ePreInitialized) {
        exit();
        assert(getEngineState() < core::EngineState::ePreInitialized);
    }
}

bool Engine::setEngineState(core::EngineState state_) {
    const auto value { static_cast<u8>(state_) };
    const auto prev { _engineState.load() };

    if (value == prev) {
        return false;
    }

    if (prev < value) {
        u8 expect { static_cast<u8>(value - 1u) };
        return _engineState.compare_exchange_strong(expect, value);
    }

    u8 expect { prev };
    return _engineState.compare_exchange_strong(expect, value);
}

core::EngineState Engine::getEngineState() const noexcept {
    return static_cast<core::EngineState>(_engineState.load());
}
