#include "World.hpp"

#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Event/WorldAddedEvent.hpp>
#include <Engine.Core/Event/WorldChangeEvent.hpp>
#include <Engine.Core/Event/WorldRemoveEvent.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/WorldContext.hpp>
#include <Engine.Core/World.hpp>
#include <Engine.Scene.Game/GameScene.hpp>
#include <Engine.Scene/Scene.hpp>

#include "Scene.hpp"
#include "Session.hpp"

using namespace hg;

World::World() = default;

World::World(const managed<void> internal_) :
	_internal(internal_) {}

bool World::valid() const noexcept {
	return _internal.get() && _internal.use_count();
}

cref<managed<void>> World::unwrap() const noexcept {
	return _internal;
}

bool World::addLevel(const ptr<Level> level_) {
	::hg::todo_panic();
}

bool World::removeLevel(const ptr<Level> level_) {
	::hg::todo_panic();
}

bool World::addActor(const ptr<Actor> actor_) {
	const auto* const world { static_cast<ptr<engine::core::World>>(_internal.get()) };
	auto* const scene { world->getScene() };
	const auto ctx = scene->registerContext();
	actor_->registerComponents(ctx.get());

	/**
	 *
	 */
	return true;
}

bool World::removeActor(const ptr<Actor> actor_) {
	const auto* const world { static_cast<ptr<engine::core::World>>(_internal.get()) };
	auto* const scene { world->getScene() };
	const auto ctx = scene->registerContext();
	// actor_->unregisterComponents(ctx);

	/**
	 *
	 */
	return true;
}

Future<World> hg::CreateWorld() noexcept {
	auto prom {
		hg::concurrent::promise<World>(
			[]() {
				auto defaultScene = make_uptr<engine::scene::GameScene>();
				defaultScene->prepare();

				const auto world { make_sptr<engine::core::World>(std::move(defaultScene)) };

				engine::Engine::getEngine()->addWorld(world);

				return World { world };
			}
		)
	};

	auto fut { prom.get() };
	execute(prom);

	return fut;
}

World hg::GetWorld(cref<Session> session_) noexcept {
	const auto* session { static_cast<ptr<::hg::engine::core::Session>>(session_.unwrap().get()) };
	const auto* context { session->getWorldContext() };
	const auto world { context->getCurrentWorld() };

	return World { world };
}

World hg::GetWorld() noexcept {
	return GetWorld(GetSession());
}

Future<World> hg::GetWorld(cref<asset_guid> guid_) noexcept {
	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<World> world_) {
	auto prom {
		hg::concurrent::promise<bool>(
			[world_ = std::move(world_)]() {
				const auto world { std::static_pointer_cast<engine::core::World, void>(world_.unwrap()) };

				// TODO: Remove world from every context / session where it might be used currently
				// TODO: Check whether we want to auto handle session's world transition when engine propagated world erase happens

				engine::Engine::getEngine()->removeWorld(world);

				return true;
			}
		)
	};

	auto fut { prom.get() };
	execute(prom);

	return fut;
}

void hg::SetWorld(cref<Session> session_, cref<World> world_) {
	const auto* const session { static_cast<ptr<engine::core::Session>>(session_.unwrap().get()) };
	auto* const context { session->getWorldContext() };
	const sptr<engine::core::World> world { std::static_pointer_cast<engine::core::World, void>(world_.unwrap()) };

	if (context->getCurrentWorld() == world) {
		return;
	}

	context->setCurrentWorld(world);
}

void hg::TransitionToWorld(cref<Session> session_, cref<World> world_) {
	// TODO:
	SetWorld(session_, world_);
}
