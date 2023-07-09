#include "SubpassComponent.hpp"

#include <Engine.Common/Collection/DenseSet.hpp>

#include "SubpassMultiAccelComponent.hpp"
#include "SubpassSingleAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassComponent::SubpassComponent() noexcept :
    Component(),
    _mounted() {}

SubpassComponent::~SubpassComponent() noexcept = default;

void SubpassComponent::mount(nmpt<const Node> target_) {
    _mounted = target_;
}

void SubpassComponent::mounted(nmpt<const Component> mounted_) {

    if (mounted_->getClass()->isExactType<SubpassSingleAccelComponent>()) {
        _accel = &static_cast<cref<SubpassSingleAccelComponent>>(*mounted_);
        return;
    }

    if (mounted_->getClass()->isExactType<SubpassMultiAccelComponent>()) {
        _accel = &static_cast<cref<SubpassMultiAccelComponent>>(*mounted_);
        return;
    }

    Component::mounted(mounted_);
}

void SubpassComponent::unmount(nmpt<const Node> target_) {
    _mounted = nullptr;
}

void SubpassComponent::unmounted(nmpt<const Component> unmounted_) {

    if (unmounted_ == _accel) {
        _accel = nullptr;
    }

    Component::unmounted(unmounted_);
}

cref<DenseSet<Requirement>> SubpassComponent::getRequirements() const noexcept {
    return _required;
}

void SubpassComponent::setRequirements(mref<DenseSet<Requirement>> requirements_) {
    _required = _STD move(requirements_);
}

cref<DenseSet<Provision>> SubpassComponent::getProvided() const noexcept {
    return _provided;
}

void SubpassComponent::setProvided(mref<DenseSet<Provision>> provided_) {
    _provided = _STD move(provided_);
}
