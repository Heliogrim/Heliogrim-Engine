#pragma once

#include <concepts>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "Scheme.hpp"

namespace hg::engine::storage {
	/**/

	template <class Derived_>
	class IUrlComp;

	template <class Type_>
	concept IsUrlComp = std::derived_from<Type_, IUrlComp<Type_>> && requires(const Type_& obj_) {
		{ obj_.encode() } -> std::same_as<String>;
		{ obj_.scheme() } -> std::same_as<UrlScheme>;
	};

	/**/

	template <class Derived_>
	class __declspec(novtable) IUrlComp {
	protected:
		// Attention: We may need to defer the constructor instantiation...
		constexpr IUrlComp() noexcept {
			static_assert(IsUrlComp<Derived_>);
		}

	public:
		constexpr IUrlComp(const IUrlComp&) noexcept = default;

		constexpr IUrlComp(IUrlComp&&) noexcept = default;

		constexpr ~IUrlComp() noexcept = default;
	};
}
