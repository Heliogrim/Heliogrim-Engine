#pragma once
#include "SceneGraph.hpp"
#include "../Node/SceneNodeFactory.hpp"
#include "../Node/SceneNodeCreateData.hpp"

namespace ember::engine::scene {

    class MutableSceneGraph :
        public SceneGraph {
    public:
        using underlying_type = SceneGraph;

    protected:
        [[nodiscard]] SceneNodeFactory factory() const noexcept;

    public:
        _Success_(return) bool push(_Inout_ mref<SceneNodeCreateData> data_);

        _Success_(return) bool push(_Inout_ ptr<SceneNode> node_);

        _Success_(return) bool insert(_Inout_ mref<SceneNodeCreateData> data_, _In_ const ptr<SceneNode> where_);

        _Success_(return) bool insert(_Inout_ ptr<SceneNode> node_, _In_ const ptr<SceneNode> where_);

        _Success_(return) bool pop(_In_ const ptr<SceneNode> node_);

        _Success_(return) bool erase(_Inout_ ref<ptr<SceneNode>> subGraph_);
    };

}
