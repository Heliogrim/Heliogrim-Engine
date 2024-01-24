#include <Windows.h>
#include <Engine.Common/__macro.hpp>

#include <Editor.Action/Action/DelegateAction.hpp>
#include <Engine.Common/stdafx.h>
#include <Engine.Logging/Logger.hpp>
#include <Heliogrim.Main/Heliogrim.Main.hpp>

#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Event/SignalShutdownEvent.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Heliogrim/Actor.hpp>

using namespace hg;

inline static std::atomic_flag suspended {};

#pragma region Heliogrim Injection Declarations

void beforeRoutine() {}

void onEngineCreate(const hg::non_owning_rptr<Engine> engine_) {}

void onEnginePreInit(const hg::non_owning_rptr<Engine> engine_) {}

void onEngineInit(const hg::non_owning_rptr<Engine> engine_) {}

void onEnginePostInit(const hg::non_owning_rptr<Engine> engine_) {}

void onEngineStart(const hg::non_owning_rptr<Engine> engine_) {}

void onEngineRunning(const hg::non_owning_rptr<Engine> engine_) {}

void onEngineStop(const hg::non_owning_rptr<Engine> engine_) {}

void onEngineShutdown(const hg::non_owning_rptr<Engine> engine_) {}

void onEngineExit(const hg::non_owning_rptr<Engine> engine_) {}

void afterRoutine() {}

#pragma endregion

/**
 * Heliogrim main entry
 *
 * @author Julius
 * @date 20.10.2021
 */
void ember_main_entry() {
    /**
     * Create suspend callback for event emitter
     */
    engine::Engine::getEngine()->getEmitter().on<engine::core::SignalShutdownEvent>(
        [](cref<engine::core::SignalShutdownEvent> event_) {
            suspended.test_and_set(std::memory_order::release);
        }
    );
}
