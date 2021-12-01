#pragma once

#include <Engine.Session/Session.hpp>

#include "Graph/SceneGraph.hpp"
#include "Graph/SceneGraphTag.hpp"

/**
 * Forward Declaration
 */
namespace ember {
    class ActorComponent;
    class IComponentRegisterContext;
    class World;
}

namespace ember::engine::scene {

    class __declspec(novtable) Scene {
    public:
        using this_type = Scene;

    protected:
        Scene() noexcept;

    public:
        virtual ~Scene();

    public:
        ref<Scene> operator=(mref<Scene>) noexcept = delete;

        ref<Scene> operator=(cref<Scene>) = delete;

    public:
        virtual void update();

    public:
        virtual bool addNode(const ptr<ActorComponent> node_) = 0;

        virtual bool addNodeCached(const ptr<ActorComponent> node_) noexcept = 0;

        virtual bool removeNode(const ptr<ActorComponent> node_) = 0;

        virtual bool removeNodeCached(const ptr<ActorComponent> node_) noexcept = 0;

    public:
        virtual ptr<IComponentRegisterContext> registerContext() noexcept = 0;

    public:
        [[nodiscard]] virtual ptr<World> getWorld() noexcept;
    };

}
