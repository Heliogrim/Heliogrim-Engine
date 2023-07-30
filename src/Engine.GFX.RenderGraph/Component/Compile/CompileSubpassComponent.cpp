#include "CompileSubpassComponent.hpp"

#include "Subpass/CompileSubpassMaterialAccelComponent.hpp"
#include "Subpass/CompileSubpassMultiAccelComponent.hpp"
#include "Subpass/CompileSubpassSingleAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

CompileSubpassComponent::CompileSubpassComponent() noexcept :
    InheritMeta(),
    _mounted(),
    _accel() {}

CompileSubpassComponent::~CompileSubpassComponent() noexcept = default;

void CompileSubpassComponent::mount(nmpt<const Node> target_) {
    _mounted = target_;
}

void CompileSubpassComponent::mounted(nmpt<const Component> mounted_) {

    if (mounted_->getMetaClass()->exact<CompileSubpassSingleAccelComponent>()) {
        _accel = &static_cast<cref<CompileSubpassSingleAccelComponent>>(*mounted_);
    } else if (mounted_->getMetaClass()->exact<CompileSubpassMultiAccelComponent>()) {
        _accel = &static_cast<cref<CompileSubpassMultiAccelComponent>>(*mounted_);
    } else if (mounted_->getMetaClass()->exact<CompileSubpassMaterialAccelComponent>()) {
        _accel = &static_cast<cref<CompileSubpassMaterialAccelComponent>>(*mounted_);
    }

    Component::mounted(mounted_);
}

void CompileSubpassComponent::unmount(nmpt<const Node> target_) {
    _mounted = nullptr;
}

void CompileSubpassComponent::unmounted(nmpt<const Component> unmounted_) {

    if (unmounted_ == _accel) {
        _accel = nullptr;
    }

    Component::unmounted(unmounted_);
}

bool CompileSubpassComponent::addExpectedProvision(mref<Provision> expected_) {
    const auto result = _expectedProvision.insert(_STD move(expected_));
    return result.second;
}

bool CompileSubpassComponent::addExpectedRequirement(mref<Requirement> expected_) {
    const auto result = _expectedRequirement.insert(_STD move(expected_));
    return result.second;
}

DenseSet<Requirement> CompileSubpassComponent::expectedRequirement() const noexcept {
    return _expectedRequirement;
}

DenseSet<Provision> CompileSubpassComponent::expectedProvision() const noexcept {
    return _expectedProvision;
}
