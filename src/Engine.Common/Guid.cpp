#include "Guid.hpp"

#include <bit>
#include <utility>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <combaseapi.h>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
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

	static_assert(sizeof(Guid) == sizeof(GUID));
	auto* rg = std::bit_cast<GUID*>(&guid_);
	CoCreateGuid(rg);
	#else
        #pragma error("Not implemented uuid generator.")
	#endif
}

#if _WIN32
void Guid::hton(ref<u8[16]> dst_) noexcept {
	(*std::bit_cast<u32*>(&dst_[0])) = ::htonl(pre);
	(*std::bit_cast<u16*>(&dst_[4])) = ::htons(c0);
	(*std::bit_cast<u16*>(&dst_[6])) = ::htons(c1);
	(*std::bit_cast<u64*>(&dst_[8])) = ::htonll(post);
}

Guid Guid::ntoh(cref<u8[16]> src_) noexcept {
	return {
		::ntohl(*std::bit_cast<u32*>(&src_[0])),
		::ntohs(*std::bit_cast<u16*>(&src_[4])),
		::ntohs(*std::bit_cast<u16*>(&src_[6])),
		::ntohll(*std::bit_cast<u64*>(&src_[8]))
	};
}

#else

void Guid::hton(ref<u8[16]> dst_) noexcept {
	// TODO!
	std::unreachable();
}

Guid Guid::ntoh(cref<u8[16]> src_) noexcept {
	// TODO!
	std::unreachable();
}

#endif
