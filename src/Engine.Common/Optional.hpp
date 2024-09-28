#pragma once

#include <tl/optional.hpp>

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
		bool operator==(std::nullptr_t) const noexcept = delete;

		constexpr bool operator!=(std::nullptr_t) const noexcept {
			return ::tl::operator!=(static_cast<const underlying_type&>(*this), nullptr);
		}
	};

	template <typename Ty_>
	Opt(Ty_&&) -> Opt<Ty_>;

	/**/

	constexpr static auto None = tl::nullopt;

	[[nodiscard]] constexpr static decltype(auto) Some(auto&& args_) noexcept {
		return Opt(std::forward<decltype(args_)>(args_));
	}
}

/**/

template <class T>
struct tl::detail::is_optional_impl<hg::Opt<T>> : std::true_type {};
