#pragma once

#include <type_traits>
#include <tl/optional.hpp>

#include "Wrapper.hpp"
#include "Meta/Reference.hpp"

namespace hg {
	template <typename Ty_>
	using Optional = tl::optional<Ty_>;

	template <class Ty_>
	class Opt :
		public tl::optional<Ty_> {
	public:
		using this_type = Opt<Ty_>;
		using underlying_type = tl::optional<Ty_>;

	public:
		using underlying_type::optional;

		template <typename Uy_>
		constexpr Opt(Uy_&& other_) noexcept :
			underlying_type(std::forward<Uy_>(other_)) {}

	public:
		// Note: Comparing a optional pointer-like type, might introduce ambiguity when equal comparing to nullptr
		bool operator==(std::nullptr_t) const = delete;

		bool operator!=(std::nullptr_t) const = delete;
	};

	template <typename Ty_>
	class Opt<Ty_&> :
		public tl::optional<Ty_&> {
	public:
		using this_type = Opt<Ty_&>;
		using underlying_type = tl::optional<Ty_&>;

	public:
		using underlying_type::optional;

		template <typename Uy_>
		constexpr Opt(Uy_&& other_) noexcept :
			underlying_type(std::forward<Uy_>(other_)) {}

	public:
		[[nodiscard]] constexpr bool is_null() const noexcept {
			return not underlying_type::has_value();
		}

		[[nodiscard]] constexpr const ptr<const Ty_> address() const noexcept {
			return underlying_type::has_value() ? std::addressof(underlying_type::value()) : nullptr;
		}

	public:
		// Note: Comparing a optional pointer-like type, might introduce ambiguity when equal comparing to nullptr
		bool operator==(std::nullptr_t) const = delete;

		bool operator!=(std::nullptr_t) const = delete;
	};

	template <typename Ty_>
	Opt(Ty_&&) -> Opt<Ty_>;

	/**/

	constexpr inline auto None = tl::nullopt;

	template <typename Type_> requires (not std::is_reference_v<Type_>)
	[[nodiscard]] constexpr ::hg::Opt<Type_> Some(auto&&... args_) noexcept {
		return ::hg::Opt<Type_>(std::forward<decltype(args_)>(args_)...);
	}

	template <typename Type_> requires (std::is_reference_v<Type_>)
	[[nodiscard]] constexpr ::hg::Opt<Type_> Some(ref<::hg::meta::drop_ref_t<Type_>> ref_) noexcept {
		return ::hg::Opt<Type_>(ref_);
	}

	[[nodiscard]] constexpr decltype(auto) Some(auto&& args_) noexcept {
		return ::hg::Opt(std::forward<decltype(args_)>(args_));
	}

	/**/

	template <typename Type_>
	concept IsOpt = tl::detail::is_optional<Type_>::value;
}

/**/

template <class T>
struct tl::detail::is_optional_impl<::hg::Opt<T>> : std::true_type {};
