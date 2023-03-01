#include "Camera.hpp"

#include <Engine.Common/stdafx.h>
#include <Engine.Common/Math/Quaternion.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Common/Math/Convertion.hpp>

using namespace hg::engine::gfx;
using namespace hg;

/*
const static math::mat4 clip_matrix = (
    1.0F, 0.0F, 0.0F, 0.0F,
    0.0F, -1.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.5F, 0.5F,
    0.0F, 0.0F, 0.5F, 1.0F
);
*/

CameraViewMode Camera::viewMode() const noexcept {
    return _mode;
}

void Camera::setViewMode(const CameraViewMode mode_) {
    _mode = mode_;
}

float Camera::aspect() const noexcept {
    return _aspect;
}

void Camera::setAspect(const float ratio_) {
    _aspect = ratio_;
}

float Camera::fov() const noexcept {
    return _fov;
}

void Camera::setFov(const float fov_) {
    _fov = fov_;
}

float Camera::nearClip() const noexcept {
    return _znear;
}

float Camera::farClip() const noexcept {
    return _zfar;
}

math::mat4 Camera::projection() const noexcept {
    //return clip_matrix * _perspective;
    return _perspective;
}

const math::vec3& Camera::position() const noexcept {
    return _position;
}

void Camera::setPosition(const math::vec3& position_) {
    _position = position_;
    updateView();
}

cref<math::quaternion> Camera::rotation() const noexcept {
    return _rotation;
}

void Camera::setRotation(cref<math::quaternion> rotation_) {
    _rotation = rotation_;
    updateView();
}

void Camera::setRotation(cref<math::vec3> euler_) {
    setRotation(math::quaternion::euler(euler_));
}

void Camera::setPerspective(const float fov_, const float aspectRatio_, const float near_, const float far_) {
    _fov = fov_;
    _aspect = aspectRatio_;
    _znear = near_;
    _zfar = far_;

    _perspective = math::perspective(
        glm::radians(_fov),
        _aspect,
        _znear,
        _zfar
    );
}

const math::mat4& Camera::view() const noexcept {
    return _view;
}

void Camera::updateView() {

    math::mat4 translate { math::mat4::make_identity() };
    translate.translate(_position);

    const math::mat4 rotate { math::as<math::quaternion, math::mat4>(_rotation) };

    if (_mode == CameraViewMode::eFirstPerson) {
        _view = rotate * translate;
    } else {
        _view = translate * rotate;
    }
}

void Camera::lookAt(cref<math::vec3> target_) {
    // TODO: Calculate quaternion rotation from delta vectors
    _view = math::lookAt(_position, target_, math::vec3_up);
}
