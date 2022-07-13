#include "SceneNodePath.hpp"

using namespace ember::engine::scene;
using namespace ember;

SceneNodePath::decision_type SceneNodePath::cur() const noexcept {
    return static_cast<decision_type>(path & decision_mask);
}

SceneNodePath::decision_type SceneNodePath::pop() {

    const auto decision { static_cast<decision_type>(path & decision_mask) };
    path >>= decision_size;

    return decision;
}

ref<SceneNodePath> SceneNodePath::push(const decision_type decision_) {

    // Pushing decision onto stack
    path <<= decision_size;
    path |= static_cast<decision_type>(decision_) & decision_mask;

    // Marking layer as occupied
    path |= decision_occupied_mask;

    return *this;
}

SceneNodePath::decision_type SceneNodePath::peek() const noexcept {
    return static_cast<decision_type>((path >> decision_size) & decision_mask);
}

SceneNodePath SceneNodePath::inversed() const noexcept {

    SceneNodePath inv {};

    // Move over current path by decision wide words
    for (_STD decay_t<decltype(decision_size)> offset { 0u }; (path >> offset) & decision_occupied_mask; offset +=
         decision_size) {
        // Masking is done by push itself
        inv.push(static_cast<decision_type>(path >> offset));
    }

    return inv;
}
