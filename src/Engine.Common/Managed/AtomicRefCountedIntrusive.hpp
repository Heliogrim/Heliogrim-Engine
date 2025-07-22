#pragma once

#include <memory>
#include <stdexcept>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "../Wrapper.hpp"
#include "../Meta/Type.hpp"

namespace hg {
	template <typename Ty_>
	concept IsAtomicIntrusiveRefCountable = requires(Ty_& obj_) {
			{ obj_.ref_inc() } -> std::convertible_to<u16>;
			{ obj_.ref_dec() } -> std::convertible_to<u16>;
			{ obj_.ref_dec(std::declval<u16>()) } -> std::convertible_to<u16>;
		} && requires(const Ty_& obj_) {
			{ obj_.ref_count() } -> std::convertible_to<u16>;
		};

	template <typename Ty_>
	concept IncompleteOrAtomicIntrusiveRefCountable = (not CompleteType<Ty_>) || (IsAtomicIntrusiveRefCountable<Ty_>);

	/**/

	template <IncompleteOrAtomicIntrusiveRefCountable Ty_>
	class AtomicRefCountedIntrusive;

	/**/

	template <class Derived_>
	class ArcFromThis {
	public:
		template <IncompleteOrAtomicIntrusiveRefCountable Ux_>
		friend class ::hg::AtomicRefCountedIntrusive;

	public:
		using this_type = ArcFromThis<Derived_>;

		using counter_type = std::atomic_uint_fast16_t;
		using derived_type = Derived_;

	public:
		template <typename DerivedType_ = Derived_>
		constexpr ArcFromThis() noexcept :
			_refs(1) {
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
		counter_type _refs;

	public:
		auto ref_inc() noexcept {
			return ++_refs;
		}

		auto ref_dec() noexcept {
			return --_refs;
		}

		auto ref_dec(const u16 refs_) noexcept {
			return _refs -= refs_;
		}

		[[nodiscard]] auto ref_count() const noexcept {
			return _refs.load(std::memory_order::consume);
		}

	public:
		template <typename Proxy_ = Derived_>
		[[nodiscard]] AtomicRefCountedIntrusive<Proxy_> arci_from_this() noexcept;
	};

	/**/

	namespace {
		using arci_packed_type = std::uintptr_t;
		using arci_atomic_packed_type = std::atomic_uintptr_t;

		constexpr arci_packed_type arci_packed_shift = 16uLL;
		constexpr arci_packed_type arci_packed_ref_mask =
			0b00000000'00000000'00000000'00000000'00000000'00000000'11111111'11111111;
		constexpr arci_packed_type arci_packed_ptr_mask =
			0b11111111'11111111'11111111'11111111'11111111'11111111'00000000'00000000;
	};

	template <IncompleteOrAtomicIntrusiveRefCountable Ty_>
	class AtomicRefCountedIntrusive final {
	public:
		using this_type = AtomicRefCountedIntrusive<Ty_>;

		using storage_type = arci_atomic_packed_type;
		using value_type = Ty_;

	private:
		[[nodiscard]] static arci_packed_type encode_pointer(const ptr<Ty_> pt_) noexcept {
			return reinterpret_cast<arci_packed_type>(pt_) << packed_shift;
		}

		[[nodiscard]] static arci_packed_type encode_counter(const u16 refs_) noexcept {
			return arci_packed_type { refs_ };
		}

		[[nodiscard]] static Ty_* decode_pointer(const arci_packed_type& packed_) noexcept {
			return reinterpret_cast<Ty_*>((packed_ & arci_packed_ptr_mask) >> arci_packed_shift);
		}

		[[nodiscard]] static u16 decode_counter(const arci_packed_type& packed_) noexcept {
			return static_cast<u16>(packed_ & arci_packed_ref_mask);
		}

		[[nodiscard]] static arci_packed_type encode(const ptr<Ty_> pt_, const u16 refs_) noexcept {
			return encode_pointer(pt_) | encode_counter(refs_);
		}

	public:
		template <typename... Args_> requires std::is_constructible_v<Ty_, Args_...>
		[[nodiscard]] constexpr static this_type create(Args_&&... args_) noexcept {

			if constexpr (std::is_aggregate_v<Ty_>) {
				auto obj = new Ty_ { ::std::forward<Args_>(args_)... };
				return this_type { encode(obj, 1u) };
			} else {
				auto obj = new Ty_(::std::forward<Args_>(args_)...);
				return this_type { encode(obj, 1u) };
			}
		}

		[[nodiscard]] constexpr static this_type create_from_storage(const ptr<Ty_> self_) noexcept {
			return this_type { encode(self_, 1u) };
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
			_obj(++const_cast<ref<arci_atomic_packed_type>>(other_._obj)) {

			auto* const subject = decode_pointer(_obj.load(std::memory_order::relaxed));

			if (subject != nullptr) {
				subject->ref_inc();
			}
			--const_cast<ref<arci_atomic_packed_type>>(other_._obj);

			/**/

			_obj.store(encode(subject, 1u), std::memory_order::relaxed);
		}

		constexpr AtomicRefCountedIntrusive(this_type&& other_) noexcept :
			_obj(++other_._obj) {

			// Erase previous ownership of other_ subject
			const auto old_refs = decode_counter(_obj.load(std::memory_order::relaxed));
			if (old_refs >= 2u) {
				other_._obj.fetch_and(arci_packed_ref_mask, std::memory_order::release);

				// Release bumped reference
				other_._obj.fetch_sub(1u, std::memory_order::release);

				// Sanitize encoded value
				auto* const subject = decode_pointer(_obj.load(std::memory_order::relaxed));
				_obj.store(encode(subject, 1u), std::memory_order::relaxed);
				return;
			}

			// If old_refs < 2u, we got an inactive or uninitialized pointer
			other_._obj.fetch_sub(1u, std::memory_order::relaxed);
			_obj.store(0uLL, std::memory_order::relaxed);
		}

		constexpr ~AtomicRefCountedIntrusive() {
			dec_not_null();
		}

	public:
		ref<this_type> operator=(this_type&& other_) noexcept {
			if (std::addressof(other_) != this) {

				/* "Lock" other local encoded pointer */

				// Alias: ~fetch_add (..., std::memory_order::acq_rel)
				auto source = ++other_._obj;
				while (decode_counter(source) > 2u) {
					other_._obj.fetch_sub(1u, std::memory_order::release);
					// TODO: yield
					source = ++other_._obj;
				}

				/* Exclusively extract remote pointer */

				const auto remote_refs = decode_counter(source);

				other_._obj.fetch_and(arci_packed_ref_mask, std::memory_order::release);
				other_._obj.fetch_sub(remote_refs, std::memory_order::release);

				auto* const remote = decode_pointer(source);

				/* "Lock" local encoded pointer */

				// Alias: ~fetch_add (..., std::memory_order::acq_rel)
				auto target = ++_obj;
				while (decode_counter(target) > 2u) {
					_obj.fetch_sub(1u, std::memory_order::release);
					// TODO: yield;
					target = ++_obj;
				}

				/**/

				auto* local = decode_pointer(target);
				const auto local_refs = decode_counter(target);

				// Note: local_refs == 1u -> inactive pointer ( got moved )
				//			local_refs == 2u -> active pointer ( still owned )
				if (local != nullptr && local_refs == 2u) {
					// Modify local intrusive counter to pseudo-holder
					_obj.fetch_and(arci_packed_ref_mask, std::memory_order::release);
				}

				if (local_refs == 1u) {

					// Note: As the previous value is an inactive pointer
					//			we are required to "relock" as an active one

					target = ++_obj;
					while (decode_counter(target) != 2u) {
						_obj.fetch_sub(1u, std::memory_order::release);
						// TODO: yield
						target = ++_obj;
					}

					// Note: We may end up in between multiple assignments which could potentially
					//			store a pointer before the lock expansion
					if (auto recursive = decode_pointer(target); recursive != nullptr) {
						// Modify local intrusive counter to pseudo-holder
						_obj.fetch_and(arci_packed_ref_mask, std::memory_order::release);
						// Note: As local_refs was originally 1u, we can assume that the initial local was nullptr
						local = recursive;
					}
				}

				/* Store remote to local */
				_obj.fetch_or(encode_pointer(remote), std::memory_order_acq_rel);

				/* Release local "lock" */

				// Note: We will only release one reference bump, as the second one was required
				//			to lift the inactive to and active pointer state
				_obj.fetch_sub(1u, std::memory_order::release);

				/* Cleanup deferred destruction */
				if (local != nullptr) {
					// Drop stored object with exclusive ownership
					if (local->ref_dec() == 0) {
						auto alloc = std::allocator<Ty_> {};
						auto alloc_traits = std::allocator_traits<std::allocator<Ty_>> {};
						alloc_traits.destroy(alloc, local);
					}
				}
			}
			return *this;
		}

		ref<this_type> operator=(const this_type& other_) noexcept = delete;

	private:
		storage_type _obj;

	private:
		constexpr void destroy_obj(ptr<Ty_> obj_) {

			_obj.store(arci_packed_type { 0 }, std::memory_order::relaxed);
			delete obj_;
		}

		constexpr void cascade_dec(const ptr<Ty_> obj_) {
			if (obj_ != nullptr && obj_->ref_dec() == 0) {
				destroy_obj(obj_);
			}
		}

		constexpr void dec_not_null() {

			if (decode_counter(_obj.load(std::memory_order::consume)) == 0) {
				return;
			}

			// Alias: ~fetch_sub (..., std::memory_order::acq_rel)
			const auto packed = --_obj;
			if (decode_counter(packed) == 0) {
				cascade_dec(decode_pointer(packed));
			}
		}

		constexpr void throw_if_null(const arci_packed_type packed_) const {
			/*
			if (decode_pointer(packed_) == nullptr) {
				throw std::runtime_error("Failed to access ref counted object with ref count of zero.");
			}
			 */
			::hg::assertrt(decode_pointer(packed_) != nullptr);
		}

	public:
		[[nodiscard]] bool ref_unique() const noexcept {
			using ref_count_type = decltype(std::declval<const Ty_>().ref_count());
			auto* const subject = decode_pointer(_obj.load(std::memory_order::consume));
			return subject == nullptr ? true : subject->ref_count() == ref_count_type { 1 } ? true : false;
		}

		[[nodiscard]] auto ref_count() const noexcept {
			using ref_count_type = decltype(std::declval<const Ty_>().ref_count());
			auto* const subject = decode_pointer(_obj.load(std::memory_order::consume));
			return subject == nullptr ? ref_count_type { 0 } : subject->ref_count();
		}

		[[nodiscard]] constexpr ptr<Ty_> get() const noexcept {
			const auto packed = _obj.load(std::memory_order::consume);
			return decode_pointer(packed);
		}

		constexpr void reset() {
			dec_not_null();
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
			const auto stored = _obj.load(std::memory_order::consume);
			return stored == 0 || decode_pointer(stored) == nullptr;
		}

		[[nodiscard]] constexpr bool operator==(cref<this_type> other_) const noexcept {
			const auto selfStored = _obj.load(std::memory_order::consume);
			const auto otherStored = other_._obj.load(std::memory_order::consume);
			return selfStored == otherStored || decode_pointer(selfStored) == decode_pointer(otherStored);
		}

		[[nodiscard]] constexpr bool operator!=(cref<this_type> other_) const noexcept {
			const auto selfStored = _obj.load(std::memory_order::consume);
			const auto otherStored = other_._obj.load(std::memory_order::consume);
			return selfStored != otherStored && decode_pointer(selfStored) != decode_pointer(otherStored);
		}

	public:
		/**
		 * Pedantic Support
		 */
		template <IsAtomicIntrusiveRefCountable ToType_> requires
			std::is_convertible_v<ptr<Ty_>, ptr<ToType_>> ||
			std::derived_from<ToType_, Ty_>
		[[nodiscard]] AtomicRefCountedIntrusive<ToType_> into() const & noexcept {
			return clone(*this).template into<ToType_>();
		}

		template <IsAtomicIntrusiveRefCountable ToType_> requires
			std::is_convertible_v<ptr<Ty_>, ptr<ToType_>> ||
			std::derived_from<ToType_, Ty_>
		[[nodiscard]] AtomicRefCountedIntrusive<ToType_> into() && noexcept {
			return static_cast<mref<AtomicRefCountedIntrusive<ToType_>>>(
				*reinterpret_cast<ptr<AtomicRefCountedIntrusive<ToType_>>>(this)
			);
		}

	private:
		template <IsAtomicIntrusiveRefCountable ToType_> requires
			std::is_convertible_v<ptr<Ty_>, ptr<ToType_>> ||
			std::derived_from<ToType_, Ty_>
		[[nodiscard]] explicit operator AtomicRefCountedIntrusive<ToType_>() && noexcept {
			return static_cast<mref<AtomicRefCountedIntrusive<ToType_>>>(
				*reinterpret_cast<ptr<AtomicRefCountedIntrusive<ToType_>>>(this)
			);
		}
	};

	/**/

	template <class Derived_>
	template <typename Proxy_>
	AtomicRefCountedIntrusive<Proxy_> ArcFromThis<Derived_>::arci_from_this() noexcept {
		const auto result = ref_inc();
		::hg::assertd(result);
		return AtomicRefCountedIntrusive<Proxy_>::create_from_storage(static_cast<ptr<Derived_>>(this));
	}
}

/**/

namespace std {
	template <class Type_>
	struct hash<::hg::AtomicRefCountedIntrusive<Type_>> :
		public ::std::hash<::std::uintptr_t> {
		[[nodiscard]] size_t operator()(::hg::cref<::hg::AtomicRefCountedIntrusive<Type_>> value_) const noexcept {
			return static_cast<const ::std::hash<::std::uintptr_t>&>(*this)(
				reinterpret_cast<::std::uintptr_t>(std::addressof(*value_))
			);
		}
	};
}
