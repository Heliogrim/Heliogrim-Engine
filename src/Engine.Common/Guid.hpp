#pragma once

#include "Types.hpp"
#include "Wrapper.hpp"
#include "__macro.hpp"
#include "Integral128.hpp"
#include "Math/Hash.hpp"

#ifdef _WIN32
struct _GUID;
#endif

namespace hg {
	struct Guid {
		constexpr Guid() noexcept :
			data() {}

		constexpr Guid(const Guid& other_) noexcept :
			data(other_.data) {}

		constexpr Guid(Guid&& other_) noexcept :
			data(std::move(other_.data)) {}

		constexpr Guid(const uint128_t& data_) noexcept :
			data(data_) {}

		template <
			std::integral PreType_,
			std::integral C0Type_,
			std::integral C1Type_,
			std::integral PostType_
		>
		constexpr Guid(const PreType_& pre_, const C0Type_& c0_, const C1Type_& c1_, const PostType_& post_) :
			pre(pre_),
			c0(c0_),
			c1(c1_),
			post(post_) {}

		Guid(
			const u8 (&bytes_)[16]
		) :
			Guid() {
			std::memcpy(bytes, bytes_, 16);
		}

		#ifdef _WIN32
		explicit Guid(const _GUID& value_);
		#endif

		constexpr ref<Guid> operator=(cref<Guid> other_) noexcept {
			data = other_.data;
			return *this;
		}

		constexpr ref<Guid> operator=(mref<Guid> other_) noexcept {
			data = std::move(other_.data);
			return *this;
		}

		[[nodiscard]] operator bool() const noexcept {
			return data.operator bool();
		}

		[[nodiscard]] bool operator!() const noexcept {
			return !data;
		}

		[[nodiscard]] FORCE_INLINE bool operator>(const Guid& other_) const noexcept {
			return data.low > other_.data.low && data.high > other_.data.high;
		}

		[[nodiscard]] FORCE_INLINE bool operator<(const Guid& other_) const noexcept {
			return data.low < other_.data.low && data.high < other_.data.high;
		}

		[[nodiscard]] FORCE_INLINE bool operator>=(const Guid& other_) const noexcept {
			return data.low >= other_.data.low && data.high >= other_.data.high;
		}

		[[nodiscard]] FORCE_INLINE bool operator<=(const Guid& other_) const noexcept {
			return data.low <= other_.data.low && data.high <= other_.data.high;
		}

		[[nodiscard]] FORCE_INLINE bool operator==(const Guid& other_) const noexcept {
			return data == other_.data;
		}

		[[nodiscard]] FORCE_INLINE bool operator!=(const Guid& other_) const noexcept {
			return data != other_.data;
		}

		union {
			struct {
				uint128_t data;
			};

			struct {
				u32 pre;
				u16 c0;
				u16 c1;
				u64 post;
			};

			struct {
				u8 bytes[16];
			};
		};
	};

	extern void GuidGenerate(ref<Guid> guid_);
}

namespace std {
	template <>
	struct hash<::hg::Guid> {
		[[nodiscard]] std::size_t operator()(::hg::cref<::hg::Guid> value_) const noexcept {

			size_t seed = 31587166197;

			::hg::hash::hash_combine(seed, value_.data.high);
			::hg::hash::hash_combine(seed, value_.data.low);

			return seed;
		}
	};
}
