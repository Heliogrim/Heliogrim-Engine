#include "CameraComponent.hpp"

using namespace ember;

CameraComponent::CameraComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_) :
    ModelComponent(component_type_id { typeId }, _STD move(owner_), _STD move(parent_)),
    _fov(90.F),
    _aspectRation(16.F / 9.F),
    _lockedAspect(false),
    _visibleFrustum(false),
    _offset(math::vec3 { 0.F }),
    _projectMode(CameraProjectMode::ePerspective),
    _viewMode(CameraViewMode::eFirstPerson) {}

CameraComponent::~CameraComponent() = default;

float CameraComponent::getFov() const noexcept {
    return _fov;
}

void CameraComponent::setFov(const float fov_) {
    _fov = fov_;
}

float CameraComponent::getAspectRatio() const noexcept {
    return _aspectRation;
}

void CameraComponent::setAspectRatio(const float aspectRatio_) {
    _aspectRation = aspectRatio_;
}

bool CameraComponent::isAspectLocked() const noexcept {
    return _lockedAspect;
}

void CameraComponent::lockAspectRatio(const bool lock_) {
    _lockedAspect = lock_;
}

bool CameraComponent::isFrustumVisible() const noexcept {
    return _visibleFrustum;
}

void CameraComponent::setFrustumVisible(const bool visible_) {
    _visibleFrustum = visible_;
}

cref<math::vec3> CameraComponent::offset() const noexcept {
    return _offset;
}

void CameraComponent::setOffset(cref<math::vec3> offset_) {
    _offset = offset_;
}

CameraComponent::CameraProjectMode CameraComponent::getProjectMode() const noexcept {
    return _projectMode;
}

void CameraComponent::setProjectMode(const CameraProjectMode mode_) {
    _projectMode = mode_;
}

CameraComponent::CameraViewMode CameraComponent::getViewMode() const noexcept {
    return _viewMode;
}

void CameraComponent::setViewMode(const CameraViewMode mode_) {
    _viewMode = mode_;
}
