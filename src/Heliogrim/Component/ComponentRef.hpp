#pragma once

#include <cstddef>
#include <Heliogrim/Actor/ActorRef.hpp>

namespace hg {
	/**
	 * Stable reference to a component inside an archetype slot: the owning actor's handle plus the byte offset from the actor.
	 *
	 * A default constructed ref is empty and never resolves; valid refs only come from `of` or the engine.
	 */
	template <typename Type_>
	struct ComponentRef {
	private:
		friend struct ::hg::archetype_detail::RefAccess;

	public:
		constexpr ComponentRef() noexcept = default;

	private:
		engine::acs::Handle _handle {};
		std::ptrdiff_t _offset = 0;
		mutable ptr<Type_> _cached = nullptr;
		mutable u64 _epoch = ~0uLL;

	public:
		/**
		 * @returns a ref to a component of an archetype actor, None for nullptr, an unowned component or a non-archetype owner
		 */
		[[nodiscard]] static Opt<ComponentRef> of(const ptr<const Type_> component_) {
			if (component_ == nullptr) {
				return None;
			}
			const ptr<const Actor> owner = component_->getOwner();
			auto actor = ActorRef<Actor>::of(owner);
			if (not actor.has_value()) {
				return None;
			}
			ComponentRef result {};
			result._handle = actor.value()._handle;
			result._offset = reinterpret_cast<const std::byte*>(component_) - reinterpret_cast<const std::byte*>(owner);
			return Some(result);
		}

		[[nodiscard]] engine::acs::Handle handle() const noexcept {
			return _handle;
		}

		[[nodiscard]] std::ptrdiff_t offset() const noexcept {
			return _offset;
		}

		[[nodiscard]] Opt<ref<Type_>> get() const noexcept {
			if (_epoch != engine::acs::Epoch::current()) {
				// The offset may come from a loaded layout; the registry rejects one that is no component of the actor's class.
				const auto core = engine::Engine::getEngine();
				_cached = core && _handle.space() != engine::acs::detachedSpace ?
					reinterpret_cast<ptr<Type_>>(core->getActors()->getRegistry()->resolveComponent(_handle, _offset)) :
					nullptr;
				_epoch = engine::acs::Epoch::current();
			}
			if (_cached == nullptr) {
				return None;
			}
			return Some(*_cached);
		}
	};
}
