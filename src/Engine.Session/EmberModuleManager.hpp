#pragma once

#include <Engine.Common/Wrapper.hpp>

/**
 * Forward Declaration
 */
namespace ember::engine {
    class Audio;
    class Graphics;
    class Network;
    class Physics;
    class ResourceManager;
    class Scheduler;
}

namespace ember::engine::acs {
    class Registry;
}

namespace ember::engine::assets {
    class AssetDatabase;
    class AssetFactory;
    class AssetSeeder;
}

namespace ember::engine::session {

    class EmberModuleManager {
    public:
        using this_type = EmberModuleManager;

    public:
        EmberModuleManager() = default;

        ~EmberModuleManager() = default;

    public:
        void tidy();

        void setup();

    public:
        void start();

        void stop();

        void wait();

    private:
        ptr<Audio> _audio;

    public:
        [[nodiscard]] ptr<Audio> audio() const {
            return _audio;
        }

        void setAudio(const ptr<Audio> audio_) {
            _audio = audio_;
        }

    private:
        ptr<Graphics> _graphics;

    public:
        [[nodiscard]] ptr<Graphics> graphics() const {
            return _graphics;
        }

        void setGraphics(const ptr<Graphics> graphics_) {
            _graphics = graphics_;
        }

    private:
        ptr<Network> _network;

    public:
        [[nodiscard]] ptr<Network> network() const {
            return _network;
        }

        void setNetwork(const ptr<Network> network_) {
            _network = network_;
        }

    private:
        ptr<Physics> _physics;

    public:
        [[nodiscard]] ptr<Physics> physics() const {
            return _physics;
        }

        void setPhysics(const ptr<Physics> physics_) {
            _physics = physics_;
        }

    private:
        ptr<ResourceManager> _resourceManager;

    public:
        [[nodiscard]] ptr<ResourceManager> resourceManager() const {
            return _resourceManager;
        }

        void setResourceManager(const ptr<ResourceManager> resourceManager_) {
            _resourceManager = resourceManager_;
        }

    private:
        ptr<Scheduler> _scheduler;

    public:
        [[nodiscard]] ptr<Scheduler> scheduler() const {
            return _scheduler;
        }

        void setScheduler(const ptr<Scheduler> scheduler_) {
            _scheduler = scheduler_;
        }

    private:
        ptr<acs::Registry> _acsRegistry;

    public:
        [[nodiscard]] ptr<acs::Registry> acsRegistry() const {
            return _acsRegistry;
        }

        void setAcsRegistry(const ptr<acs::Registry> acsRegistry_) {
            _acsRegistry = acsRegistry_;
        }

    private:
        ptr<assets::AssetDatabase> _assetDatabase;

    public:
        [[nodiscard]] ptr<assets::AssetDatabase> assetDatabase() const {
            return _assetDatabase;
        }

        void setAssetDatabase(const ptr<assets::AssetDatabase> assetDatabase_) {
            _assetDatabase = assetDatabase_;
        }

    private:
        ptr<assets::AssetFactory> _assetFactory;

    public:
        [[nodiscard]] ptr<assets::AssetFactory> assetFactory() const {
            return _assetFactory;
        }

        void setAssetFactory(const ptr<assets::AssetFactory> assetFactory_) {
            _assetFactory = assetFactory_;
        }

    private:
        ptr<assets::AssetSeeder> _assetSeeder;

    public:
        [[nodiscard]] ptr<assets::AssetSeeder> assetSeeder() const {
            return _assetSeeder;
        }

        void setAssetSeeder(const ptr<assets::AssetSeeder> assetSeeder_) {
            _assetSeeder = assetSeeder_;
        }
    };

}
