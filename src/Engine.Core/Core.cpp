#include "Core.hpp"

#include <Engine.GFX/Graphics.hpp>
#include <Engine.Scene/RevScene.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Scheduler/Thread/Thread.hpp>
#include <Engine.Scheduler/Task/Task.hpp>

#include "Ember/World.hpp"

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
    // TODO: Store associated render scenes and render targets to ::ember::engine::gfx::scene::RenderSceneManager
}

void Core::stop() {

    const auto session { Session::get() };
    _stopped.test_and_set(_STD memory_order_release);

    // TODO: Shutdown scenes

    sptr<::ember::engine::scene::Scene> holder { nullptr };
    for (const auto scene : _scenes) {
        if (scene.get() == _currentMainScene) {
            holder = scene;
        }
    }
}

const ptr<scene::Scene> Core::getCurrentScene() const noexcept {
    return _currentMainScene;
}

void Core::setCurrentScene(const ptr<scene::Scene> scene_) {
    _currentMainScene = scene_;
}

sptr<ember::engine::scene::Scene> Core::resolveScene(const ptr<World> world_) {

    const auto where {
        _STD find_if(_scenes.begin(), _scenes.end(),
            [scene = world_->unwrap()](cref<sptr<::ember::engine::scene::Scene>> entry_) {
                return scene == entry_.get();
            })
    };

    if (where != _scenes.end()) {
        return *where;
    }

    return nullptr;
}

void Core::addScene(cref<sptr<ember::engine::scene::Scene>> scene_) {
    _scenes.insert(scene_);
}

void Core::removeScene(cref<sptr<ember::engine::scene::Scene>> scene_) {

    if (_currentMainScene == scene_.get()) {
        _currentMainScene = nullptr;
    }

    _scenes.erase(scene_);
}

void Core::removeScene(mref<ptr<ember::engine::scene::Scene>> scene_) {

    if (_currentMainScene == scene_) {
        _currentMainScene = nullptr;
    }

    const auto where = _STD find_if(_scenes.begin(), _scenes.end(),
        [scene_](cref<sptr<::ember::engine::scene::Scene>> entry_) {
            return entry_.get() == scene_;
        });

    if (where != _scenes.end()) {
        _scenes.erase(where);
    }
}
