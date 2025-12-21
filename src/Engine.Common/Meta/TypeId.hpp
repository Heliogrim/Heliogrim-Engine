#pragma once

#include <concepts>

#include "../Types.hpp"
#include "../Wrapper.hpp"
#include "../Hash/Murmur3.hpp"

namespace hg {
	/**
	 * A class type identifier.
	 *
	 * @author Julius
	 * @date 06.09.2020
	 */
	//typedef size_t type_id;
	struct type_id {
		constexpr type_id() noexcept :
			data(0) {}

		explicit constexpr type_id(const u64 value_) :
			data(value_) {}

		constexpr type_id(const type_id& other_) noexcept = default;

		constexpr type_id(type_id&& other_) noexcept = default;

		constexpr ~type_id() = default;

		type_id& operator=(const type_id& other_) noexcept = default;

		type_id& operator=(type_id&& other_) noexcept = default;

		[[nodiscard]] constexpr bool operator>(const type_id& other_) const noexcept {
			return data > other_.data;
		}

		[[nodiscard]] constexpr bool operator<(const type_id& other_) const noexcept {
			return data < other_.data;
		}

		[[nodiscard]] constexpr bool operator>=(const type_id& other_) const noexcept {
			return data >= other_.data;
		}

		[[nodiscard]] constexpr bool operator<=(const type_id& other_) const noexcept {
			return data <= other_.data;
		}

		[[nodiscard]] constexpr bool operator==(const type_id& other_) const noexcept {
			return data == other_.data;
		}

		[[nodiscard]] constexpr bool operator!=(const type_id& other_) const noexcept {
			return data != other_.data;
		}

		// TODO: Check whether data could be const to make type_id immutable
		u64 data;
	};

	namespace {
		/**
		 * Fnv-1a 86
		 *
		 * @author Julius
		 * @date 06.09.2020
		 *
		 * @param  str_ The.
		 * @param  count_ Number of.
		 *
		 * @returns A type_id.
		 */
		constexpr type_id fnv1a_86(const char* str_, const std::size_t count_) {
			return GUARD_UNCHECKED_LLVL(
				type_id {
				( (count_ ? fnv1a_86(str_, count_ - 1).data : 2166136261uLL) ^ static_cast<cref<u64>>(str_[count_]) ) * 16777619uLL
				}
			);
		}
	}

	/**
	 * Cast that converts the given const char* to a ""_typeId
	 *
	 * @author Julius
	 * @date 06.09.2020
	 *
	 * @param  str_ The.
	 * @param  count_ Number of.
	 *
	 * @returns The result of the operation.
	 */
	consteval type_id operator""_typeId(const char* str_, const std::size_t count_) noexcept {
		return fnv1a_86(str_, count_);
	}

	/**
	 * Type has static type identifier
	 * 
	 */
	template <typename Ty, typename TypeIdType_ = type_id>
	concept HasStaticType = requires(Ty) {
		{ Ty::typeId };
	};

	/**
	 * Type has dynamic type identifier
	 * 
	 */
	template <typename Ty, typename TypeIdType_ = type_id>
	concept HasDynamicType = requires(Ty obj) {
		{ obj.getTypeId() } -> std::same_as<TypeIdType_>;
	};

	/**
	 * Type has static and a dynamic type identifier
	 * 
	 */
	template <typename Ty, typename TypeIdType_ = type_id>
	concept HasType = HasStaticType<Ty, TypeIdType_> && HasDynamicType<Ty, TypeIdType_>;
}

namespace std {
	/**
	 * Specific less check for type_id
	 *
	 * @author Julius
	 * @date 13.09.2021
	 */
	template <>
	struct less<hg::type_id> {
		[[nodiscard]] constexpr bool operator()(const hg::type_id& left_, const hg::type_id& right_) const noexcept {
			return left_.data < right_.data;
		}
	};

	/**
	 * Specific hash of type_id
	 *
	 * @author Julius
	 * @date 13.09.2021
	 */
	template <>
	struct hash<hg::type_id> {
		[[nodiscard]] constexpr std::size_t operator()(const hg::type_id& value_) const noexcept {
			// We assume that type_id was created by literal, which will result in constexpr of FNV-1a, which is already a suitable hash
			/*
			hg::u64 dst[2];
			hg::hash::murmur3_x64_128(
			    &value_,
			    sizeof(decltype(hg::type_id::data)),
			    0x9FB21C651E98DF25uL,
			    &dst
			);
			return dst[0];
			 */
			return value_.data;
		}
	};
}
