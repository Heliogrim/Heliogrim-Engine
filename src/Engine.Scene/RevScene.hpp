#pragma once
#include "IRenderScene.hpp"
#include "RenderGraph.hpp"
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
        RenderGraph _renderGraph;

    public:
        [[nodiscard]] const ptr<RenderGraph> renderGraph() noexcept override;

    private:
        using cache_type = _STD pair<SceneTag, ptr<SceneNodeModel>>;

        Vector<cache_type> _cached;

    public:
        bool addNode(const ptr<SceneComponent> node_) override final;

        bool addNodeCached(const ptr<SceneComponent> node_) noexcept override final;

        bool removeNode(const ptr<SceneComponent> node_) override final;

        bool removeNodeCached(const ptr<SceneComponent> node_) noexcept override final;

    public:
        ptr<IComponentRegisterContext> registerContext() noexcept override;

        void add(const ptr<ActorComponent> component_) override;

        void add(cref<ComponentHierarchy> hierarchy_) override;
    };
}
