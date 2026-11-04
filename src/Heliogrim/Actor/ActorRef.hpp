#pragma once

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Handle/Epoch.hpp>
#include <Engine.ACS/Handle/Handle.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Core/Engine.hpp>

namespace hg {
	class Actor;

	/**
	 * Stable reference to an archetype actor; re-resolves its handle once per relocation epoch.
	 */
	template <typename Type_ = Actor>
	struct ActorRef {
		engine::acs::Handle handle {};
		mutable ptr<Type_> cached = nullptr;
		mutable u64 epoch = ~0uLL;

		[[nodiscard]] Opt<ref<Type_>> get() const noexcept {
			// A detached handle goes stale on mount by design; the mounted actor gets a new handle.
			if (handle.space() == engine::acs::detachedSpace) {
				return None;
			}
			if (epoch != engine::acs::Epoch::current()) {
				const auto core = engine::Engine::getEngine();
				cached = core ? static_cast<ptr<Type_>>(core->getActors()->getRegistry()->resolve(handle)) : nullptr;
				epoch = engine::acs::Epoch::current();
			}
			if (cached == nullptr) {
				return None;
			}
			return Some(*cached);
		}
	};
}
