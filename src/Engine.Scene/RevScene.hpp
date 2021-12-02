#pragma once
#include "IRenderScene.hpp"
#include "Scene.hpp"
#include "Ember/IComponentRegisterContext.hpp"

namespace ember::engine::scene {

    class RevScene final :
        public Scene,
        public IRenderScene,
        public IComponentRegisterContext {
    public:
        using this_type = RevScene;
        using underlying_type = Scene;

    public:
        RevScene() noexcept;

        ~RevScene() override final;

    public:
        void update() override final;

    private:
        SceneGraph _graph;

    public:
        [[nodiscard]] const ptr<SceneGraph> renderGraph() noexcept override;

    private:
        Vector<ptr<ActorComponent>> _cachedNew;

    public:
        bool addNode(const ptr<ActorComponent> node_) override final;

        bool addNodeCached(const ptr<ActorComponent> node_) noexcept override final;

        bool removeNode(const ptr<ActorComponent> node_) override final;

        bool removeNodeCached(const ptr<ActorComponent> node_) noexcept override final;

    public:
        ptr<IComponentRegisterContext> registerContext() noexcept override;

        void add(const ptr<ActorComponent> component_) override;

        void add(cref<ComponentHierarchy> hierarchy_) override;

    private:
        uptr<World> _world;

    public:
        [[nodiscard]] ptr<World> getWorld() noexcept override final;
    };
}
