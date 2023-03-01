#include "SceneNodeVersion.hpp"

using namespace hg::engine::scene;
using namespace hg;

SceneNodeVersion::SceneNodeVersion() noexcept :
    data(invalid_node_version::bit_sequence) {}

SceneNodeVersion::SceneNodeVersion(invalid_node_version) noexcept :
    data(invalid_node_version::bit_sequence) {}

SceneNodeVersion::SceneNodeVersion(cref<decltype(data)> data_) noexcept :
    data(data_) {}

SceneNodeVersion::SceneNodeVersion(cref<SceneNodeVersion> other_) noexcept = default;

SceneNodeVersion::SceneNodeVersion(mref<SceneNodeVersion> other_) noexcept :
    data(_STD exchange(other_.data, invalid_node_version::bit_sequence)) {}

ref<SceneNodeVersion> SceneNodeVersion::operator=(cref<SceneNodeVersion> other_) noexcept {

    if (this != _STD addressof(other_)) {
        data = other_.data;
    }

    return *this;
}

ref<SceneNodeVersion> SceneNodeVersion::operator=(mref<SceneNodeVersion> other_) noexcept {

    if (this != _STD addressof(other_)) {
        data = _STD exchange(other_.data, invalid_node_version::bit_sequence);
    }

    return *this;
}
