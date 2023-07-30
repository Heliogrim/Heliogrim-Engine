#include "SubpassComponent.hpp"

#include <Engine.Common/Collection/DenseSet.hpp>

#include "Subpass/SubpassMultiAccelComponent.hpp"
#include "Subpass/SubpassSingleAccelComponent.hpp"
#include "Subpass/SubpassMaterialAccelComponent.hpp"

#include "Subpass/SubpassSceneMeshInvocationComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassComponent::SubpassComponent() noexcept :
    InheritMeta(),
    _mounted(),
    _accel(),
    _invoke() {}

SubpassComponent::~SubpassComponent() noexcept = default;

void SubpassComponent::mount(nmpt<const Node> target_) {
    _mounted = target_;
}

void SubpassComponent::mounted(nmpt<const Component> mounted_) {

    if (mounted_->getMetaClass()->exact<SubpassSingleAccelComponent>()) {
        _accel = &static_cast<cref<SubpassSingleAccelComponent>>(*mounted_);
    } else if (mounted_->getMetaClass()->exact<SubpassMultiAccelComponent>()) {
        _accel = &static_cast<cref<SubpassMultiAccelComponent>>(*mounted_);
    } else if (mounted_->getMetaClass()->exact<SubpassMaterialAccelComponent>()) {
        _accel = &static_cast<cref<SubpassMaterialAccelComponent>>(*mounted_);
    }

    if (mounted_->getMetaClass()->exact<SubpassSceneMeshInvocationComponent>()) {
        _invoke = &static_cast<cref<SubpassSceneMeshInvocationComponent>>(*mounted_);
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

DenseSet<Requirement> SubpassComponent::getRequirements() const noexcept {
    auto aggregated = _accel->aggregateImportedSymbols();

    auto tmp = DenseSet<Requirement>();
    tmp.reserve(aggregated.size());

    for (auto&& symbol : aggregated) {
        tmp.insert(_STD move(symbol));
    }

    return tmp;
}

DenseSet<Provision> SubpassComponent::getProvided() const noexcept {

    auto aggregated = _accel->aggregateExportedSymbols();

    auto tmp = DenseSet<Provision>();
    tmp.reserve(aggregated.size());

    for (auto&& symbol : aggregated) {
        tmp.insert(_STD move(symbol));
    }

    return tmp;
}
