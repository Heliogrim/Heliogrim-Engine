#pragma once

#include <atomic>
#include <memory>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Wrapper.hpp"
#include "../Meta/Type.hpp"

namespace hg {
	/**
	 * Object counter surface of the split-count protocol: `C = live * 2^32 + net`, where `live` is the number of
	 *	Arci instances holding the object and `net` (signed, low 32 bits) collects folded pin credits minus debits.
	 *	`ref_fetch_add` returns the previous value of C.
	 */
	template <typename Ty_>
	concept IsAtomicIntrusiveRefCountable = requires(const Ty_& obj_) {
			{ obj_.ref_fetch_add(std::declval<s64>(), std::memory_order::relaxed) } -> std::same_as<s64>;
			{ obj_.ref_count() } -> std::convertible_to<u32>;
		};

	template <typename Ty_>
	concept IncompleteOrAtomicIntrusiveRefCountable = (not CompleteType<Ty_>) || (IsAtomicIntrusiveRefCountable<Ty_>);

	// Pointer conversion or downcast, never dropping const from the handled type.
	template <typename From_, typename To_>
	concept ArciConvertibleTo = std::is_convertible_v<ptr<From_>, ptr<To_>> ||
		(std::derived_from<To_, From_> && (std::is_const_v<To_> || not std::is_const_v<From_>));

	/**/

	// Every atomic access of the object counter and the Arci slot word goes through the policy, so a test or
	//	model-checker policy can intercept them.
	struct StdAtomicPolicy {
		template <class Ux_>
		using atomic = std::atomic<Ux_>;

		static void fence(const std::memory_order order_) noexcept {
			std::atomic_thread_fence(order_);
		}
	};

	namespace {
		constexpr s64 arci_live_one = s64 { 1 } << 32;
	}

	/**/

	template <IncompleteOrAtomicIntrusiveRefCountable Ty_, class Policy_ = StdAtomicPolicy>
	class AtomicRefCountedIntrusive;

	/**/

	template <class Derived_, class Policy_ = StdAtomicPolicy>
	class ArcFromThis {
	public:
		template <IncompleteOrAtomicIntrusiveRefCountable Ux_, class Px_>
		friend class ::hg::AtomicRefCountedIntrusive;

	public:
		using this_type = ArcFromThis<Derived_, Policy_>;

		using counter_type = typename Policy_::template atomic<s64>;
		using derived_type = Derived_;

	public:
		// Starts with one live word and its net credit, adopted by `Arci::create` / `create_from_storage`.
		template <typename DerivedType_ = Derived_>
		constexpr ArcFromThis() noexcept :
			_refs(arci_live_one + 1) {
			static_assert(
				std::derived_from<DerivedType_, this_type>,
				"Used crtp helper class for ref counting incorrectly."
			);
		}

		ArcFromThis(const this_type&) = delete;

		ArcFromThis(this_type&&) = delete;

		constexpr ~ArcFromThis() noexcept = default;

	public:
		ref<this_type> operator=(const this_type&) = delete;

		ref<this_type> operator=(this_type&&) = delete;

	private:
		// Ownership bookkeeping, not object state: `Arci<const Derived_>` must still count.
		mutable counter_type _refs;

	public:
		s64 ref_fetch_add(const s64 delta_, const std::memory_order order_) const noexcept {
			return _refs.fetch_add(delta_, order_);
		}

		// Number of live Arci instances; a snapshot under concurrency.
		[[nodiscard]] u32 ref_count() const noexcept {
			// Round so a negative `net` does not borrow from `live`.
			return static_cast<u32>((_refs.load(std::memory_order::relaxed) + (s64 { 1 } << 31)) >> 32);
		}

	public:
		template <typename Proxy_ = Derived_>
		[[nodiscard]] AtomicRefCountedIntrusive<Proxy_, Policy_> arci_from_this() noexcept;
	};

	/**/

	namespace {
		using arci_packed_type = std::uintptr_t;

		constexpr arci_packed_type arci_packed_shift = 16uLL;
		constexpr arci_packed_type arci_packed_ref_mask =
			0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'11111111;
		constexpr arci_packed_type arci_packed_ptr_mask =
			0b11111111'11111111'11111111'11111111'11111111'11111111'00000000'00000000;
	};

	/**
	 * Split reference counting: the slot word packs the 48-bit pointer with a 16-bit pin count T, the object keeps
	 *	`C = live * 2^32 + net`. A copy pins the source word (one RMW, so the pointer it reads is the one it pins),
	 *	then adds one live; a dropped word folds `-live + (T - 1)` into C. The object is freed when C reaches zero.
	 *	Every modification of a slot that may be copied concurrently is a single RMW, so copy vs move-assign / reset /
	 *	move-from on the same instance is supported. Copy vs destroy of the same instance is a contract violation.
	 *
	 * Attention: `get()`, `operator->`, `operator*`, `ref_count()` and `ref_unique()` dereference a plain-loaded
	 *	pointer, unsafe against a concurrent reassign of the same slot.
	 */
	template <IncompleteOrAtomicIntrusiveRefCountable Ty_, class Policy_>
	class AtomicRefCountedIntrusive final {
	public:
		template <IncompleteOrAtomicIntrusiveRefCountable Ux_, class Px_>
		friend class ::hg::AtomicRefCountedIntrusive;

	public:
		using this_type = AtomicRefCountedIntrusive<Ty_, Policy_>;

		using storage_type = typename Policy_::template atomic<arci_packed_type>;
		using value_type = Ty_;

		/**
		 * A copier that pins a word with `T >= pin_flush_at` moves the word's pins into the object counter. Each
		 *	thread pins at most once before it flushes, so `T <= pin_flush_at + threads` stays below 2^16 for fewer
		 *	than 2^16 - 2^14 concurrent copiers of one slot. Flush is lock-free, not wait-free (CAS retry).
		 */
		static constexpr u16 pin_flush_at = 1u << 14;

	private:
		[[nodiscard]] static arci_packed_type encode(const ptr<Ty_> pt_, const u16 pins_) noexcept {
			return (reinterpret_cast<arci_packed_type>(pt_) << arci_packed_shift) | arci_packed_type { pins_ };
		}

		[[nodiscard]] static Ty_* decode_pointer(const arci_packed_type& packed_) noexcept {
			return reinterpret_cast<Ty_*>((packed_ & arci_packed_ptr_mask) >> arci_packed_shift);
		}

		[[nodiscard]] static u16 decode_pins(const arci_packed_type& packed_) noexcept {
			return static_cast<u16>(packed_ & arci_packed_ref_mask);
		}

		// Stale pins on a null word are ignored; drop them so they never travel.
		[[nodiscard]] static arci_packed_type normalize(const arci_packed_type packed_) noexcept {
			return decode_pointer(packed_) == nullptr ? arci_packed_type { 0 } : packed_;
		}

		[[nodiscard]] static arci_packed_type pin_copy(ref<storage_type> slot_) noexcept {

			if (decode_pointer(slot_.load(std::memory_order::relaxed)) == nullptr) {
				return arci_packed_type { 0 };
			}

			// Reads inside the release sequence of the writer's exchange, so the object is visible.
			// unverified: needs weak-memory check (acquire pin sees the object published by the writer's exchange)
			const auto old = slot_.fetch_add(1u, std::memory_order::acquire);
			auto* const subject = decode_pointer(old);
			if (subject == nullptr) {
				return arci_packed_type { 0 };
			}

			subject->ref_fetch_add(arci_live_one, std::memory_order::relaxed);

			if (decode_pins(old) >= pin_flush_at) {
				flush(slot_, subject);
			}

			return encode(subject, 0u);
		}

		static void flush(ref<storage_type> slot_, const ptr<Ty_> subject_) noexcept {

			auto current = slot_.load(std::memory_order::relaxed);
			while (decode_pointer(current) == subject_ && decode_pins(current) != 0u) {

				const auto pins = decode_pins(current);

				// shortcut: |net| must stay below 2^31 (about 131k live words each holding about 16k pins), when to upgrade: a 64-bit net field or a bounded-pin guard
				// Credit before the pins leave the word; the undo cannot reach zero, as the flusher holds a live ref.
				subject_->ref_fetch_add(s64 { pins }, std::memory_order::relaxed);
				// unverified: needs weak-memory check (release pairs with the acquire of a later fold's slot read)
				if (slot_.compare_exchange_strong(
					current,
					encode(subject_, 0u),
					std::memory_order::release,
					std::memory_order::relaxed
				)) {
					return;
				}
				subject_->ref_fetch_add(-s64 { pins }, std::memory_order::relaxed);
			}
		}

		static void fold(const arci_packed_type packed_) noexcept {

			auto* const subject = decode_pointer(packed_);
			if (subject == nullptr) {
				return;
			}

			// shortcut: |net| must stay below 2^31 (about 131k live words each holding about 16k pins), when to upgrade: a 64-bit net field or a bounded-pin guard
			// unverified: needs weak-memory check (release fold pairs with the acquire fence on zero before delete)
			const s64 delta = -arci_live_one + (s64 { decode_pins(packed_) } - 1);
			if (subject->ref_fetch_add(delta, std::memory_order::release) + delta == 0) {
				Policy_::fence(std::memory_order::acquire);
				// Note: Matches the allocation of `create`.
				delete subject;
			}
		}

	public:
		template <typename... Args_> requires std::is_constructible_v<Ty_, Args_...>
		[[nodiscard]] constexpr static this_type create(Args_&&... args_) noexcept {

			// TODO: auto alloc = std::allocator<Ty_> {};
			// TODO: auto alloc_traits = std::allocator_traits<std::allocator<Ty_>> {};

			// TODO: auto obj = alloc.allocate(1);
			// TODO: alloc_traits.construct(alloc, obj, std::forward<Args_>(args_)...);

			using alloc_type = std::remove_const_t<Ty_>;
			if constexpr (std::is_aggregate_v<alloc_type>) {
				auto obj = new alloc_type { ::std::forward<Args_>(args_)... };
				return this_type { encode(obj, 0u) };
			} else {
				auto obj = new alloc_type(::std::forward<Args_>(args_)...);
				return this_type { encode(obj, 0u) };
			}
		}

		// Adopts one live word plus its net credit already present on the object (see `ArcFromThis`).
		[[nodiscard]] constexpr static this_type create_from_storage(const ptr<Ty_> self_) noexcept {
			return this_type { encode(self_, 0u) };
		}

	private:
		explicit constexpr AtomicRefCountedIntrusive(mref<arci_packed_type> storage_) noexcept :
			_obj(storage_) {}

	public:
		constexpr AtomicRefCountedIntrusive() noexcept :
			_obj(0LL) {}

		constexpr AtomicRefCountedIntrusive(::std::nullptr_t) noexcept :
			AtomicRefCountedIntrusive() {}

		constexpr AtomicRefCountedIntrusive(const this_type& other_) noexcept :
			_obj(pin_copy(other_._obj)) {}

		// unverified: needs weak-memory check (acquire sees flush credits of the moved word's pins)
		constexpr AtomicRefCountedIntrusive(this_type&& other_) noexcept :
			_obj(normalize(other_._obj.exchange(0u, std::memory_order::acq_rel))) {}

		template <typename From_> requires std::is_const_v<Ty_> && std::same_as<From_, std::remove_const_t<Ty_>>
		constexpr AtomicRefCountedIntrusive(cref<AtomicRefCountedIntrusive<From_, Policy_>> other_) noexcept :
			AtomicRefCountedIntrusive(other_.template into<Ty_>()) {}

		template <typename From_> requires std::is_const_v<Ty_> && std::same_as<From_, std::remove_const_t<Ty_>>
		constexpr AtomicRefCountedIntrusive(mref<AtomicRefCountedIntrusive<From_, Policy_>> other_) noexcept :
			AtomicRefCountedIntrusive(std::move(other_).template into<Ty_>()) {}

		constexpr ~AtomicRefCountedIntrusive() {
			// unverified: needs weak-memory check (acquire orders flush credits on this word before the fold)
			fold(_obj.load(std::memory_order::acquire));
		}

	public:
		ref<this_type> operator=(this_type&& other_) noexcept {
			if (std::addressof(other_) != this) {
				// unverified: needs weak-memory check (both exchanges acq_rel: publish the object, see flush credits)
				const auto moved = normalize(other_._obj.exchange(0u, std::memory_order::acq_rel));
				fold(_obj.exchange(moved, std::memory_order::acq_rel));
			}
			return *this;
		}

		ref<this_type> operator=(const this_type& other_) noexcept = delete;

	private:
		// Mutable: copying pins the source word, also for a `const` source.
		mutable storage_type _obj;

		constexpr void throw_if_null(const arci_packed_type packed_) const {
			::hg::assertrt(decode_pointer(packed_) != nullptr);
		}

	public:
		[[nodiscard]] bool ref_unique() const noexcept {
			auto* const subject = decode_pointer(_obj.load(std::memory_order::relaxed));
			return subject == nullptr ? true : subject->ref_count() == 1u;
		}

		[[nodiscard]] auto ref_count() const noexcept {
			using ref_count_type = decltype(std::declval<const Ty_>().ref_count());
			auto* const subject = decode_pointer(_obj.load(std::memory_order::relaxed));
			return subject == nullptr ? ref_count_type { 0 } : subject->ref_count();
		}

		[[nodiscard]] constexpr ptr<Ty_> get() const noexcept {
			const auto packed = _obj.load(std::memory_order::consume);
			return decode_pointer(packed);
		}

		constexpr void reset() {
			// unverified: needs weak-memory check (acq_rel exchange hands the word's pins and flush credits to the fold)
			fold(_obj.exchange(0u, std::memory_order::acq_rel));
		}

		// Attention: Releasing an intrusive ref-counted object is semantically incorrect.
		auto release() noexcept = delete;

	public:
		[[nodiscard]] constexpr ptr<Ty_> operator->() const {
			const auto packed = _obj.load(std::memory_order::consume);
			throw_if_null(packed);
			return decode_pointer(packed);
		}

		[[nodiscard]] constexpr cref<Ty_> operator*() const {
			const auto packed = _obj.load(std::memory_order::consume);
			throw_if_null(packed);
			return *decode_pointer(packed);
		}

		[[nodiscard]] constexpr ref<Ty_> operator*() {
			const auto packed = _obj.load(std::memory_order::consume);
			throw_if_null(packed);
			return *decode_pointer(packed);
		}

	public:
		[[nodiscard]] constexpr bool operator==(std::nullptr_t) const noexcept {
			return decode_pointer(_obj.load(std::memory_order::consume)) == nullptr;
		}

		[[nodiscard]] constexpr bool operator==(cref<this_type> other_) const noexcept {
			return get() == other_.get();
		}

		[[nodiscard]] constexpr bool operator!=(cref<this_type> other_) const noexcept {
			return get() != other_.get();
		}

	public:
		/**
		 * One counted reference with the pointer cached: reads are plain loads, immune to a concurrent reassign of the
		 *	source slot. The held word is a private copy (pin protocol on creation) that is never pinned itself, so its
		 *	T stays 0 and destruction is a plain fold. Only built in place, never copied, moved or heap allocated.
		 */
		template <typename Access_> requires std::is_convertible_v<ptr<Ty_>, ptr<Access_>>
		class View final {
		public:
			explicit View(cref<this_type> source_) noexcept :
				_hold(source_),
				_obj(_hold.get()) {}

			View(const View&) = delete;

			View(View&&) = delete;

			View& operator=(const View&) = delete;

			View& operator=(View&&) = delete;

			~View() = default;

			static void* operator new(std::size_t) = delete;

			static void* operator new[](std::size_t) = delete;

			static void operator delete(void*) = delete;

			static void operator delete[](void*) = delete;

		private:
			this_type _hold;
			ptr<Access_> _obj;

		public:
			[[nodiscard]] ptr<Access_> get() const noexcept {
				return _obj;
			}

			[[nodiscard]] ptr<Access_> operator->() const noexcept {
				return _obj;
			}
		};

		template <typename Access_ = Ty_> requires std::is_convertible_v<ptr<Ty_>, ptr<Access_>>
		[[nodiscard]] View<Access_> view() const noexcept {
			return View<Access_> { *this };
		}

	public:
		/**
		 * Pedantic Support
		 */
		template <IsAtomicIntrusiveRefCountable ToType_> requires ArciConvertibleTo<Ty_, ToType_>
		[[nodiscard]] AtomicRefCountedIntrusive<ToType_, Policy_> into() const & noexcept {
			return clone(*this).template into<ToType_>();
		}

		// Moves the word; pins belong to the shared object counter, so they travel with the converted pointer.
		template <IsAtomicIntrusiveRefCountable ToType_> requires ArciConvertibleTo<Ty_, ToType_>
		[[nodiscard]] AtomicRefCountedIntrusive<ToType_, Policy_> into() && noexcept {
			using target_type = AtomicRefCountedIntrusive<ToType_, Policy_>;
			// unverified: needs weak-memory check (acq_rel exchange sees flush credits of the moved word's pins)
			const auto moved = normalize(_obj.exchange(0u, std::memory_order::acq_rel));
			return target_type {
				target_type::encode(static_cast<ptr<ToType_>>(decode_pointer(moved)), decode_pins(moved))
			};
		}
	};

	/**/

	template <class Derived_, class Policy_>
	template <typename Proxy_>
	AtomicRefCountedIntrusive<Proxy_, Policy_> ArcFromThis<Derived_, Policy_>::arci_from_this() noexcept {
		// No pin supplies the new word's net credit, so add it with the live one.
		// unverified: needs weak-memory check (relaxed by analogy with the shared_ptr copy)
		const auto previous = ref_fetch_add(arci_live_one + 1, std::memory_order::relaxed);
		::hg::assertd(previous != 0);
		return AtomicRefCountedIntrusive<Proxy_, Policy_>::create_from_storage(static_cast<ptr<Derived_>>(this));
	}
}

/**/

namespace std {
	template <class Type_, class Policy_>
	struct hash<::hg::AtomicRefCountedIntrusive<Type_, Policy_>> :
		public ::std::hash<::std::uintptr_t> {
		[[nodiscard]] size_t operator()(
			::hg::cref<::hg::AtomicRefCountedIntrusive<Type_, Policy_>> value_
		) const noexcept {
			return static_cast<const ::std::hash<::std::uintptr_t>&>(*this)(
				reinterpret_cast<::std::uintptr_t>(std::addressof(*value_))
			);
		}
	};
}
