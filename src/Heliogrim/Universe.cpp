#include "Universe.hpp"

#include <Engine.Common/Concurrent/Promise.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Core/Event/UniverseAddedEvent.hpp>
#include <Engine.Core/Event/UniverseChangeEvent.hpp>
#include <Engine.Core/Event/UniverseRemoveEvent.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Scene.Game/GameScene.hpp>
#include <Engine.Scene/Scene.hpp>

#include "Scene.hpp"
#include "Session.hpp"

using namespace hg;

Universe::Universe() = default;

Universe::Universe(mref<SharedPtr<::hg::engine::core::Universe>> internal_) :
	_internal(std::move(internal_)) {}

bool Universe::valid() const noexcept {
	return _internal.get() && _internal.use_count();
}

cref<decltype(Universe::_internal)> Universe::unwrap() const noexcept {
	return _internal;
}

bool Universe::addLevel(const ptr<Level> level_) {
	::hg::todo_panic();
}

bool Universe::removeLevel(const ptr<Level> level_) {
	::hg::todo_panic();
}

bool Universe::addActor(const ptr<Actor> actor_) {
	const auto* const universe = _internal.get();
	const auto scene = universe->getScene();
	const auto ctx = scene->registerContext();
	actor_->registerComponents(ctx.get());

	/**
	 *
	 */
	return true;
}

bool Universe::removeActor(const ptr<Actor> actor_) {
	const auto* const universe = _internal.get();
	const auto scene = universe->getScene();
	const auto ctx = scene->registerContext();
	// actor_->unregisterComponents(ctx);

	/**
	 *
	 */
	return true;
}

Future<Universe> hg::CreateUniverse() noexcept {
	auto prom {
		hg::concurrent::promise<Universe>(
			[]() {
				auto defaultScene = make_uptr<engine::scene::GameScene>();
				defaultScene->prepare();

				auto universe { make_sptr<engine::core::Universe>(std::move(defaultScene)) };
				engine::Engine::getEngine()->addUniverse(universe);

				return Universe { std::move(universe) };
			}
		)
	};

	auto fut { prom.get() };
	execute(prom);

	return fut;
}

Universe hg::GetUniverse(cref<Session> session_) noexcept {
	const auto* session = session_.unwrap().get();
	const auto& context = session->getUniverseContext();
	const auto universe = context.getCurrentUniverse();

	return Universe { clone(universe) };
}

Universe hg::GetUniverse() noexcept {
	return GetUniverse(GetSession());
}

Future<Universe> hg::GetUniverse(cref<asset_guid> guid_) noexcept {
	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<Universe> universe_) {
	auto prom {
		hg::concurrent::promise<bool>(
			[universe_ = std::move(universe_)]() {
				const auto universe = clone(universe_.unwrap());

				// TODO: Remove universe from every context / session where it might be used currently
				// TODO: Check whether we want to auto handle session's universe transition when engine propagated universe erase happens

				engine::Engine::getEngine()->removeUniverse(universe);

				return true;
			}
		)
	};

	auto fut { prom.get() };
	execute(prom);

	return fut;
}

void hg::SetUniverse(cref<Session> session_, cref<Universe> universe_) {
	const auto* const session = session_.unwrap().get();
	auto& context = session->getUniverseContext();
	const auto& universe = universe_.unwrap();

	if (context.getCurrentUniverse() == universe) {
		return;
	}

	context.setCurrentUniverse(universe);
}

void hg::TransitionToUniverse(cref<Session> session_, cref<Universe> universe_) {
	// TODO:
	SetUniverse(session_, universe_);
}
