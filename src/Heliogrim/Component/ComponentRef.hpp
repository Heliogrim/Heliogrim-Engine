#pragma once

#include <cstddef>
#include <Heliogrim/Actor/ActorRef.hpp>

namespace hg {
	/**
	 * Stable reference to a component inside an archetype slot: the owning actor's handle plus the byte offset from the actor.
	 */
	template <typename Type_>
	struct ComponentRef {
		engine::acs::Handle handle {};
		std::ptrdiff_t offset = 0;
		mutable ptr<Type_> cached = nullptr;
		mutable u64 epoch = ~0uLL;

		[[nodiscard]] Opt<ref<Type_>> get() const noexcept {
			if (epoch != engine::acs::Epoch::current()) {
				auto actor = ActorRef<Actor> { handle }.get();
				cached = actor.has_value()
					? reinterpret_cast<ptr<Type_>>(reinterpret_cast<std::byte*>(std::addressof(actor.value())) + offset)
					: nullptr;
				epoch = engine::acs::Epoch::current();
			}
			if (cached == nullptr) {
				return None;
			}
			return Some(*cached);
		}
	};
}
