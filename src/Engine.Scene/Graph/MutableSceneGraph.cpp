#include "MutableSceneGraph.hpp"

using namespace ember::engine::scene;
using namespace ember;

SceneNodeFactory MutableSceneGraph::factory() const noexcept {
    return SceneNodeFactory { _storage };
}

bool MutableSceneGraph::push(mref<SceneNodeCreateData> data_) {
    const auto nodeFactory { factory() };
    return _root.get()->push(&data_, &nodeFactory);
}

bool MutableSceneGraph::push(ptr<SceneNode> node_) {
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
