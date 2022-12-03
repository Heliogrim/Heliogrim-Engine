#include "SceneViewEye.hpp"

#include <Engine.Common/Math/Convertion.hpp>

using namespace ember::engine::gfx::scene;
using namespace ember;

SceneViewEye::SceneViewEye() :
    _position(math::vec3 { 0.F }),
    _projection(math::mat4::make_identity()),
    _view(math::mat4::make_identity()) {}

void SceneViewEye::updateProjection(cref<math::mat4> projection_) {
    _projection = projection_;
}

void SceneViewEye::updateView(cref<math::vec3> origin_, cref<math::vec3> eulerRotation_) {
    updateView(origin_, math::quaternion::euler(eulerRotation_));
}

void SceneViewEye::updateView(cref<math::vec3> origin_, cref<math::quaternion> rotation_) {

    _position = origin_;

    const auto rotationMatrix { math::as<math::quaternion, math::mat4>(rotation_) };
    const auto translateMatrix { math::mat4::make_identity().translate(origin_) };

    constexpr bool fpc = true;// Using first person as default
    if constexpr (fpc) {
        _view = rotationMatrix * translateMatrix;
    } else {
        _view = translateMatrix * rotationMatrix;
    }
}

cref<math::vec3> SceneViewEye::getOrigin() const noexcept {
    return _position;
}

cref<math::mat4> SceneViewEye::getProjectionMatrix() const noexcept {
    return _projection;
}

void SceneViewEye::getInvProjectionMatrix(ref<math::mat4> matrix_) const noexcept {
    matrix_ = _projection.inverse();
}

cref<math::mat4> SceneViewEye::getViewMatrix() const noexcept {
    return _view;
}

void SceneViewEye::getInvViewMatrix(ref<math::mat4> matrix_) const noexcept {
    matrix_ = _view.inverse();
}

math::vec4 SceneViewEye::worldToView(const math::vec4 worldPosition_) {
    return _projection * _view /* * math::mat4::make_identity() */ * worldPosition_;
}

math::vec4 SceneViewEye::viewToWorld(const math::vec4 viewPosition_) {

    math::mat4 ip { math::mat4::make_identity() };
    math::mat4 iv { math::mat4::make_identity() };

    getInvProjectionMatrix(ip);
    getInvViewMatrix(iv);

    return iv * ip * viewPosition_;
}

math::vec2 SceneViewEye::worldToScreen(cref<math::vec4> worldPosition_) {

    // Affine projection of world position to view space
    const math::vec4 viewSpace = worldToView(worldPosition_);
    if (viewSpace.w <= 0.F) {
        return math::vec2 { -1.F, -1.F };
    }

    // Normalize to screen-space
    const auto normFactor { 1.F / viewSpace.w };
    const math::vec4 hs { viewSpace.x * normFactor, viewSpace.y * normFactor, viewSpace.z * normFactor, viewSpace.w };

    // Move from screen-space to ui space
    const math::vec2 norm {
        (hs.x / 2.F) + 0.5F,
        /* 1.F - */
        (hs.y / 2.F) + 0.5F// No need to change y direction, as vulkan by default uses top-down space
    };

    return norm;
}

void SceneViewEye::screenToWorld(
    cref<math::vec2> screenPosition_,
    ref<math::vec3> worldOrigin_,
    ref<math::vec3> worldDirection_
) {
    const math::vec2 screen {
        (screenPosition_.x - 0.5F) * 2.F,
        (screenPosition_.y - 0.5F) * 2.F// @see vulkan specs at SceneViewEye::worldToScreen
    };

    const math::vec4 eyeToFar { screen.x, screen.y, 1.F, 1.F };
    const math::vec4 eyeToNear { screen.x, screen.y, 0.001F, 1.F };

    math::mat4 ip { math::mat4::make_identity() };
    getInvProjectionMatrix(ip);

    auto farH { ip * eyeToFar };
    auto nearH { ip * eyeToNear };

    if (farH.w != 0.F) {
        const auto normFactor { 1.F / farH.w };
        farH = { farH.x * normFactor, farH.y * normFactor, farH.z * normFactor, 1.F };
    }

    if (nearH.w != 0.F) {
        const auto normFactor { 1.F / nearH.w };
        nearH = { nearH.x * normFactor, nearH.y * normFactor, nearH.z * normFactor, 1.F };
    }

    /**/

    auto viewSpaceDir { farH - nearH };
    viewSpaceDir.normalize();

    /**/

    math::mat4 iv { math::mat4::make_identity() };
    getInvViewMatrix(iv);

    const auto origin { iv * nearH };

    viewSpaceDir.w = 0.F;
    auto direction { iv * viewSpaceDir };
    direction.normalize();

    /**/

    worldOrigin_ = origin;
    worldDirection_ = direction;
}
