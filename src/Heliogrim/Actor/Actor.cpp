#include "Actor.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Scene/SceneBase.hpp>

#include "../ActorInitializer.hpp"
#include "../IComponentRegisterContext.hpp"
#include "../Component/HierarchyComponent.hpp"
#include "../Component/SceneComponent.hpp"
#include "../Core/Session.hpp"
#include "../Universe/Universe.hpp"

using namespace hg;

Actor::Actor(cref<ActorInitializer> initializer_) noexcept :
	_guid(initializer_._guid),
	_label(),
	_universe(None),
	_rootComponent(None),
	_components() {}

Actor::~Actor() noexcept = default;

ActorGuid Actor::guid() const noexcept {
	return _guid;
}

ref<const ActorLabel> Actor::label() const noexcept {
	return _label;
}

void Actor::setLabel(mref<ActorLabel> label_) {
	_label = ::hg::move(label_);
}

void Actor::unsafe_set_guid(cref<ActorGuid> guid_) {
	_guid = guid_;
}

Opt<ref<engine::core::Universe>> Actor::getUniverse() const noexcept {
	return _universe;
}

Opt<ref<engine::core::Level>> Actor::getLevel() const noexcept {
	return _level;
}

Opt<ref<HierarchyComponent>> Actor::getRootComponent() const noexcept {
	return _rootComponent;
}

void Actor::setOwnerLevel(mref<Opt<ref<engine::core::Level>>> owner_) {
	_level = ::hg::move(owner_);
}

void Actor::scheduleComponentRemoval(mref<ptr<HierarchyComponent>> component_) {
	_universe->getScene()->registerContext()->remove(component_);
}

void Actor::scheduleComponentRelease(mref<ptr<LogicComponent>> component_) const {
	engine::Engine::getEngine()->getActors()->getRegistry()->releaseActorComponent(VolatileComponent<> { ::hg::move(component_) });
}

cref<CompactSet<ptr<HierarchyComponent>>> Actor::getComponents() const noexcept {
	return _components;
}

void Actor::addComponent(mref<VolatileComponent<HierarchyComponent>> component_, Opt<ref<const AddComponentOptions>> options_) {

	if (options_.is_null() || options_->parent.is_null()) {
		::hg::assertrt(component_->getParentComponent() == nullptr && _rootComponent.is_null());
		auto* const component = ::hg::move(component_).release();
		_rootComponent = Some<ref<HierarchyComponent>>(*component);
		_components.insert(component);
		return;
	}

	::hg::assertrt(component_->getParentComponent() == options_->parent.address());
	::hg::assertd(
		/* Check: Failed to ensure acknowledged parent component.*/
		_components.contains(component_->getParentComponent())
	);

	_components.insert(::hg::move(component_).release());
}

void Actor::registerComponents(ref<engine::core::Universe> universe_, ref<IComponentRegisterContext> context_) {

	if (_universe != None && _universe.address() != std::addressof(universe_)) [[unlikely]] {
		IM_CORE_WARN("Tried to register actor components in a mismatching scene.");
		return;
	}

	_universe = Some<ref<engine::core::Universe>>(universe_);

	/**
	 * Guard for non rooted actor
	 */
	if (not _rootComponent.has_value()) {
		return;
	}

	/**
	 * Insert components ordered by hierarchy from parent to child (copy components)
	 */
	auto components { getComponents() };

	auto iter { components.begin() };
	while (!components.empty()) {
		/**
		 *
		 */
		auto* const entry = *iter;
		const auto parent { entry->getParentComponent() };

		/**
		 *
		 */
		auto prevIter = components.find(parent);
		if (prevIter != components.end()) {
			iter = std::move(prevIter);
			continue;
		}

		/**
		 * Insert component
		 */
		context_.add(entry);

		/**
		 *
		 */
		auto nextIter { components.erase(iter) };
		iter = std::move(nextIter != components.end() ? nextIter : components.begin());
	}
}

void Actor::unregisterComponents(ref<IComponentRegisterContext> context_) {

	// TODO: Check whether we need to remove components in (hierarchical) order
	for (const auto& entry : getComponents()) {
		context_.remove(entry);
	}

	_universe = None;
}

template <>
void VolatileActor<Actor>::destroy(ptr<Actor> obj_) {

	::hg::assertd(obj_->getUniverse().is_null());
	auto registry = engine::Engine::getEngine()->getActors()->getRegistry();

	for (auto* const component : obj_->getComponents()) {
		::hg::assertd(component->getTypeId().data);
		registry->releaseActorComponent(VolatileComponent<> { component });
	}
	registry->destroyActor(std::move(obj_));
}

bool hg::RegisterActorClass(ptr<const ActorClass> class_) noexcept {
	::hg::todo_panic();
}

VolatileActor<> hg::CreateActor() {
	auto& registry = *engine::Engine::getEngine()->getActors()->getRegistry();
	return VolatileActor<> { registry.createActor(ActorInitializer { registry }) };
}

Future<VolatileActor<>> hg::CreateActor(async_t) {
	concurrent::promise<VolatileActor<>> p {
		[]() -> VolatileActor<> {
			auto& registry = *engine::Engine::getEngine()->getActors()->getRegistry();
			return VolatileActor<> { registry.createActor(ActorInitializer { registry }) };
		}
	};

	auto f { p.get() };
	p();

	return Future { std::move(f) };
}

VolatileActor<> hg::CreateActor(ptr<const ActorClass> class_) noexcept {
	auto& registry = *engine::Engine::getEngine()->getActors()->getRegistry();
	return VolatileActor<> { registry.createActor(class_, ActorInitializer { registry }) };
}

Future<VolatileActor<>> hg::CreateActor(
	ptr<const ActorClass> class_,
	cref<SerializedActor> serialized_
) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CloneActorInto(cref<Actor> actor_, cref<Universe> universe_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::SpawnActor(ptr<const ActorClass> class_, cref<Universe> universe_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::SpawnActor(
	ptr<const ActorClass> class_,
	cref<SerializedActor> serialized_,
	cref<Universe> universe_
) noexcept {
	::hg::todo_panic();
}

bool hg::Destroy(mref<VolatileActor<>> actor_) noexcept {
	std::move(actor_).reset();
	return true;
}
