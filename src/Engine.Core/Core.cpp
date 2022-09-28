#include "Core.hpp"

#include <Engine.GFX/Graphics.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Engine.Scheduler/Task/Task.hpp>

using namespace ember::engine;
using namespace ember;

Core::Core() noexcept :
    _stopped(),
    _currentMainScene(nullptr) {

    _stopped.test_and_set(_STD memory_order_relaxed);
}

Core::~Core() = default;

void Core::start() {

    const auto session { Session::get() };
    _stopped.clear(_STD memory_order_release);

    /**
     * Build Default Scene
     */
    _currentMainScene = new scene::RevScene();

    /**
     * Register function callbacks
     */
    session->modules().scheduler()->exec(
        scheduler::task::make_repetitive_task([&sig = _stopped, &scene = _currentMainScene]() {
                if (sig.test(_STD memory_order_consume)) {
                    return false;
                }

                if (scene) {
                    scene->update();
                }

                return true;
            },
            scheduler::task::TaskMask::eNormal,
            scheduler::ScheduleStageBarriers::ePublishStrong,
            scheduler::ScheduleStageBarriers::ePublishStrong
        )
    );

    /**
     * Use current scene
     */
    session->modules().graphics()->useAsRenderScene(
        static_cast<ptr<scene::IRenderScene>>(
            static_cast<scene::RevScene*>(_currentMainScene)
        )
    );

    /**
     * Pseudo workload
     */
    // scheduler::thread::self::sleepFor(10000);
}

void Core::stop() {

    const auto session { Session::get() };
    _stopped.test_and_set(_STD memory_order_release);

    /**
     * Unuse current scene
     */
    auto graphics { session->modules().graphics() };
    if (graphics) {
        graphics->useAsRenderScene(nullptr);
    }

    /**
     * Pseudo workload
     */
    scheduler::thread::self::sleepFor(1000);

}

const ptr<scene::Scene> Core::getCurrentScene() const noexcept {
    return _currentMainScene;
}
