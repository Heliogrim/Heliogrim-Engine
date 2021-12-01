#include "Core.hpp"

#include <Engine.GFX/Graphics.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>

using namespace ember::engine;
using namespace ember;

Core::Core() noexcept :
    _currentMainScene(nullptr) {}

Core::~Core() = default;

void Core::start() {

    auto session { Session::get() };

    /**
     * Build Default Scene
     */
    _currentMainScene = new scene::RevScene();

    /**
     * Use current scene
     */
    session->modules().graphics()->useAsRenderScene(static_cast<scene::RevScene*>(_currentMainScene));

    /**
     * Pseudo workload
     */
    scheduler::thread::self::sleepFor(15000);

}

void Core::stop() {

    auto session { Session::get() };

    /**
     * Unuse current scene
     */
    session->modules().graphics()->useAsRenderScene(nullptr);

    /**
     * Pseudo workload
     */
    scheduler::thread::self::sleepFor(15000);

}

const ptr<scene::Scene> Core::getCurrentScene() const noexcept {
    return _currentMainScene;
}
