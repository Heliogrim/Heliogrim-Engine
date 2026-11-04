#include "ActorClass.hpp"

#include <memory>

#include "../Actor.hpp"
#include "../../Component/HierarchyComponent.hpp"

using namespace hg;

void ActorClass::rebind(ref<HierarchyComponent> component_, const ptr<Actor> owner_, const ptr<HierarchyComponent> parent_) noexcept {
	component_._owner.cached = owner_;
	component_._parent = parent_;
}

void ActorClass::rebindRoot(ref<Actor> actor_, ref<HierarchyComponent> root_) noexcept {
	actor_._rootComponent = Some<ref<HierarchyComponent>>(root_);
}

void ActorClass::reseat(ref<Actor> dst_, ref<Actor> src_) noexcept {
	// The destination members hold byte copies that share the source's storage; treat them as raw memory.
	std::construct_at(std::addressof(dst_._label), std::move(src_._label));
	std::construct_at(std::addressof(dst_._components), std::move(src_._components));
	std::destroy_at(std::addressof(src_._label));
	std::destroy_at(std::addressof(src_._components));
}
