#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::reflow {
	template <ref<const StringView> Name_, typename Attribute_>
	struct NamedAttribute {
		using this_type = NamedAttribute<Name_, Attribute_>;

		constexpr static StringView name = Name_;
		Attribute_ attribute;

		constexpr NamedAttribute() noexcept = default;

		constexpr NamedAttribute(auto&& arg0_, auto&&... args_) noexcept
			requires (not std::is_same_v<meta::remove_reference_t<decltype(arg0_)>, this_type>):
			attribute(::hg::forward<decltype(arg0_)>(arg0_), ::hg::forward<decltype(args_)>(args_)...) {}

		constexpr NamedAttribute(cref<this_type> other_) noexcept = default;

		constexpr NamedAttribute(mref<this_type> other_) noexcept = default;

		constexpr ~NamedAttribute() noexcept = default;

		constexpr ref<this_type> operator=(cref<this_type> other_) noexcept = default;

		constexpr ref<this_type> operator=(mref<this_type> other_) noexcept = default;
	};
}
