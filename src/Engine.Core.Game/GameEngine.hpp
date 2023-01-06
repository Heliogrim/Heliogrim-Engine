#pragma once

#include <Engine.Core/Engine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

namespace ember::engine {
    class GameEngine :
        public Engine {
    public:
        GameEngine();

        ~GameEngine() override;

    public:
        bool preInit() override;

        bool init() override;

        bool postInit() override;

        bool start() override;

        bool stop() override;

        bool shutdown() override;

        bool exit() override;

    private:
        uptr<Assets> _assets;
        uptr<Audio> _audio;
        uptr<Graphics> _graphics;
        uptr<Input> _input;
        uptr<Network> _network;
        uptr<Physics> _physics;

        uptr<Platform> _platform;
        uptr<ResourceManager> _resources;
        uptr<Scheduler> _scheduler;

        GlobalEventEmitter _emitter;

    public:
        [[nodiscard]] non_owning_rptr<Assets> getAssets() const noexcept override;

        [[nodiscard]] non_owning_rptr<Audio> getAudio() const noexcept override;

        [[nodiscard]] non_owning_rptr<Graphics> getGraphics() const noexcept override;

        [[nodiscard]] non_owning_rptr<Input> getInput() const noexcept override;

        [[nodiscard]] non_owning_rptr<Network> getNetwork() const noexcept override;

        [[nodiscard]] non_owning_rptr<Physics> getPhysics() const noexcept override;

        [[nodiscard]] non_owning_rptr<Platform> getPlatform() const noexcept override;

        [[nodiscard]] non_owning_rptr<ResourceManager> getResources() const noexcept override;

        [[nodiscard]] non_owning_rptr<Scheduler> getScheduler() const noexcept override;

        [[nodiscard]] ref<GlobalEventEmitter> getEmitter() const noexcept override;

    private:
        Vector<non_owning_rptr<core::WorldContext>> _worldContexts;

    public:
        [[nodiscard]] Vector<non_owning_rptr<core::WorldContext>> getWorldContexts() const noexcept override;

    public:
        void addWorld(cref<sptr<core::World>> world_) override;

        void removeWorld(cref<sptr<core::World>> world_) override;

    private:
        /**
         * GameEngine specific member functions and attributes
         */

        /**
         * As long as we use a GameEngine instance we can expect to have at least one major world
         *  We still have the possibility to execute multiple game sessions at once, but this should
         *  be manage explicitly to model the interaction of multiple worlds.
         */
        uptr<core::Session> _gameSession;

    public:
        [[nodiscard]] const non_owning_rptr<core::Session> getGameSession() const noexcept;

    private:
        void setupCorePipelines();
    };
}
