#include "Actor.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Universe.hpp>
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
	_rootComponent(nullptr),
	_components() {}

Actor::~Actor() noexcept = default;

ActorGuid Actor::guid() const noexcept {
	return _guid;
}

void Actor::unsafe_set_guid(cref<ActorGuid> guid_) {
	_guid = guid_;
}

ptr<HierarchyComponent> Actor::getRootComponent() const noexcept {
	return _rootComponent;
}

cref<Transform> Actor::getUniverseTransform() const noexcept {
	if (_rootComponent != nullptr) {
		return _rootComponent->getUniverseTransform();
	}

	// TODO: Cleanup dangling reference within engine frontend
	return Transform {};
}

cref<CompactSet<ptr<HierarchyComponent>>> Actor::getComponents() const noexcept {
	return _components;
}

void Actor::addComponent(ptr<HierarchyComponent> component_) {
	/**
	 *
	 */
	if (component_->getParentComponent() == nullptr && !_rootComponent) {
		_rootComponent = component_;
	}

	#ifdef _DEBUG
	if (component_->getParentComponent() != nullptr) {
		::hg::assertd(
			_components.contains(component_->getParentComponent()) &&
			"Failed to ensure acknowledged parent component."
		);
	}
	#endif

	_components.insert(component_);
}

void Actor::registerComponents(ref<IComponentRegisterContext> context_) {
	/**
	 * Guard for non rooted actor
	 */
	if (_rootComponent == nullptr) {
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
		const auto entry { *iter };
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

template <>
void VolatileActor<Actor>::destroy(ptr<Actor> obj_) {
	auto registry = engine::Engine::getEngine()->getActors()->getRegistry();

	const auto actorGuid = obj_->guid();
	for (const auto* const component : obj_->getComponents()) {
		::hg::assertd(component->getTypeId().data);
		registry->releaseActorComponent(actorGuid, component->getTypeId());
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

Future<bool> hg::Destroy(mref<ptr<Actor>> actor_, cref<Universe> universe_) noexcept {

	const auto* const universe = universe_.unwrap().get();
	const auto scene = universe->getScene();
	auto& registry = *engine::Engine::getEngine()->getActors()->getRegistry();

	const auto guid { actor_->guid() };
	// Warning: Check modify-on-read condition
	// Warning: Multiple unsafe paths!!!
	for (const auto* const component : actor_->getComponents()) {
		assert(component->getTypeId().data);

		if (IsType<SceneComponent>(*component)) {
			scene->remove(static_cast<const ptr<const SceneComponent>>(component));
		}

		registry.releaseActorComponent(guid, component->getTypeId());
	}

	/**/
	registry.destroyActor(std::move(actor_));

	/**/
	concurrent::promise<bool> p {
		[]() {
			return true;
		}
	};

	auto f = p.get();
	p();

	return Future { std::move(f) };
}
