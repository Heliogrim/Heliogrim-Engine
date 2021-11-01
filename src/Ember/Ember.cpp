#include "Ember.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Common/Exception/NotImplementedException.hpp>
#include <Engine.ECS/System.hpp>
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

#include "Engine.ECS.Subsystem/Subsystem.hpp"

using namespace ember;

ref<Audio> Ember::audio() noexcept {
    throw NotImplementedException();
}

AssetDatabase Ember::assets() noexcept {
    return AssetDatabase {
        engine::Session::get()->assetDatabase()
    };
}

Graphics Ember::graphics() noexcept {
    return Graphics {
        engine::Session::get()->graphics()
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

    /**
     *
     */
    auto audio = make_ptr<engine::Audio>(session);
    session->setAudio(audio);

    // auto graphics = make_sptr<engine::Graphics>(static_session.get());
    auto graphics = engine::Graphics::make(session);
    session->setGraphics(graphics);

    auto network = make_ptr<engine::Network>(session);
    session->setNetwork(network);

    auto scene = make_ptr<engine::scene::Scene>(session);
    session->setScene(scene);

    // auto ecs = make_sptr<engine::ecs::Registry>(static_session.get());
    auto ecs = make_ptr<engine::ecs::System>();
    session->setEcs(ecs);

    auto subsystem = make_ptr<engine::ecs::Subsystem>(session, ecs);
    session->setSubsystem(subsystem);

    auto physics = make_ptr<engine::Physics>(session);
    session->setPhysics(physics);

    /**
     *
     */
    subsystem->setup();
    subsystem->schedule();

    audio->setup();
    audio->schedule();

    scene->setup();

    graphics->setup();
    graphics->schedule();

    physics->setup();
    physics->schedule();

    network->setup();
    network->schedule();

    /**
     *
     */
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

    delete static_cast<ptr<engine::Network>>(session->network());
    session->setNetwork(nullptr);

    engine::Graphics::destroy();
    session->setGraphics(nullptr);

    delete static_cast<ptr<engine::Audio>>(session->audio());
    session->setAudio(nullptr);

    delete static_cast<ptr<engine::Physics>>(session->physics());
    session->setPhysics(nullptr);

    delete static_cast<ptr<engine::scene::Scene>>(session->scene());
    session->setScene(nullptr);

    delete session->subsystem();
    session->setSubsystem(nullptr);

    delete session->ecs();
    session->setEcs(nullptr);
}

void Ember::wait() {
    /**
     *
     */
    engine::Session::get()->wait();
}

ref<World> Ember::world() noexcept {
    throw NotImplementedException();
}
