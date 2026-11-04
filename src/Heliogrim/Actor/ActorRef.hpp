#pragma once

#include <Engine.ACS/ActorModule.hpp>
#include <Engine.ACS/Registry.hpp>
#include <Engine.ACS/Handle/Epoch.hpp>
#include <Engine.ACS/Handle/Handle.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Core/Engine.hpp>

namespace hg::archetype_detail {
	struct RefAccess;
}

namespace hg {
	class Actor;
	class Level;
	class Universe;

	template <typename>
	struct ComponentRef;

	/**
	 * Stable reference to an archetype actor; re-resolves its handle once per relocation epoch.
	 *
	 * A default constructed ref is empty and never resolves; valid refs only come from `of` or the engine.
	 */
	template <typename Type_ = Actor>
	struct ActorRef {
		template <typename>
		friend struct ActorRef;
		template <typename>
		friend struct ComponentRef;
		friend struct ::hg::archetype_detail::RefAccess;
		friend class ::hg::Level;
		friend class ::hg::Universe;

	public:
		constexpr ActorRef() noexcept = default;

	private:
		constexpr explicit ActorRef(const engine::acs::Handle handle_) noexcept :
			_handle(handle_) {}

	private:
		engine::acs::Handle _handle {};
		mutable ptr<Type_> _cached = nullptr;
		mutable u64 _epoch = ~0uLL;

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
