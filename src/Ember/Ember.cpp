#include "Ember.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>
#include <Engine.ECS/Registry.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Network/Network.hpp>
#include <Engine.Resource/Resources.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Engine.Scheduler/Async.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.SFX/Audio.hpp>

using namespace ember;

static sptr<ember::engine::Audio> static_audio = nullptr;
static sptr<ember::engine::Graphics> static_graphics = nullptr;
static sptr<ember::engine::Network> static_network = nullptr;
static sptr<ember::engine::Resources> static_resources = nullptr;

ref<Audio> Ember::audio() noexcept {
    throw NotImplementedException();
}

ref<Graphics> Ember::graphics() noexcept {
    throw NotImplementedException();
}

ref<Network> Ember::network() noexcept {
    throw NotImplementedException();
}

ref<Scheduler> Ember::scheduler() noexcept {
    throw NotImplementedException();
}

void Ember::start() {
    engine::scheduler::Scheduler::make();
    engine::scheduler::Scheduler::get().setup(0);

    engine::scheduler::exec({
        []() {
            static_audio = make_sptr<ember::engine::Audio>();
            static_audio->setup();
            static_audio->schedule();
        }
    });

    engine::scheduler::exec({
        []() {
            const auto ptr = ember::engine::Graphics::make();
            static_graphics = sptr<ember::engine::Graphics>(ptr);
            static_graphics->setup();
            static_graphics->schedule();
        }
    });

    engine::scheduler::exec({
        []() {
            static_network = make_sptr<ember::engine::Network>();
            static_network->setup();
            static_network->schedule();
        }
    });

    engine::scheduler::exec({
        []() {
            static_resources = make_sptr<ember::engine::Resources>();
            static_resources->setup();
            static_resources->schedule();
        }
    });

    engine::scheduler::exec({
        []() {
            ember::engine::ecs::registry::make();
        }
    });

    engine::scheduler::exec({
        []() {
            ember::engine::scene::Scene::make();
        }
    });
}

void Ember::stop() {
    engine::scheduler::exec({
        []() {
            ember::engine::scene::Scene::destroy();
        }
    });

    engine::scheduler::exec({
        []() {
            ember::engine::ecs::registry::destroy();
        }
    });

    engine::scheduler::exec({
        []() {
            static_graphics.reset();
            static_audio.reset();
            static_network.reset();
            static_resources.reset();
        }
    });

    // Wait until every task to destruct finished
    wait();

    engine::scheduler::Scheduler::destroy();
}

void Ember::wait() { }

ref<world::World> Ember::world() noexcept {
    throw NotImplementedException();
}
