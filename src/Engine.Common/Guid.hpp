#pragma once

#include <concepts>

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

		constexpr Guid(const Guid& other_) noexcept :
			pre(other_.pre),
			c0(other_.c0),
			c1(other_.c1),
			post(other_.post) {}

		constexpr Guid(Guid&& other_) noexcept :
			pre(other_.pre),
			c0(other_.c0),
			c1(other_.c1),
			post(other_.post) {}

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

		#ifdef _WIN32
		explicit Guid(const _GUID& value_);
		#endif

		constexpr ref<Guid> operator=(cref<Guid> other_) noexcept {
			pre = other_.pre;
			c0 = other_.c0;
			c1 = other_.c1;
			post = other_.post;
			return *this;
		}

		constexpr ref<Guid> operator=(mref<Guid> other_) noexcept {
			pre = other_.pre;
			c0 = other_.c0;
			c1 = other_.c1;
			post = other_.post;
			return *this;
		}

	public:
		void hton(ref<u8[16]> dst_) noexcept;

		[[nodiscard]] static Guid ntoh(cref<u8[16]> src_) noexcept;

	public:
		[[nodiscard]] operator bool() const noexcept {
			return pre || c0 || c1 || post;
		}

		[[nodiscard]] bool operator!() const noexcept {
			return not static_cast<bool>(*this);
		}

		[[nodiscard]] FORCE_INLINE bool operator==(const Guid& other_) const noexcept {
			return pre == other_.pre && c0 == other_.c0 && c1 == other_.c1 && post == other_.post;
		}

		[[nodiscard]] FORCE_INLINE bool operator!=(const Guid& other_) const noexcept {
			return pre != other_.pre || c0 != other_.c0 || c1 != other_.c1 || post != other_.post;
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
		[[nodiscard]] std::size_t operator()(::hg::cref<::hg::Guid> value_) const noexcept {

			size_t seed = 31587166197;

			::hg::hash::hash_combine(seed, value_.pre);
			::hg::hash::hash_combine(seed, value_.c0);
			::hg::hash::hash_combine(seed, value_.c1);
			::hg::hash::hash_combine(seed, value_.post);

			return seed;
		}
	};
}
