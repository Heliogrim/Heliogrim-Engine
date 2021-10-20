#include "SessionModules.hpp"

#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Database/AssetSeeder.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scheduler/Scheduler.hpp>

using namespace ember::engine::session;
using namespace ember;

SessionModules::SessionModules() noexcept = default;

SessionModules::~SessionModules() noexcept = default;

void SessionModules::tidy() {

    /**
     * Destroy - Resources
     */
    delete static_cast<ptr<ResourceManager>>(_resourceManager);
    _resourceManager = nullptr;

    /**
     * Destroy - Asset Database & Factory
     */
    delete _assetFactory;
    _assetFactory = nullptr;

    delete _assetDatabase;
    _assetDatabase = nullptr;

    /**
     * Destroy - Scheduler
     */
    scheduler::Scheduler::destroy();
    _scheduler = nullptr;
}

void SessionModules::setup() {

    /**
     * Prepare - Scheduler
     */
    _scheduler = scheduler::Scheduler::make();

    /**
     * Prepare - Asset Database & Factory
     */
    _assetDatabase = make_ptr<assets::AssetDatabase>();
    _assetFactory = make_ptr<assets::AssetFactory>(_assetDatabase);
    _assetSeeder = make_ptr<assets::AssetSeeder>();

    /**
     * Prepare - Resources
     */
    _resourceManager = new ResourceManager(static_cast<ptr<Session>>(this));

    /**
     * Setup
     */
    static_cast<ptr<scheduler::Scheduler>>(_scheduler)->setup(0);

    static_cast<ptr<ResourceManager>>(_resourceManager)->setup();
    static_cast<ptr<ResourceManager>>(_resourceManager)->schedule();
}

void SessionModules::start() {
    /**
     *
     */
    _assetSeeder->seed(_scheduler);
}

void SessionModules::wait() const {

    /**
     *
     */

    // TODO: Make better destroy check
    u8 counter { 0 };
    while (_audio) {

        if (counter > 8ui8) {
            scheduler::thread::self::sleepFor(1000);
        }

        // Wait until empty
        scheduler::thread::self::yield();
        ++counter;
    }

    counter = 0;
    while (_ecs) {

        if (counter > 8ui8) {
            scheduler::thread::self::sleepFor(1000);
        }

        // Wait until empty
        scheduler::thread::self::yield();
        ++counter;
    }

    counter = 0;
    while (_graphics) {

        if (counter > 8ui8) {
            scheduler::thread::self::sleepFor(1000);
        }

        // Wait until empty
        scheduler::thread::self::yield();
        ++counter;
    }

    counter = 0;
    while (_network) {

        if (counter > 8ui8) {
            scheduler::thread::self::sleepFor(1000);
        }

        // Wait until empty
        scheduler::thread::self::yield();

        ++counter;
    }

    counter = 0;
    while (_physics) {

        if (counter > 8ui8) {
            scheduler::thread::self::sleepFor(1000);
        }

        // Wait until empty
        scheduler::thread::self::yield();
        ++counter;
    }

    counter = 0;
    while (_scene) {

        if (counter > 8ui8) {
            scheduler::thread::self::sleepFor(1000);
        }

        // Wait until empty
        scheduler::thread::self::yield();
        ++counter;
    }
}

ptr<void> SessionModules::audio() const {
    return _audio;
}

void SessionModules::setAudio(ptr<void> audio_) {
    _audio = audio_;
}

ptr<void> SessionModules::graphics() const {
    return _graphics;
}

void SessionModules::setGraphics(ptr<void> graphics_) {
    _graphics = graphics_;
}

ptr<void> SessionModules::network() const {
    return _network;
}

void SessionModules::setNetwork(ptr<void> network_) {
    _network = network_;
}

ptr<void> SessionModules::physics() const {
    return _physics;
}

void SessionModules::setPhysics(ptr<void> physics_) {
    _physics = physics_;
}

ptr<void> SessionModules::scene() const {
    return _scene;
}

void SessionModules::setScene(ptr<void> scene_) {
    _scene = scene_;
}

const ptr<engine::assets::AssetDatabase> SessionModules::assetDatabase() const {
    return _assetDatabase;
}

const ptr<engine::assets::AssetFactory> SessionModules::assetFactory() const {
    return _assetFactory;
}

const ptr<engine::assets::AssetSeeder> SessionModules::assetSeeder() const {
    return _assetSeeder;
}

const ptr<void> SessionModules::resourceManager() const {
    return _resourceManager;
}

ptr<void> SessionModules::ecs() const {
    return _ecs;
}

void SessionModules::setEcs(ptr<void> ecs_) {
    _ecs = ecs_;
}

const ptr<void> SessionModules::scheduler() const {
    return _scheduler;
}
