#pragma once

#include "Meta/Reference.hpp"
#include "Move.hpp"
#include "Wrapper.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
import tl.optional;
#endif

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

	/* Spelled out: `tl::nullopt` and `tl::in_place` are `static`, so the tl.optional module does not export them. */
	constexpr inline tl::nullopt_t None { tl::nullopt_t::do_not_use {}, tl::nullopt_t::do_not_use {} };

	template <typename Type_>
	[[nodiscard]] constexpr ::hg::Opt<::hg::ref<Type_>> Some(::hg::ref<Type_> ref_) noexcept {
		return ::hg::Opt<::hg::ref<Type_>>(ref_);
	}

	template <typename Type_> requires ::std::is_rvalue_reference_v<::hg::mref<Type_>>
	[[nodiscard]] constexpr ::hg::Opt<::hg::meta::drop_ref_t<Type_>> Some(::hg::mref<Type_> ref_) noexcept {
		return ::hg::Opt<::hg::meta::drop_ref_t<Type_>>(::hg::move(ref_));
	}

	template <typename Type_> requires ::std::is_lvalue_reference_v<Type_>
	[[nodiscard]] constexpr ::hg::Opt<Type_> SomeRef(Type_ ref_) noexcept {
		return ::hg::Opt<Type_>(ref_);
	}

	template <typename Type_>
	[[nodiscard]] constexpr hg::Opt<ref<::hg::meta::drop_ref_t<Type_>>> SomeRef(Type_& ref_) noexcept {
		return ::hg::Opt<::hg::ref<::hg::meta::drop_ref_t<Type_>>>(ref_);
	}

	template <typename Type_> requires ::std::is_rvalue_reference_v<Type_>
	[[nodiscard]] constexpr ::hg::Opt<::hg::meta::drop_ref_t<Type_>> SomeVal(Type_ ref_) noexcept {
		return ::hg::Opt<::hg::meta::drop_ref_t<Type_>>(::hg::move(ref_));
	}

	template <typename Type_> requires (not::std::is_reference_v<Type_>)
	[[nodiscard]] constexpr ::hg::Opt<Type_> SomeVal(Type_ val_) noexcept {
		return ::hg::Opt<Type_>(::hg::move(val_));
	}

	template <typename Type_>
	[[nodiscard]] constexpr hg::Opt<::hg::meta::drop_ref_t<Type_>> SomeVal(auto&&... args_) noexcept {
		return ::hg::Opt<::hg::meta::drop_ref_t<Type_>>(::tl::in_place_t {},::hg::forward<decltype(args_)>(args_)...);
	}

	/**/

	template <typename Type_>
	concept IsOpt = tl::detail::is_optional<Type_>::value;
}

/**/

template <class T>
struct tl::detail::is_optional_impl<::hg::Opt<T>> : std::true_type {};
