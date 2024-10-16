#pragma once

#include <concepts>
#include <span>

#include "Types.hpp"
#include "Wrapper.hpp"
#include "__macro.hpp"
#include "Math/Hash.hpp"

#ifdef _WIN32
struct _GUID;
#endif

namespace hg {
	struct Guid {
		constexpr Guid() noexcept :
			pre(),
			c0(),
			c1(),
			post() {}

		constexpr Guid(const Guid& other_) noexcept = default;

		constexpr Guid(Guid&& other_) noexcept = default;

		constexpr Guid(const u32 pre_, const u16 c0_, const u16 c1_, const u64 post_) noexcept :
			pre(pre_),
			c0(c0_),
			c1(c1_),
			post(post_) {}

		#ifdef _WIN32
		explicit Guid(const _GUID& value_);
		#endif

		constexpr ref<Guid> operator=(cref<Guid> other_) noexcept = default;

		constexpr ref<Guid> operator=(mref<Guid> other_) noexcept = default;

	public:
		void hton(ref<u8[16]> dst_) noexcept;

		[[nodiscard]] static Guid ntoh(cref<u8[16]> src_) noexcept;

	public:
		[[nodiscard]] constexpr explicit operator bool() const noexcept {
			return pre || c0 || c1 || post;
		}

		[[nodiscard]] constexpr bool operator!() const noexcept {
			return not static_cast<bool>(*this);
		}

		[[nodiscard]] constexpr bool operator==(const Guid& other_) const noexcept {
			return pre == other_.pre && c0 == other_.c0 && c1 == other_.c1 && post == other_.post;
		}

		[[nodiscard]] constexpr bool operator!=(const Guid& other_) const noexcept {
			return pre != other_.pre || c0 != other_.c0 || c1 != other_.c1 || post != other_.post;
		}

		[[nodiscard]] constexpr bool operator>(const Guid& other_) const noexcept {

			static_assert(sizeof(decltype(*this)) == sizeof(u64) * 2u);
			static_assert(alignof(decltype(*this)) == alignof(u64));

			const auto self = std::span<const u64, 2uLL> {
				*static_cast<const ptr<const ptr<const u64>>>(
					static_cast<const void* const>(this)
				),
				2uLL
			};
			const auto other = std::span<const u64, 2uLL> {
				*static_cast<const ptr<const ptr<const u64>>>(
					static_cast<const void* const>(std::addressof(other_))
				),
				2uLL
			};

			return (self[0] > other[0]) || (self[0] == other[0] && self[1] > other[1]);
		}

		[[nodiscard]] constexpr bool operator<(const Guid& other_) const noexcept {

			static_assert(sizeof(decltype(*this)) == sizeof(u64) * 2u);
			static_assert(alignof(decltype(*this)) == alignof(u64));

			const auto self = std::span<const u64, 2uLL> {
				*static_cast<const ptr<const ptr<const u64>>>(
					static_cast<const void* const>(this)
				),
				2uLL
			};
			const auto other = std::span<const u64, 2uLL> {
				*static_cast<const ptr<const ptr<const u64>>>(
					static_cast<const void* const>(std::addressof(other_))
				),
				2uLL
			};

			return (self[0] < other[0]) || (self[0] == other[0] && self[1] < other[1]);
		}

		[[nodiscard]] constexpr std::strong_ordering operator <=>(const Guid& other_) const noexcept {

			static_assert(sizeof(decltype(*this)) == sizeof(u64) * 2u);
			static_assert(alignof(decltype(*this)) == alignof(u64));

			/**/

			if (*this < other_) {
				return std::strong_ordering::less;
			}

			if (*this == other_) {
				return std::strong_ordering::equal;
			}

			return std::strong_ordering::greater;
		}

	public:
		u32 pre;
		u16 c0;
		u16 c1;
		u64 post;
	};

	extern void GuidGenerate(ref<Guid> guid_);
}

namespace std {
	template <>
	struct hash<::hg::Guid> {
		[[nodiscard]] constexpr std::size_t operator()(::hg::cref<::hg::Guid> value_) const noexcept {

			size_t seed = 31587166197;

			::hg::hash::hash_combine(seed, value_.pre);
			::hg::hash::hash_combine(seed, value_.c0);
			::hg::hash::hash_combine(seed, value_.c1);
			::hg::hash::hash_combine(seed, value_.post);

			return seed;
		}
	};
}
