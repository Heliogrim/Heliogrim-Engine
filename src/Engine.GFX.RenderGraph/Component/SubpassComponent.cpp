#include "SubpassComponent.hpp"

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

cref<Set<Requirement>> SubpassComponent::getRequirements() const noexcept {
    return _required;
}

void SubpassComponent::setRequirements(mref<Set<Requirement>> requirements_) {
    _required = _STD move(requirements_);
}

cref<Set<Provision>> SubpassComponent::getProvided() const noexcept {
    return _provided;
}

void SubpassComponent::setProvided(mref<Set<Provision>> provided_) {
    _provided = _STD move(provided_);
}
