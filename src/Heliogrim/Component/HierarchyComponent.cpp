#include "HierarchyComponent.hpp"

using namespace hg;

HierarchyComponent::HierarchyComponent(
	mref<component_type_id> typeId_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(std::move(typeId_)),
	_owner(std::move(owner_)),
	_parent(std::move(parent_)),
	_localTransform({}, {}, math::vec3 { 1.F }) {}

HierarchyComponent::HierarchyComponent(mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_) :
	InheritMeta(component_type_id { typeId }),
	_owner(std::move(owner_)),
	_parent(std::move(parent_)),
	_localTransform({}, {}, math::vec3 { 1.F }) {}

ptr<Actor> HierarchyComponent::getOwner() const noexcept {
	return _owner.cached;
}

ptr<Actor> HierarchyComponent::getRootActor() const noexcept {
	const auto root { getRootComponent() };
	return root ? root->getOwner() : nullptr;
}

ptr<HierarchyComponent> HierarchyComponent::getRootComponent() const noexcept {

	if (not _parent) {
		return const_cast<ptr<HierarchyComponent>>(this);
	}

	return _parent->getRootComponent();
}

ptr<HierarchyComponent> HierarchyComponent::getParentComponent() const noexcept {
	return _parent;
}

cref<math::Bounding> HierarchyComponent::getBoundaries() const noexcept {
	return _boundaries;
}

cref<Transform> HierarchyComponent::getLocalTransform() const noexcept {
	return _localTransform;
}

ref<Transform> HierarchyComponent::getLocalTransform() noexcept {
	return _localTransform;
}

math::Location HierarchyComponent::getUniverseLocation() const noexcept {
	if (not _parent) {
		return _localTransform.location();
	}
	return math::Location { _parent->getUniverseMatrix() * math::fvec4 { _localTransform.location().into(), 1.F } };
}

math::Rotator HierarchyComponent::getUniverseRotator() const noexcept {
	if (not _parent) {
		return _localTransform.rotator();
	}
	return math::Rotator::outerToInner(_localTransform.rotator(), _parent->getUniverseRotator());
}

math::fmat4 HierarchyComponent::getUniverseMatrix() const noexcept {
	if (not _parent) {
		return _localTransform.asMatrix();
	}

	return _parent->getUniverseMatrix() * _localTransform;
}
