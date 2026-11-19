#pragma once

#include <Engine.Common/__macro.hpp>

#include "../AttributeOwnerLink.hpp"
#include "../NamedAttributeTuple.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	template <typename NamedAttribute_>
	struct ExtractTypeOfImpl;

	template <ref<const StringView> Name_, typename Attribute_, ReflowPhase Phase_>
	struct ExtractTypeOfImpl<NamedAttribute<Name_, StyleAttribute<Attribute_, Phase_>>> {
		using named_type = NamedAttribute<Name_, StyleAttribute<Attribute_, Phase_>>;
		using type = Attribute_;
	};

	/**/

	template <typename... Attributes_>
	class StyleAttributes {
	public:
		using this_type = StyleAttributes<Attributes_...>;
		using attributes_type = typename named_attribute_tuple<Attributes_...>::type;

	private:
		attributes_type _attributes;
		AttributeOwnerLink _owner;

		template <std::size_t Index_, typename Dummy_ = void>
		struct GetTypeOfImpl {
			using named_type = meta::drop_ref_t<decltype(std::get<Index_>(std::declval<decltype(_attributes)>()))>;
			using type = typename ExtractTypeOfImpl<named_type>::type;
		};

		template <typename Dummy_>
		struct GetTypeOfImpl<(~0uLL), Dummy_> {
			using type = std::void_t<Dummy_>;
		};

	public:
		constexpr static Flag<ReflowPhase, u8> phaseMask {
			static_cast<u8>((static_cast<u8>(decltype(Attributes_::attribute)::phase) | ... | 0u))
		};

	public:
		constexpr StyleAttributes() = default;

		constexpr explicit StyleAttributes(auto&&... args_)
			requires (sizeof...(args_) == sizeof...(Attributes_)) :
			_attributes(::std::make_tuple<Attributes_...>(::hg::forward<decltype(args_)>(args_)...)) {}

		constexpr StyleAttributes(cref<this_type> other_) noexcept = default;

		constexpr StyleAttributes(mref<this_type> other_) noexcept = default;

		constexpr ~StyleAttributes() = default;

	public:
		ref<this_type> operator=(ref<const this_type> other_) {
			if (std::addressof(other_) != this) {
				_attributes = other_._attributes;
				_owner.notify(phaseMask);
			}
			return *this;
		}

		ref<this_type> operator=(mref<this_type> other_) noexcept {
			if (std::addressof(other_) != this) {
				_attributes = ::hg::move(other_._attributes);
				_owner.notify(phaseMask);
			}
			return *this;
		}

	public:
		void bindOwner(nmpt<Widget> owner_) noexcept {
			_owner.owner = owner_;
		}

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

		template <ref<const StringView> Name_>
		using GetTypeOf = typename GetTypeOfImpl<namedIndex<Name_, 0uLL>()>::type;

	public:
		template <ref<const StringView> AttributeName_>
		[[nodiscard]] auto& get() const noexcept {
			constexpr auto index = namedIndex<AttributeName_, 0uLL>();
			static_assert(index != (~0uLL), "Failed to find attribute with name.");
			return std::get<index>(_attributes).attribute;
		}

		template <ref<const StringView> AttributeName_>
		bool update(cref<GetTypeOf<AttributeName_>> next_) noexcept {
			constexpr auto index = namedIndex<AttributeName_, 0uLL>();
			static_assert(index != (~0uLL), "Failed to find attribute with name.");
			auto& slot = std::get<index>(_attributes).attribute;
			if (not slot.update(::hg::forward<decltype(next_)>(next_))) {
				return false;
			}
			_owner.notify(slot.phase);
			return true;
		}

		template <ref<const StringView> AttributeName_>
		bool update(mref<GetTypeOf<AttributeName_>> next_) noexcept {
			constexpr auto index = namedIndex<AttributeName_, 0uLL>();
			static_assert(index != (~0uLL), "Failed to find attribute with name.");
			auto& slot = std::get<index>(_attributes).attribute;
			if (not slot.update(::hg::forward<decltype(next_)>(next_))) {
				return false;
			}
			_owner.notify(slot.phase);
			return true;
		}

		template <ref<const StringView> AttributeName_>
		[[nodiscard]] auto& valueOf() const noexcept {
			constexpr auto index = namedIndex<AttributeName_, 0uLL>();
			static_assert(index != (~0uLL), "Failed to find attribute with name.");
			return std::get<index>(_attributes).attribute.value();
		}
	};
}
