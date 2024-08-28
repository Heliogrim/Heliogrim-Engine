#include "Actor.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.ACS/Registry.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/SessionState.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Scene/SceneBase.hpp>

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

actor_guid Actor::guid() const noexcept {
	return _guid;
}

void Actor::unsafe_set_guid(cref<actor_guid> guid_) {
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

bool hg::RegisterActorClass(ptr<const ActorClass> class_) noexcept {
	::hg::todo_panic();
}

ptr<Actor> hg::CreateActor(cref<Session> session_) {
	const auto* session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };
	auto& registry { session->getState().getRegistry() };

	auto* actor { registry.createActor(session_.getActorInitializer()) };
	return actor;
}

ptr<Actor> hg::CreateActor(cref<Universe> activeUniverse_) {
	const auto* universe { activeUniverse_.unwrap().get() };

	const auto engine { engine::Engine::getEngine() };
	const auto ctxs { engine->getUniverseContexts() };// TODO: Check whether really want to copy the set

	const auto where = std::ranges::find_if(
		ctxs,
		[universe](const auto& ctx_) {
			return ctx_->getCurrentUniverse().get() == universe;
		}
	);

	if (where == std::ranges::end(ctxs)) [[unlikely]]
	{
		return nullptr;
	}

	auto& session { (*where)->getOwner() };
	auto& registry { session.getState().getRegistry() };

	SharedPtr<void> dummy {};
	SharedPtr<::hg::engine::core::Session> alias { std::move(dummy), std::addressof(session) };

	auto* actor { registry.createActor(ActorInitializer { alias }) };
	return actor;
}

Future<ptr<Actor>> hg::CreateActor(cref<Session> session_, async_t) {
	const auto initializer { session_.getActorInitializer() };
	const auto* session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };

	concurrent::promise<ptr<Actor>> p {
		[session, initializer]() {
			auto& registry { session->getState().getRegistry() };

			auto* actor { registry.createActor(initializer) };
			return actor;
		}
	};

	auto f { p.get() };
	p();

	return Future { std::move(f) };
}

Future<ptr<Actor>> hg::CreateActor(cref<Universe> activeUniverse_, async_t) {
	const auto* universe { activeUniverse_.unwrap().get() };

	const auto engine { engine::Engine::getEngine() };
	const auto ctxs { engine->getUniverseContexts() };// TODO: Check whether really want to copy the set

	const auto where = std::ranges::find_if(
		ctxs,
		[universe](const auto& ctx_) {
			return ctx_->getCurrentUniverse().get() == universe;
		}
	);

	auto* const session { where == std::ranges::end(ctxs) ? nullptr : std::addressof((*where)->getOwner()) };
	concurrent::promise<ptr<Actor>> p {
		[session]() -> ptr<Actor> {
			if (session) [[likely]]
			{
				auto& registry { session->getState().getRegistry() };

				SharedPtr<void> dummy {};
				SharedPtr<::hg::engine::core::Session> alias { std::move(dummy), session };

				auto* actor { registry.createActor(ActorInitializer { alias }) };
				return actor;
			}

			return nullptr;
		}
	};

	auto f { p.get() };
	p();

	return Future { std::move(f) };
}

ptr<Actor> hg::CreateActor(ptr<const ActorClass> class_, cref<Session> session_) noexcept {

	auto initializer { session_.getActorInitializer() };
	const auto* session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };
	auto& registry { session->getState().getRegistry() };

	auto* actor { registry.createActor(class_, initializer) };
	return actor;
}

ptr<Actor> hg::CreateActor(ptr<const ActorClass> class_, cref<Universe> activeUniverse_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CreateActor(
	ptr<const ActorClass> class_,
	cref<SerializedActor> serialized_,
	cref<Session> session_
) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CreateActor(
	ptr<const ActorClass> class_,
	cref<SerializedActor> serialized_,
	cref<Universe> activeUniverse_
) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CloneActorInto(cref<Actor> actor_, cref<Session> session_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CloneActorInto(cref<Actor> actor_, cref<Universe> activeUniverse_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::SpawnActor(ptr<const ActorClass> class_, cref<Universe> activeUniverse_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::SpawnActor(
	ptr<const ActorClass> class_,
	cref<SerializedActor> serialized_,
	cref<Universe> activeUniverse_
) noexcept {
	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<ptr<Actor>> actor_, cref<Session> session_) noexcept {
	const auto* const session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };

	const auto scene = session->getUniverseContext().getCurrentUniverse()->getScene();
	auto& registry { session->getState().getRegistry() };

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

Future<bool> hg::Destroy(mref<ptr<Actor>> actor_, cref<Universe> activeUniverse_) noexcept {
	const auto* universe { activeUniverse_.unwrap().get() };

	const auto engine { engine::Engine::getEngine() };
	const auto ctxs { engine->getUniverseContexts() };// TODO: Check whether really want to copy the set

	const auto where = std::ranges::find_if(
		ctxs,
		[universe](const auto& ctx_) {
			return ctx_->getCurrentUniverse().get() == universe;
		}
	);

	auto* const coreSession { where == std::ranges::end(ctxs) ? nullptr : std::addressof((*where)->getOwner()) };

	SharedPtr<void> dummy {};
	const Session session {
		SharedPtr<::hg::engine::core::Session> { std::move(dummy), coreSession }
	};

	return Destroy(std::move(actor_), session);
}
