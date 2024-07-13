#pragma once

#include <memory>
#include <utility>

#include <Engine.Asserts/Asserts.hpp>

#include "Optional.hpp"
#include "Sal.hpp"
#include "Wrapper.hpp"
#include "Meta/Type.hpp"

namespace hg {
	struct UniqueValueDefaultDestroyer {
		template <CompleteType Type_>
		constexpr void operator()([[maybe_unused]] const ptr<Type_> obj_) const noexcept {
			// __noop();
		}
	};

	template <typename Type_>
	concept IsDefaultUvDestroyer = std::is_same_v<UniqueValueDefaultDestroyer, Type_>;

	/**/

	template <
		CompleteType Type_,
		typename DestroyType_ = UniqueValueDefaultDestroyer> requires
		(std::is_invocable_v<DestroyType_, std::add_const_t<std::add_pointer_t<Type_>>>)
	class UniqueValue {
	public:
		using this_type = UniqueValue<Type_, DestroyType_>;

		using value_type = Type_;
		using destroy_type = DestroyType_;

		constexpr static bool is_default_deleter = IsDefaultUvDestroyer<destroy_type>;

	public:
		constexpr UniqueValue() noexcept
			requires (std::is_default_constructible_v<value_type> && std::is_default_constructible_v<destroy_type>)
		= default;

		explicit constexpr UniqueValue(std::in_place_t) noexcept
			requires (std::is_default_constructible_v<value_type> && std::is_default_constructible_v<destroy_type>) :
			// TODO: Fix this / Ensure inplace construction
			_value(value_type {}),
			_deleter() {}

		template <class... Args_> requires std::is_constructible_v<value_type, Args_...>
		explicit constexpr UniqueValue(Args_&&... args_) noexcept
			requires (is_default_deleter && std::is_default_constructible_v<destroy_type>) :
			_value(std::forward<Args_>(args_)...),
			_deleter() {}

		template <class Deleter_> requires std::is_constructible_v<destroy_type, Deleter_>
		explicit constexpr UniqueValue(mref<value_type> value_, Deleter_&& deleter_) noexcept
			requires (not is_default_deleter) :
			_value(std::move(value_)),
			_deleter(std::forward<Deleter_>(deleter_)) {}

		template <class Deleter_, class... Args_> requires
			std::is_constructible_v<value_type, Args_...> &&
			std::is_constructible_v<destroy_type, Deleter_>
		explicit constexpr UniqueValue(Deleter_&& deleter_, Args_&&... args_) noexcept
			requires (not is_default_deleter && sizeof...(Args_) > 0) :
			_value(std::forward<Args_>(args_)...),
			_deleter(std::forward<Deleter_>(deleter_)) {}

		template <class Deleter_> requires std::is_constructible_v<destroy_type, Deleter_>
		explicit constexpr UniqueValue(Deleter_&& deleter_) noexcept requires (not is_default_deleter) :
			// TODO: Fix this / Ensure inplace construction
			_value(value_type {}),
			_deleter(std::forward<Deleter_>(deleter_)) {}

		UniqueValue(cref<this_type>) = delete;

		constexpr UniqueValue(mref<this_type> other_) noexcept :
			// Note: Moving tl::optional<T> will not cleanup the monad state -> bugprone
			_value(other_._value.take()),
			_deleter(std::move(other_._deleter)) {}

		constexpr ~UniqueValue() noexcept {
			tidy();
		}

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		constexpr ref<this_type> operator=(mref<this_type> other_) noexcept {
			if (std::addressof(other_) != this) {
				tidy();
				// Note: Moving tl::optional<T> will not cleanup the monad state -> bugprone
				_value = other_._value.take();
				_deleter = std::move(other_._deleter);
			}
			return *this;
		}

	private:
		Optional<value_type> _value;
		[[no_unique_address]] destroy_type _deleter;

	private:
		constexpr void tidy() noexcept {
			if (not empty()) {
				_value.and_then(
					[deleter = std::move(_deleter)](_In_ ref<value_type> obj_) -> decltype(_value) {
						deleter(std::addressof(obj_));
						return { tl::nullopt };
					}
				);
			}
		}

	public:
		[[nodiscard]] constexpr bool empty() const noexcept {
			return not _value.has_value();
		}

		[[nodiscard]] constexpr cref<value_type> get(std::nothrow_t) const noexcept {
			if (std::is_constant_evaluated()) {
				::hg::assertrt(not empty());
				return _value.value();
			}

			::hg::assertd(not empty());
			return _value.value();
		}

		[[nodiscard]] constexpr cref<value_type> get() const {
			::hg::assertrt(not empty());
			return _value.value();
		}

		[[nodiscard]] constexpr ref<value_type> get(std::nothrow_t) noexcept {
			if (std::is_constant_evaluated()) {
				::hg::assertrt(not empty());
				return _value.value();
			}

			::hg::assertd(not empty());
			return _value.value();
		}

		[[nodiscard]] constexpr ref<value_type> get() {
			::hg::assertrt(not empty());
			return _value.value();
		}

		[[nodiscard]] constexpr cref<value_type> operator*() const {
			return get();
		}

		[[nodiscard]] constexpr ref<value_type> operator*() {
			return get();
		}

		[[nodiscard]] constexpr ptr<const value_type> operator->() const {
			return std::addressof(get());
		}

		[[nodiscard]] constexpr ptr<value_type> operator->() {
			return std::addressof(get());
		}
	};
}
