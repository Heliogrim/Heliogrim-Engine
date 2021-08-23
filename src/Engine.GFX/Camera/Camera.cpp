#include "Camera.hpp"

#include <Engine.Common/stdafx.h>
#include <Engine.Common/Math/Quaternion.hpp>

using namespace ember::engine::gfx;
using namespace ember;

/** The clip matrix */
static auto clip_matrix = math::mat4
(
    1.0F, 0.0F, 0.0F, 0.0F,
    0.0F, -1.0F, 0.0F, 0.0F,
    0.0F, 0.0F, 0.5F, 0.5F,
    0.0F, 0.0F, 0.5F, 1.0F
);

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

const math::vec3& Camera::lookAt() const noexcept {
    return _lookAt;
}

void Camera::setLookAt(const math::vec3& lookAt_) {
    _lookAt = lookAt_;
}

const math::vec3& Camera::position() const noexcept {
    return _position;
}

void Camera::setPosition(const math::vec3& position_) {
    _position = position_;
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

void Camera::update() {
    /**
     * View
     */
    const math::vec3 up { 0.F, -1.F, 0.F };
    _view = math::lookAt(_position, _lookAt, up);

    /**
     * Perspective
     */
    _perspective = math::perspective(
        glm::radians(_fov),
        _aspect,
        _znear,
        _zfar
    );
}
