#include "Ember.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Common/Exception/NotImplementedException.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Event/BootEvent.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>
#include <Engine.Event/SchedulerBootEvent.hpp>
#include <Engine.Event/ShutdownEvent.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Network/Network.hpp>
#include <Engine.PFX/Physics.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scene/Scene.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.Session/Session.hpp>
#include <Engine.SFX/Audio.hpp>

using namespace ember;

ref<Audio> Ember::audio() noexcept {
    throw NotImplementedException();
}

AssetDatabase Ember::assets() noexcept {
    return AssetDatabase {
        engine::Session::get()->modules().assetDatabase()
    };
}

Graphics Ember::graphics() noexcept {
    return Graphics {
        engine::Session::get()->modules().graphics()
    };
}

ref<Network> Ember::network() noexcept {
    throw NotImplementedException();
}

ref<Scheduler> Ember::scheduler() noexcept {
    throw NotImplementedException();
}

void Ember::start() {

    /**
     *
     */
    auto session = engine::Session::get();
    session->start();
}

void Ember::stop() {
    /**
     *
     */
    auto session = engine::Session::get();

    /**
     *
     */
    session->emitter().emit<ShutdownEvent>();
    session->stop();
}

void Ember::wait() {
    /**
     *
     */
    if (auto session = engine::Session::get()) {
        session->wait();
    }
}

ref<World> Ember::world() noexcept {
    throw NotImplementedException();
}
