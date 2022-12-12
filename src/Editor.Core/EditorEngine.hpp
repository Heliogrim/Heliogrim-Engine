#pragma once

#include <Engine.Core/Engine.hpp>
#include <Engine.Event/GlobalEventEmitter.hpp>

namespace ember::editor {
    class EditorEngine :
        public ::ember::engine::Engine {
    public:
        EditorEngine();

        ~EditorEngine() override;

    public:
        bool preInit() override;

        bool init() override;

        bool postInit() override;

        bool start() override;

        bool stop() override;

        bool shutdown() override;

        bool exit() override;

    private:
        uptr<engine::Assets> _assets;
        uptr<engine::Audio> _audio;
        uptr<engine::Graphics> _graphics;
        uptr<engine::Input> _input;
        uptr<engine::Network> _network;
        uptr<engine::Physics> _physics;

        uptr<engine::Platform> _platform;
        uptr<engine::ResourceManager> _resources;
        uptr<engine::Scheduler> _scheduler;

        GlobalEventEmitter _emitter;

    public:
        [[nodiscard]] non_owning_rptr<engine::Assets> getAssets() const noexcept override;

        [[nodiscard]] non_owning_rptr<engine::Audio> getAudio() const noexcept override;

        [[nodiscard]] non_owning_rptr<engine::Graphics> getGraphics() const noexcept override;

        [[nodiscard]] non_owning_rptr<engine::Input> getInput() const noexcept override;

        [[nodiscard]] non_owning_rptr<engine::Network> getNetwork() const noexcept override;

        [[nodiscard]] non_owning_rptr<engine::Physics> getPhysics() const noexcept override;

        [[nodiscard]] non_owning_rptr<engine::Platform> getPlatform() const noexcept override;

        [[nodiscard]] non_owning_rptr<engine::ResourceManager> getResources() const noexcept override;

        [[nodiscard]] non_owning_rptr<engine::Scheduler> getScheduler() const noexcept override;

    public:
        [[nodiscard]] ref<GlobalEventEmitter> getEmitter() const noexcept override;

    private:
        Vector<non_owning_rptr<engine::core::WorldContext>> _worldContexts;

    public:
        [[nodiscard]] Vector<non_owning_rptr<engine::core::WorldContext>> getWorldContexts() const noexcept override;

    public:
        void addWorld(cref<sptr<engine::core::World>> world_) override;

        void removeWorld(cref<sptr<engine::core::World>> world_) override;

    private:
        /**
         * EditorEngine specific member function and attributes
         */

        /**
         * While using a EditorEngine instance we expect to hold more than one game session at once, but there is a minimal setup to suggest.
         *  So we will be holding the editor's session and one played / edited / addressed session to differ between the systems and encapsulate resources.
         */
        uptr<engine::core::Session> _editorSession;
        uptr<engine::core::Session> _primaryGameSession;

    public:
        [[nodiscard]] const non_owning_rptr<engine::core::Session> getEditorSession() const noexcept;

        [[nodiscard]] const non_owning_rptr<engine::core::Session> getPrimaryGameSession() const noexcept;
    };
}
