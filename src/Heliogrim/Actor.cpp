#include "Actor.hpp"

#include <Engine.ACS/Registry.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/SessionState.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Core/WorldContext.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Scene/SceneBase.hpp>

#include "IComponentRegisterContext.hpp"

#include "ActorComponent.hpp"
#include "SceneComponent.hpp"
#include "Session.hpp"
#include "World.hpp"

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

ptr<ActorComponent> Actor::getRootComponent() const noexcept {
	return _rootComponent;
}

cref<Transform> Actor::getWorldTransform() const noexcept {
	if (_rootComponent != nullptr) {
		return _rootComponent->getWorldTransform();
	}

	// TODO: Cleanup dangling reference within engine frontend
	return Transform {};
}

cref<CompactSet<ptr<ActorComponent>>> Actor::getComponents() const noexcept {
	return _components;
}

void Actor::addComponent(const ptr<ActorComponent> component_) {
	/**
	 *
	 */
	if (component_->getParentComponent() == nullptr && !_rootComponent) {
		_rootComponent = component_;
	}

	#ifdef _DEBUG
	if (component_->getParentComponent() != nullptr) {
		assert(
			_components.contains(component_->getParentComponent()) &&
			"Failed to ensure acknowledged parent component."
		);
	}
	#endif

	_components.insert(component_);
}

void Actor::registerComponents(const ptr<IComponentRegisterContext> context_) {
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
		context_->add(entry);

		/**
		 *
		 */
		auto nextIter { components.erase(iter) };
		iter = std::move(nextIter != components.end() ? nextIter : components.begin());
	}
}

ptr<Actor> hg::CreateActor(cref<Session> session_) {
	const auto* session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };
	auto& registry { session->getState().getRegistry() };

	auto* actor { registry.createActor(session_.getActorInitializer()) };
	return actor;
}

ptr<Actor> hg::CreateActor(cref<World> activeWorld_) {
	const auto* world { activeWorld_.unwrap().get() };

	const auto engine { engine::Engine::getEngine() };
	const auto ctxs { engine->getWorldContexts() };// TODO: Check whether really want to copy the set

	const auto where = std::ranges::find_if(
		ctxs,
		[world](const auto& ctx_) {
			return ctx_->getCurrentWorld().get() == world;
		}
	);

	if (where == std::ranges::end(ctxs)) [[unlikely]]
	{
		return nullptr;
	}

	auto& session { (*where)->getOwner() };
	auto& registry { session.getState().getRegistry() };

	managed<void> dummy {};
	managed<void> alias { std::move(dummy), std::addressof(session) };

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

Future<ptr<Actor>> hg::CreateActor(cref<World> activeWorld_, async_t) {
	const auto* world { activeWorld_.unwrap().get() };

	const auto engine { engine::Engine::getEngine() };
	const auto ctxs { engine->getWorldContexts() };// TODO: Check whether really want to copy the set

	const auto where = std::ranges::find_if(
		ctxs,
		[world](const auto& ctx_) {
			return ctx_->getCurrentWorld().get() == world;
		}
	);

	auto* const session { where == std::ranges::end(ctxs) ? nullptr : std::addressof((*where)->getOwner()) };
	concurrent::promise<ptr<Actor>> p {
		[session]() -> ptr<Actor> {
			if (session) [[likely]]
			{
				auto& registry { session->getState().getRegistry() };

				managed<void> dummy {};
				managed<void> alias { std::move(dummy), session };

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

ptr<Actor> hg::CreateActor(const ptr<const ActorClass> class_, cref<Session> session_) noexcept {

	auto initializer { session_.getActorInitializer() };
	const auto* session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };
	auto& registry { session->getState().getRegistry() };

	auto* actor { registry.createActor(class_, initializer) };
	return actor;
}

ptr<Actor> hg::CreateActor(const ptr<const ActorClass> class_, cref<World> activeWorld_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CreateActor(
	const ptr<const ActorClass> class_,
	const ptr<SerializedActor> serialized_,
	cref<Session> session_
) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CreateActor(
	const ptr<const ActorClass> class_,
	const ptr<SerializedActor> serialized_,
	cref<World> activeWorld_
) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CloneActorInto(const ptr<Actor> actor_, cref<Session> session_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::CloneActorInto(const ptr<Actor> actor_, cref<World> activeWorld_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::SpawnActor(const ptr<const ActorClass> class_, cref<World> activeWorld_) noexcept {
	::hg::todo_panic();
}

Future<ptr<Actor>> hg::SpawnActor(
	const ptr<const ActorClass> class_,
	const ptr<SerializedActor> serialized_,
	cref<World> activeWorld_
) noexcept {
	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<ptr<Actor>> actor_, cref<Session> session_) noexcept {
	const auto* const session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };

	const auto scene = session->getWorldContext().getCurrentWorld()->getScene();
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

Future<bool> hg::Destroy(mref<ptr<Actor>> actor_, cref<World> activeWorld_) noexcept {
	const auto* world { activeWorld_.unwrap().get() };

	const auto engine { engine::Engine::getEngine() };
	const auto ctxs { engine->getWorldContexts() };// TODO: Check whether really want to copy the set

	const auto where = std::ranges::find_if(
		ctxs,
		[world](const auto& ctx_) {
			return ctx_->getCurrentWorld().get() == world;
		}
	);

	auto* const coreSession { where == std::ranges::end(ctxs) ? nullptr : std::addressof((*where)->getOwner()) };

	managed<void> dummy {};
	const Session session {
		managed<void> { std::move(dummy), coreSession }
	};

	return Destroy(std::move(actor_), session);
}
