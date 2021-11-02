#include "MutableSceneGraph.hpp"

using namespace ember::engine::scene;
using namespace ember;

SceneNodeFactory MutableSceneGraph::factory() const noexcept {
    return SceneNodeFactory { _storage };
}

const ptr<SceneNode> MutableSceneGraph::push(mref<SceneNodeCreateData> data_) {

    const auto nodeFactory { factory() };
    auto result = nodeFactory.assembleRoot();

    result.body->payload() = data_.payload;
    result.body->transform() = data_.transformation;
    result.body->bounding() = data_.bounding;

    _root.get()->push(_STD move(result.head));
    return result.body;
}

const ptr<SceneNode> MutableSceneGraph::push(ptr<SceneNode> node_) {
    throw NotImplementedException();
}

bool MutableSceneGraph::insert(mref<SceneNodeCreateData> data_, const ptr<SceneNode> where_) {
    throw NotImplementedException();
}

bool MutableSceneGraph::insert(ptr<SceneNode> node_, const ptr<SceneNode> where_) {
    throw NotImplementedException();
}

bool MutableSceneGraph::pop(const ptr<SceneNode> node_) {
    throw NotImplementedException();
}

bool MutableSceneGraph::erase(ref<ptr<SceneNode>> subGraph_) {
    throw NotImplementedException();
}
