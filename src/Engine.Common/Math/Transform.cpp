#include "Transform.hpp"

using namespace ember::math;
using namespace ember;

ref<Transform> Transform::operator=(mref<Transform> other_) noexcept {

    if (_STD addressof(other_) != this) {
        _position = _STD move(other_._position);
        _rotation = _STD move(other_._rotation);
        _scale = _STD move(other_._scale);
    }

    return *this;
}
