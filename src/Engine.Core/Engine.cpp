#include <cassert>

import Engine.Core;
import Engine.Logging;

using namespace hg::engine::core;
using namespace hg::engine;
using namespace hg;

namespace {
	// Releases a drained batch from the pending count even when a remove listener throws, or the teardown wait never ends.
	struct PendingRelease {
		ref<std::atomic_size_t> count;
		size_t taken;

		~PendingRelease() {
			count.fetch_sub(taken);
		}
	};
}

Engine::Engine() {
	Engine::_cached.store(this, std::memory_order_release);
	Logger::setup();
}

Engine::~Engine() {
	tidy();
	// Teardown must have drained removals while the modules were alive.
	::hg::assertrt(not hasPendingRemovedUniverses());

	Logger::destroy();
	Engine::_cached.store(nullptr, std::memory_order_release);
}

constinit std::atomic<ptr<Engine>> Engine::_cached { nullptr };

nmpt<Engine> Engine::getEngine() noexcept {
	return _cached.load(std::memory_order_acquire);
}

void Engine::tidy() {
	if (getEngineState() == core::EngineState::eStarted) {
		stop();
		assert(getEngineState() < core::EngineState::eStarted);
	}

	if (getEngineState() == core::EngineState::eStopped || getEngineState() == core::EngineState::eInitialized) {
		shutdown();
		assert(getEngineState() < core::EngineState::eInitialized);
	}

	if (getEngineState() == core::EngineState::ePreInitialized) {
		exit();
		assert(getEngineState() < core::EngineState::ePreInitialized);
	}
}

bool Engine::setEngineState(core::EngineState state_) {
	const auto value { static_cast<u8>(state_) };
	const auto prev { _engineState.load() };

	if (value == prev) {
		return false;
	}

	if (prev < value) {
		u8 expect { static_cast<u8>(value - 1u) };
		return _engineState.compare_exchange_strong(expect, value);
	}

	u8 expect { prev };
	return _engineState.compare_exchange_strong(expect, value);
}

core::EngineState Engine::getEngineState() const noexcept {
	return static_cast<core::EngineState>(_engineState.load());
}

void Engine::enqueueRemovedUniverse(mref<SharedPtr<core::Universe>> universe_) {
	_removedPending.fetch_add(1uz);
	concurrent::SpinLock lck { _removedMtx };
	_removedUniverses.push_back(std::move(universe_));
}

void Engine::drainRemovedUniverses() {
	Vector<SharedPtr<core::Universe>> taken;

	/**/

	{
		concurrent::SpinLock lck { _removedMtx };
		taken.swap(_removedUniverses);
	}

	const PendingRelease release { _removedPending, taken.size() };

	// Serial on purpose: universe destruction mutates the shared actor registry unlocked.
	for (auto& universe : taken) {
		getEmitter().emit<UniverseRemoveEvent>(clone(universe));
		universe->teardown();
		universe.reset();
	}
}

bool Engine::hasPendingRemovedUniverses() const noexcept {
	return _removedPending.load() != 0uz;
}
