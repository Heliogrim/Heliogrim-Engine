#include "SceneViewEye.hpp"

#include <Engine.Common/Math/Convertion.hpp>
#include <Engine.Common/Math/Coordinates.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg;

SceneViewEye::SceneViewEye() :
    _location(),
    _projection(math::mat4::make_identity()),
    _view(math::mat4::make_identity()),
    _vkLocation(),
    _vkFlipY(true) {}

void SceneViewEye::updateProjection(cref<math::mat4> projection_) {

    _projection = projection_;

    #if TRUE
    if (_vkFlipY) {
        _projection[1][1] *= -1.F;
    }
    #endif
}

void SceneViewEye::updateView(cref<math::Location> origin_, cref<math::Rotator> rotation_) {

    /* Handle (vk) Rotation */

    math::mat4 rotationMatrix { math::mat4::make_identity() };

    #if FALSE
    rotationMatrix.rotate(rotation_.pitch() * (_vkFlipY ? -1.F : 1.F), math::vec3_right);
    #else
    rotationMatrix.rotate(rotation_.pitch(), math::vec3_right);
    #endif

    rotationMatrix.rotate(rotation_.yaw(), math::vec3_up);
    rotationMatrix.rotate(rotation_.roll(), math::vec3_forward);

    /* Handle (vk) Translation */

    _location = origin_;

    auto translation = origin_.fvec3();

    #if FALSE
    if (_vkFlipY) {
        translation.y *= -1.F;
    }
    #endif

    const auto translationMatrix = math::mat4::make_identity().translate(translation);

    /* Update View */

    constexpr bool fpc = true;// Using first person as default
    if constexpr (fpc) {
        _view = rotationMatrix * translationMatrix;
    } else {
        _view = translationMatrix * rotationMatrix;
    }

    /* Handle (vk) View Position */

    _vkLocation = math::Location(_view.inverse()[3]);
    //_vkLocation = math::Location { _location.operator math::fvec3() * math::fvec3 { -1.F, 1.F, -1.F } };
}

void SceneViewEye::updateView(cref<math::Location> origin_, cref<math::quaternion> rotation_) {
    updateView(origin_, math::as<math::Rotator>(rotation_));
}

cref<math::Location> SceneViewEye::getOrigin() const noexcept {
    return _location;
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

math::vec4 SceneViewEye::worldToView(const math::vec4 worldPosition_) const {
    return _projection * _view /* * math::mat4::make_identity() */ * worldPosition_;
}

math::vec4 SceneViewEye::viewToWorld(const math::vec4 viewPosition_) const {

    math::mat4 ip { math::mat4::make_identity() };
    math::mat4 iv { math::mat4::make_identity() };

    getInvProjectionMatrix(ip);
    getInvViewMatrix(iv);

    return iv * ip * viewPosition_;
}

math::vec2 SceneViewEye::worldToScreen(cref<math::vec4> worldPosition_) const {

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
) const {
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

void SceneViewEye::vkSetFlipY(bool flip_) {
    _vkFlipY = flip_;
    // TODO: Update View and Projection
}

cref<math::Location> SceneViewEye::getVkLocation() const noexcept {
    return _vkLocation;
}
