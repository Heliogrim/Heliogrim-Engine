#pragma once

#include <Engine.Session/Session.hpp>

#include "Graph/SceneGraph.hpp"
#include "SceneTag.hpp"
#include "NodeTypeRegistry.hpp";

/**
 * Forward Declaration
 */
namespace ember {
    class SceneComponent;
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

    protected:
        NodeTypeRegistry _nodeTypeRegistry;

    public:
        void setNodeType(const SceneTag tag_, cref<component_type_id> typeId_,
            cref<NodeTypeRegistryValue::model_type> model_);

    public:
        virtual bool addNode(const ptr<SceneComponent> node_) = 0;

        virtual bool addNodeCached(const ptr<SceneComponent> node_) noexcept = 0;

        virtual bool removeNode(const ptr<SceneComponent> node_) = 0;

        virtual bool removeNodeCached(const ptr<SceneComponent> node_) noexcept = 0;

    public:
        virtual ptr<IComponentRegisterContext> registerContext() noexcept = 0;

    public:
        [[nodiscard]] virtual ptr<World> getWorld() noexcept;
    };
}
