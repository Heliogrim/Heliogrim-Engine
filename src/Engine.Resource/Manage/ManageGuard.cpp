#include "ManageGuard.hpp"

#include "ResourceBase.hpp"

using namespace ember::engine::resource;
using namespace ember;

constexpr ManageGuard::ManageGuard() noexcept :
    _resource(nullptr),
    _ownedFlags(ResourceUsageFlag::eNone) {}

ManageGuard::ManageGuard(const ptr<resource_type> resource_) :
    _resource(resource_),
    _ownedFlags(ResourceUsageFlag::eNone) {
    acquire();
}

ManageGuard::ManageGuard(const ptr<resource_type> resource_, const ResourceUsageFlags flags_) :
    _resource(resource_),
    _ownedFlags(ResourceUsageFlag::eNone) {
    acquire(flags_);
}

ManageGuard::ManageGuard(
    const ptr<resource_type> resource_,
    const std::defer_lock_t
) :
    _resource(resource_),
    _ownedFlags(ResourceUsageFlag::eNone) {}

ManageGuard::ManageGuard(mref<this_type> other_) noexcept :
    _resource(_STD exchange(other_._resource, nullptr)),
    _ownedFlags(_STD exchange(other_._ownedFlags, ResourceUsageFlag::eNone)) {}

ManageGuard::~ManageGuard() {
    if (_ownedFlags != ResourceUsageFlag::eNone) {
        release();
    }
}

ref<ManageGuard::this_type> ManageGuard::operator=(mref<this_type> other_) noexcept {

    if (_STD addressof(other_) != this) {

        if (_ownedFlags != ResourceUsageFlag::eNone) {
            release();
        }

        _resource = _STD exchange(other_._resource, nullptr);
        _ownedFlags = _STD exchange(other_._ownedFlags, ResourceUsageFlag::eNone);
    }

    return *this;
}

bool ManageGuard::try_acquire(const ResourceUsageFlags flags_) noexcept {

    ManageGuard guard {};
    if (_resource->try_acquire(guard, flags_)) {
        (*this) = _STD move(guard);
        return true;
    }

    return false;
}

ref<ManageGuard::this_type> ManageGuard::acquire(const ResourceUsageFlags flags_) {
    (*this) = _STD move(_resource->acquire(flags_));
    return *this;
}

ref<ManageGuard::this_type> ManageGuard::acquire(const ptr<resource_type> resource_, const ResourceUsageFlags flags_) {
    (*this) = _STD move(resource_->acquire(flags_));
    return *this;
}

ref<ManageGuard::this_type> ManageGuard::release() {
    _resource->release(_ownedFlags);
    _ownedFlags = ResourceUsageFlag::eNone;
    return *this;
}

const ptr<ManageGuard::resource_type> ManageGuard::reset(const ptr<resource_type> next_) {

    if (owns()) {
        release();
    }

    return _STD exchange(_resource, next_);
}

const ptr<ManageGuard::resource_type> ManageGuard::reset(_STD nullptr_t) {

    if (owns()) {
        release();
    }

    return _STD exchange(_resource, nullptr);
}

bool ManageGuard::empty() const noexcept {
    return _resource == nullptr;
}

bool ManageGuard::owns() const noexcept {
    return _ownedFlags != ResourceUsageFlag::eNone;
}

ResourceUsageFlags ManageGuard::owned_flags() const noexcept {
    return _ownedFlags;
}

ManageGuard::operator bool() const noexcept {
    return not empty() && owns();
}

const non_owning_rptr<const ManageGuard::value_type> ManageGuard::imm() const noexcept {
    if (_ownedFlags != ResourceUsageFlag::eNone) {
        return _resource->value();
    }
    return nullptr;
}

const non_owning_rptr<const ManageGuard::value_type> ManageGuard::imm() noexcept {
    if (_ownedFlags != ResourceUsageFlag::eNone) {
        return _resource->value();
    }
    return nullptr;
}

const ptr<ManageGuard::value_type> ManageGuard::mut() const noexcept {
    if (_ownedFlags & ResourceUsageFlag::eWrite) {
        return _resource->value();
    }
    return nullptr;
}

const ptr<ManageGuard::value_type> ManageGuard::mut() noexcept {
    if (_ownedFlags & ResourceUsageFlag::eWrite) {
        return _resource->value();
    }
    return nullptr;
}
