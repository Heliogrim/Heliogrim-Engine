#include "Guid.hpp"

#include <bit>
#include <utility>

#include "__macro.hpp"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <combaseapi.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

#elif not defined(ENV_WIN) || not(ENV_WIN)

#include <uuid/uuid.h>
#endif

using namespace hg;

#ifdef _WIN32
Guid::Guid(const _GUID& value_) :
	pre(value_.Data1),
	c0(value_.Data2),
	c1(value_.Data3),
	post(*reinterpret_cast<const u64* const>(value_.Data4)) {}
#endif

void hg::GuidGenerate(ref<Guid> guid_) {
	#ifdef _WIN32

	if constexpr (sizeof(Guid) != sizeof(GUID)) {
		GUID tmp {};
		CoCreateGuid(&tmp);
		guid_ = Guid { static_cast<u32>(tmp.Data1), tmp.Data2, tmp.Data3, *std::bit_cast<u64*>(tmp.Data4) };

	} else if constexpr (sizeof(Guid) == sizeof(GUID)) {
		auto* rg = std::bit_cast<GUID*>(&guid_);
		CoCreateGuid(rg);
	}

	#elif not defined(ENV_WIN) || not(ENV_WIN)

	uuid_t tmp {};
	uuid_generate(tmp);

	guid_ = Guid {
		*std::bit_cast<u32*>(&tmp[0]),
		*std::bit_cast<u16*>(&tmp[4]),
		*std::bit_cast<u16*>(&tmp[6]),
		*std::bit_cast<u64*>(&tmp[8])
	};

	#else
        #error "Not implemented uuid generator."
	#endif
}

void Guid::hton(ref<u8[16]> dst_) noexcept {
	if constexpr (std::endian::native != std::endian::big) {
		(*std::bit_cast<u32*>(&dst_[0])) = std::byteswap(pre);
		(*std::bit_cast<u16*>(&dst_[4])) = std::byteswap(c0);
		(*std::bit_cast<u16*>(&dst_[6])) = std::byteswap(c1);
		(*std::bit_cast<u64*>(&dst_[8])) = std::byteswap(post);
	} else {
		(*std::bit_cast<u32*>(&dst_[0])) = pre;
		(*std::bit_cast<u16*>(&dst_[4])) = c0;
		(*std::bit_cast<u16*>(&dst_[6])) = c1;
		(*std::bit_cast<u64*>(&dst_[8])) = post;
	}
}

Guid Guid::ntoh(cref<u8[16]> src_) noexcept {
	if constexpr (std::endian::native != std::endian::big) {
		return {
			std::byteswap(*std::bit_cast<u32*>(&src_[0])),
			std::byteswap(*std::bit_cast<u16*>(&src_[4])),
			std::byteswap(*std::bit_cast<u16*>(&src_[6])),
			std::byteswap(*std::bit_cast<u64*>(&src_[8]))
		};
	} else {
		return {
			*std::bit_cast<u32*>(&src_[0]),
			*std::bit_cast<u16*>(&src_[4]),
			*std::bit_cast<u16*>(&src_[6]),
			*std::bit_cast<u64*>(&src_[8])
		};
	}
}
