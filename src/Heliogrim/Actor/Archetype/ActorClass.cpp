#include "ActorClass.hpp"

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
