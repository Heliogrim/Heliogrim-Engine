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

#include "Archetype/ActorClass.hpp"
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
	_archetype(initializer_._archetype),
	_components() {

	if (_archetype != nullptr) {
		// The root is entry 0, constructed in front of the actor before this constructor runs.
		auto* const slot = reinterpret_cast<std::byte*>(this) - _archetype->desc().actorOffset;
		_rootComponent = Some<ref<HierarchyComponent>>(
			*std::launder(reinterpret_cast<ptr<HierarchyComponent>>(slot + _archetype->componentOffsets().front()))
		);
	}
}

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

void Actor::scheduleComponentRegister(const ptr<HierarchyComponent> component_) {
	_universe->getScene()->registerContext()->add(component_);
}

void Actor::scheduleComponentRemoval(mref<ptr<HierarchyComponent>> component_) {
	_universe->getScene()->registerContext()->remove(component_);
}

void Actor::scheduleComponentRelease(mref<ptr<LogicComponent>> component_) const {
	engine::Engine::getEngine()->getActors()->getRegistry()->releaseActorComponent(VolatileComponent<> { ::hg::move(component_) });
}

ComponentRange Actor::getComponents() const noexcept {
	if (_archetype == nullptr) {
		return ComponentRange::dynamic(_components);
	}

	const auto desc = _archetype->desc();
	return ComponentRange::archetype(
		const_cast<std::byte*>(reinterpret_cast<const std::byte*>(this)) - desc.actorOffset,
		_archetype->componentOffsets(),
		_archetype->startBitmap(),
		desc.actorOffset,
		_archetype->minAlign()
	);
}

void Actor::addComponent(mref<VolatileComponent<HierarchyComponent>> component_, Opt<ref<const AddComponentOptions>> options_) {

	// shortcut: sealed archetypes reject runtime components until de-optimization exists (spec 11).
	::hg::assertrt(_archetype == nullptr);

	if (not options_.is_null() && options_->autoRegister && not _universe.is_null()) {
		scheduleComponentRegister(component_.get());
	}

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
	 * Archetype entries are already ordered parent before child
	 */
	if (_archetype != nullptr) {
		for (auto* const entry : getComponents()) {
			context_.add(entry);
		}
		return;
	}

	/**
	 * Insert components ordered by hierarchy from parent to child (copy components)
	 */
	auto components { _components };

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

VolatileActor<> hg::CreateActor(ptr<const MetaClass> class_) noexcept {
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
