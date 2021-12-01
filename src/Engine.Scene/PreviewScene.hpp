#pragma once
#include "Scene.hpp"

namespace ember::engine::scene {

    class PreviewScene final :
        public Scene {
    public:
        using this_type = PreviewScene;
        using underlying_type = Scene;

    public:
        PreviewScene() noexcept;

        ~PreviewScene() override final;

    public:
        bool addNode(const ptr<ActorComponent> node_) override final;

        bool addNodeCached(const ptr<ActorComponent> node_) noexcept override final;

        bool removeNode(const ptr<ActorComponent> node_) override final;

        bool removeNodeCached(const ptr<ActorComponent> node_) noexcept override final;
    };
}
