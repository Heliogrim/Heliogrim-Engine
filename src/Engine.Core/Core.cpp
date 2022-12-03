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
    _currentMainScene(nullptr),
    _currentUiScene(nullptr) {

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

    #pragma region Temporary UI Override
    _currentUiScene = new scene::RevScene();
    #pragma endregion

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

    #pragma region Temporary UI Override
    session->modules().scheduler()->exec(
        scheduler::task::make_repetitive_task([&sig = _stopped, &scene = _currentUiScene]() {
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
    #pragma endregion

    /**
     * Use current scene
     */
    // TODO: Store associated render scenes and render targets to ::ember::engine::gfx::scene::RenderSceneManager
}

void Core::stop() {

    const auto session { Session::get() };
    _stopped.test_and_set(_STD memory_order_release);

    // TODO: Shutdown scenes
}

const ptr<scene::Scene> Core::getCurrentScene() const noexcept {
    return _currentMainScene;
}
