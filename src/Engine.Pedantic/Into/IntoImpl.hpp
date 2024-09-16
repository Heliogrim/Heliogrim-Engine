#pragma once

#include <type_traits>
#include <utility>
#include <Engine.Common/Forward.hpp>

#include "../Concept/SameAs.hpp"

namespace hg {
	template <
		typename TargetType_,
		typename SourceType_
	>
	struct into_impl;

	/*
	{
	    [[nodiscard]] constexpr TargetType_ operator()(SourceType_&& obj_) noexcept;
	};
	*/

	template <
		typename To_,
		typename From_,
		template <typename...> typename VoidTemplate_,
		template <typename...> typename Template_
	>
	concept has_into_impl_same = (std::is_same_v<VoidTemplate_<To_>, void>) &&
		(not std::is_same_v<Template_<From_>, void>) &&
		requires(Template_<From_> obj_) {
			{
				std::declval<into_impl<Template_<To_>, Template_<From_>>>()(
					::hg::forward<Template_<From_>>(obj_)
				)
			} -> same_as<Template_<To_>>;
		};

	template <
		typename To_,
		typename From_,
		template <typename...> typename ToTemplate_,
		template <typename...> typename FromTemplate_
	>
	concept has_into_impl_diff = (not std::is_same_v<ToTemplate_<To_>, void>) &&
		(not std::is_same_v<FromTemplate_<From_>, void>) &&
		requires(FromTemplate_<From_> obj_) {
			{
				std::declval<into_impl<ToTemplate_<To_>, FromTemplate_<From_>>>()(
					::hg::forward<FromTemplate_<From_>>(obj_)
				)
			} -> same_as<ToTemplate_<To_>>;
		};

	template <
		typename To_,
		typename From_,
		template <typename...> typename ToTemplate_,
		template <typename...> typename FromTemplate_
	>
	concept has_into_impl = has_into_impl_same<To_, From_, ToTemplate_, FromTemplate_> ||
		has_into_impl_diff<To_, From_, ToTemplate_, FromTemplate_>;
}
