#include "Universe.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Async/Await/Promise.hpp>
#include <Engine.Common/Move.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Core/Session.hpp>
#include <Engine.Core/Universe.hpp>
#include <Engine.Core/UniverseContext.hpp>
#include <Engine.Core/Event/UniverseAddedEvent.hpp>
#include <Engine.Core/Event/UniverseChangeEvent.hpp>
#include <Engine.Core/Event/UniverseRemoveEvent.hpp>
#include <Engine.Level/Level.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Scene.Game/GameScene.hpp>
#include <Engine.Scene/Scene.hpp>

#include "Level.hpp"
#include "Scene.hpp"
#include "../Async/Execute.hpp"
#include "../Core/Session.hpp"

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

bool Universe::addLevel(cref<Level> level_) {

	::hg::assertrt(valid() && level_.valid());

	const auto& universe = unwrap();
	const auto& level = level_.unwrap();

	universe->addLevel(clone(level));

	// TODO: Update internal scene.
	return true;
}

bool Universe::removeLevel(cref<Level> level_) {

	::hg::assertrt(valid() && level_.valid());

	const auto& universe = unwrap();
	const auto& level = level_.unwrap();

	universe->removeLevel(level);

	// TODO: Update internal scene.
	return true;
}

ptr<Actor> Universe::addActor(mref<VolatileActor<>> actor_) {

	auto* const universe = _internal.get();

	/**/
	auto* const actor = actor_.get();
	universe->getRootLevel()->addActor(::hg::move(actor_));

	/**/
	const auto scene = universe->getScene();
	const auto ctx = scene->registerContext();

	actor->registerComponents(*universe, *ctx.get());

	/**/

	return actor;
}

Opt<VolatileActor<>> Universe::removeActor(ptr<Actor> actor_) {

	const auto* const universe = _internal.get();

	/**/

	auto removed = universe->getRootLevel()->removeActor(actor_);

	if (removed == None) {
		return removed;
	}

	/**/

	const auto scene = universe->getScene();
	auto ctx = scene->registerContext();
	actor_->unregisterComponents(ctx.load<>());

	/**/

	return removed;
}

void Universe::dropActor(mref<ptr<Actor>> actor_) {

	const auto* const universe = _internal.get();
	auto removed = universe->getRootLevel()->removeActor(actor_);

	if (removed == None) {
		return;
	}

	/**/

	const auto scene = universe->getScene();
	auto ctx = scene->registerContext();
	actor_->unregisterComponents(ctx.load<>());

	/**/

	removed.reset();
}

Future<Universe> hg::CreateUniverse() noexcept {
	auto prom = hg::concurrent::promise<Universe>(
		[]() {
			auto defaultScene = make_uptr<engine::scene::GameScene>();
			defaultScene->prepare();

			auto defaultLevel = engine::core::make_root_like_level();

			auto universe {
				make_sptr<engine::core::Universe>(
					std::move(defaultScene),
					DenseSet<Arci<engine::core::Level>> { std::move(defaultLevel) }
				)
			};
			engine::Engine::getEngine()->addUniverse(clone(universe));

			return Universe { std::move(universe) };
		}
	);

	auto fut { prom.get() };
	execute(::hg::move(prom));

	return fut;
}

Universe hg::GetUniverse(cref<Session> session_) noexcept {
	const auto* session = session_.unwrap().get();
	const auto& context = session->getUniverseContext();
	const auto& universe = context.getCurrentUniverse();

	return Universe { clone(universe) };
}

Universe hg::GetUniverse() noexcept {
	return GetUniverse(GetSession());
}

Future<Universe> hg::GetUniverse(cref<AssetGuid> guid_) noexcept {
	::hg::todo_panic();
}

Future<bool> hg::Destroy(mref<Universe> universe_) {
	auto prom {
		hg::concurrent::promise<bool>(
			[universe_ = std::move(universe_)]() {
				auto universe = clone(universe_.unwrap());

				// TODO: Remove universe from every context / session where it might be used currently
				// TODO: Check whether we want to auto handle session's universe transition when engine propagated universe erase happens

				engine::Engine::getEngine()->removeUniverse(std::move(universe));

				return true;
			}
		)
	};

	auto fut { prom.get() };
	execute(::hg::move(prom));

	return fut;
}

void hg::SetUniverse(ref<Session> session_, cref<Universe> universe_) {
	const auto* const session = session_.unwrap().get();
	auto& context = session->getUniverseContext();
	const auto& universe = universe_.unwrap();

	if (context.getCurrentUniverse() == universe) {
		return;
	}

	context.setCurrentUniverse(clone(universe));
}

void hg::TransitionToUniverse(ref<Session> session_, cref<Universe> universe_) {
	// TODO:
	SetUniverse(session_, universe_);
}
