#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Meta/Type.hpp>

#include "Meta/MetaClass.hpp"
#include "Meta/TypedMetaClass.hpp"

namespace hg::refl {
	/**/

	enum class TypeInfoBits : u8 {
		eIsConstBit = 0x0,
		eIsLRefBit = 0x1,
		eIsRRefBit = 0x2,
	};

	/**/

	struct FullTypeInfo final {
	public:
		using this_type = FullTypeInfo;
		using meta_class_type = MetaClass;

	public:
		__restricted_ptr<const meta_class_type> meta;

		union {
			struct {
				bool is_const : 1;
				bool is_lref : 1;
				bool is_rref : 1;
			};

			std::underlying_type_t<TypeInfoBits> bitset {};
		};

	public:
		[[nodiscard]] constexpr bool check(const TypeInfoBits infoBit_) const noexcept {

			using underlying_type = std::underlying_type_t<TypeInfoBits>;
			return (bitset >> static_cast<underlying_type>(infoBit_)) & 0x1;
		}

		template <TypeInfoBits InfoBit_>
		[[nodiscard]] constexpr bool check() const noexcept {

			using underlying_type = std::underlying_type_t<TypeInfoBits>;
			return (bitset >> static_cast<underlying_type>(InfoBit_)) & 0x1;
		}

	public:
		[[nodiscard]] constexpr bool exact(_In_ cref<FullTypeInfo> check_) const noexcept {
			return is_const == check_.is_const && is_lref == check_.is_lref && is_rref == check_.is_rref &&
				meta == check_.meta;
			/* Question: Do we need `meta->exact(check_.meta->typeId())`? */
		}

		[[nodiscard]] constexpr bool is(_In_ cref<FullTypeInfo> check_) const noexcept {
			return is_const == check_.is_const && is_lref == check_.is_lref && is_rref == check_.is_rref &&
				(meta == check_.meta || (meta != nullptr && check_.meta != nullptr && meta->is(check_.meta->typeId())));
		}

	public:
		template <CompleteType Type_>
		consteval static FullTypeInfo from() noexcept {
			return {
				.meta = TypedMetaClass<Type_>::get(),
				.is_const = std::is_const_v<std::remove_reference_t<Type_>>,
				.is_lref = std::is_lvalue_reference_v<Type_>,
				.is_rref = std::is_rvalue_reference_v<Type_>
			};
		}
	};
}
