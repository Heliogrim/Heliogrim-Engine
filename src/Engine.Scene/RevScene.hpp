#pragma once
#include "IRenderScene.hpp"
#include "RenderGraph.hpp"
#include "Scene.hpp"
#include "Heliogrim/IComponentRegisterContext.hpp"

namespace hg::engine::scene {
    class RevScene final :
        public InheritMeta<RevScene, Scene,
            IRenderScene,
            IComponentRegisterContext> {
    public:
        using this_type = RevScene;
        using underlying_type = Scene;

        inline constexpr static type_id typeId { "RevScene"_typeId };

    public:
        RevScene() noexcept;

        ~RevScene() final;

    public:
        void update() override final;

    private:
        RenderSceneGraph _renderGraph;
        uptr<gfx::SceneResourcePool> _resourcePool;

    public:
        [[nodiscard]] const ptr<RenderSceneGraph> renderGraph() noexcept override;

        [[nodiscard]] nmpt<gfx::SceneResourcePool> getSceneResourcePool() const noexcept override;

    private:
        using cache_type = _STD pair<SceneTag, ptr<gfx::scene::SceneModel>>;

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
