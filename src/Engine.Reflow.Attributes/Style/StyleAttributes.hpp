#pragma once

#include <tuple>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Meta/Reference.hpp>

#include "../NamedAttributeTuple.hpp"

namespace hg::engine::reflow {
	template <typename... Attributes_>
	class StyleAttributes {
	public:
		using this_type = StyleAttributes<Attributes_...>;
		using attributes_type = typename named_attribute_tuple<Attributes_...>::type;

	private:
		attributes_type _attributes;

	public:
		constexpr StyleAttributes() = default;

		constexpr explicit StyleAttributes(auto&&... args_)
			requires (sizeof...(args_) == sizeof...(Attributes_)) :
			_attributes(::std::make_tuple<Attributes_...>(::hg::forward<decltype(args_)>(args_)...)) {}

		constexpr StyleAttributes(cref<this_type> other_) noexcept = default;

		constexpr StyleAttributes(mref<this_type> other_) noexcept = default;

		constexpr ~StyleAttributes() = default;

	public:
		ref<this_type> operator=(cref<this_type> other_) noexcept = default;

		ref<this_type> operator=(mref<this_type> other_) noexcept = default;

	public:
		template <ref<const StringView> Name_, std::size_t Index_>
		[[nodiscard]] constexpr static std::size_t namedIndex() noexcept {
			if constexpr (meta::drop_ref_t<decltype(std::get<Index_>(std::declval<decltype(_attributes)>()))>::name == Name_) {
				return Index_;
			}
			if constexpr (Index_ < (sizeof...(Attributes_) - 1)) {
				return namedIndex<Name_, Index_ + 1>();
			}
			return (~0uLL);
		}

	public:
		template <ref<const StringView> AttributeName_>
		[[nodiscard]] auto& get() const noexcept {
			constexpr auto index = namedIndex<AttributeName_, 0uLL>();
			static_assert(index != (~0uLL), "Failed to find attribute with name.");
			return std::get<index>(_attributes).attribute;
		}

		template <ref<const StringView> AttributeName_>
		bool update(auto&& next_) noexcept {
			constexpr auto index = namedIndex<AttributeName_, 0uLL>();
			static_assert(index != (~0uLL), "Failed to find attribute with name.");
			return std::get<index>(_attributes).attribute.update(::hg::forward<decltype(next_)>(next_));
		}

		template <ref<const StringView> AttributeName_>
		[[nodiscard]] auto& valueOf() const noexcept {
			constexpr auto index = namedIndex<AttributeName_, 0uLL>();
			static_assert(index != (~0uLL), "Failed to find attribute with name.");
			return std::get<index>(_attributes).attribute.value();
		}
	};
}
