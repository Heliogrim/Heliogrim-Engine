#include "EmberModuleManager.hpp"

#include <Engine.ACS/Registry.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Database/AssetSeeder.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Network/Network.hpp>
#include <Engine.PFX/Physics.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scheduler/Scheduler.hpp>
#include <Engine.SFX/Audio.hpp>

using namespace ember::engine::session;
using namespace ember;

void EmberModuleManager::tidy() {

    /**
     *
     */
    stop();
    wait();

    /**
     *
     */
    delete _resourceManager;
    _resourceManager = nullptr;

    /**
     *
     */
    delete _assetSeeder;
    _assetSeeder = nullptr;

    delete _assetFactory;
    _assetFactory = nullptr;

    delete _assetDatabase;
    _assetDatabase = nullptr;

    /**
     *
     */
    Scheduler::destroy();
    _scheduler = nullptr;
}

void EmberModuleManager::setup() {

    /**
     * 
     */
    _scheduler = Scheduler::make();

    /**
     *
     */
    _assetDatabase = make_ptr<assets::AssetDatabase>();
    _assetFactory = make_ptr<assets::AssetFactory>(_assetDatabase);
    _assetSeeder = make_ptr<assets::AssetSeeder>();

    /**
     *
     */
    _resourceManager = make_ptr<ResourceManager>();

    /**
     *
     */
    _scheduler->setup(0);
    _resourceManager->setup();
    _resourceManager->schedule();
}

void EmberModuleManager::start() {

    /**
     *
     */
    _audio = make_ptr<Audio>();
    _graphics = make_ptr<Graphics>();
    _network = make_ptr<Network>();
    _physics = make_ptr<Physics>();
    _acsRegistry = make_ptr<acs::Registry>();

    /**
     *
     */
    _audio->setup();
    _graphics->setup();
    _network->setup();
    _physics->setup();

    _graphics->registerLoader();
    _graphics->registerImporter();

    _audio->schedule();
    _graphics->schedule();
    _network->schedule();
    _physics->schedule();

}

void EmberModuleManager::stop() {

    /**
     *
     */
    delete _audio;
    _audio = nullptr;

    delete _graphics;
    _graphics = nullptr;

    delete _network;
    _network = nullptr;

    delete _physics;
    _physics = nullptr;

    delete _acsRegistry;
    _acsRegistry = nullptr;
}

void EmberModuleManager::wait() {

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
    while (_acsRegistry) {

        if (counter > 8ui8) {
            scheduler::thread::self::sleepFor(1000);
        }

        // Wait until empty
        scheduler::thread::self::yield();
        ++counter;
    }
}
