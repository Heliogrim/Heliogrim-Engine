#include "Murmur3.hpp"

using namespace hg::hash;
using namespace hg;

#ifdef _MSC_VER

#include <cstdlib>
#define ROTL32(x,y) _rotl(x,y)
#define ROTL64(x,y) _rotl64(x,y)

#else

static u32 rotl32(u32 v_, s8 r_) {
	return (v_ << r_) | (v_ >> (s8 {32} - r_));
}

static u64 rotl64(u64 v_, s8 r_) {
	return (v_ << r_) | (v_ >> (s8 {64} - r_));
}

#define ROTL32(x,y) rotl32(x,y)
#define ROTL64(x,y) rotl64(x,y)

#endif

static u32 get_block_32(const u32* ptr_, const u32 idx_) {
	return ptr_[idx_];
}

static u64 get_block_64(const u64* ptr_, const u32 idx_) {
	return ptr_[idx_];
}

static u32 fmix_32(u32 value_) {
	value_ ^= value_ >> 16;
	value_ *= 0x85ebca6buL;
	value_ ^= value_ >> 13;
	value_ *= 0xc2b2ae35uL;
	value_ ^= value_ >> 16;

	return value_;
}

static u64 fmix_64(u64 value_) {
	value_ ^= value_ >> 33;
	value_ *= 0xff51afd7ed558ccduLL;
	value_ ^= value_ >> 33;
	value_ *= 0xc4ceb9fe1a85ec53uLL;
	value_ ^= value_ >> 33;

	return value_;
}

void hash::murmur3_x86_32(const void* data_, const u32 length_, const u32 seed_, u32* dst_) {
	const u8* block = (const u8*)data_;
	const u32 nblocks = length_ / 4L;

	u32 h1 = seed_;

	constexpr u32 c1 = 0xcc9e2d51uL;
	constexpr u32 c2 = 0x1b873593uL;

	const u32* blocks = (const u32*)(block + nblocks * 4);

	for (u32 i = 0; i < nblocks; ++i) {
		u32 k1 = get_block_32(blocks, i);

		k1 *= c1;
		k1 = ROTL32(k1, 15);
		k1 *= c2;

		h1 ^= k1;
		h1 = ROTL32(h1, 13);
		h1 = h1 * 5 + 0xe6546b64uL;
	}

	const u8* tail = (const u8*)(block + nblocks * 4);

	u32 k1 = 0;

	switch (length_ & 3) {
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0] << 0;

			k1 *= c1;
			k1 = ROTL32(k1, 15);
			k1 *= c2;
			h1 ^= k1;
	}

	h1 ^= length_;
	h1 = fmix_32(h1);

	*dst_ = h1;
}

void hash::murmur3_x64_64(const void* data_, const u32 length_, const u32 seed_, u64& dst_) {
	u64 tmp[2];
	murmur3_x64_128(data_, length_, seed_, tmp);
	dst_ = tmp[0];
}

void hash::murmur3_x64_128(const void* data_, const u32 length_, const u64 seed_, void* dst_) {
	const u8* block = (const u8*)data_;
	const u32 nblocks = length_ / 16uL;

	u64 h1 = seed_;
	u64 h2 = seed_;

	constexpr u64 c1 = 0x87c37b91114253d5uLL;
	constexpr u64 c2 = 0x4cf5ad432745937fuLL;

	const u64* blocks = (const u64*)(block);

	for (u32 i = 0; i < nblocks; ++i) {
		u64 k1 = get_block_64(blocks, i * 2);
		u64 k2 = get_block_64(blocks, i * 2 + 1);

		k1 *= c1;
		k1 = ROTL64(k1, 31);
		k1 *= c2;
		h1 ^= k1;

		h1 = ROTL64(h1, 27);
		h1 += h2;
		h1 = h1 * 5 + 0x52dce729;

		k2 *= c2;
		k2 = ROTL64(k2, 33);
		k2 *= c1;
		h2 ^= k2;

		h2 = ROTL64(h2, 31);
		h2 += h1;
		h2 = h2 * 5 + 0x38495ab5;
	}

	const u8* tail = (const u8*)(block + nblocks * 16);

	u64 k1 = 0;
	u64 k2 = 0;

	switch (length_ & 15) {
		case 15: k2 ^= static_cast<u64>(tail[14]) << 48;
		case 14: k2 ^= static_cast<u64>(tail[13]) << 40;
		case 13: k2 ^= static_cast<u64>(tail[12]) << 32;
		case 12: k2 ^= static_cast<u64>(tail[11]) << 24;
		case 11: k2 ^= static_cast<u64>(tail[10]) << 16;
		case 10: k2 ^= static_cast<u64>(tail[9]) << 8;
		case 9: k2 ^= static_cast<u32>(tail[8]) << 0;

			k2 *= c2;
			k2 = ROTL64(k2, 33);
			k2 *= c1;
			h2 ^= k2;

		case 8: k1 ^= static_cast<u64>(tail[7]) << 56;
		case 7: k1 ^= static_cast<u64>(tail[6]) << 48;
		case 6: k1 ^= static_cast<u64>(tail[5]) << 40;
		case 5: k1 ^= static_cast<u64>(tail[4]) << 32;
		case 4: k1 ^= static_cast<u64>(tail[3]) << 24;
		case 3: k1 ^= static_cast<u64>(tail[2]) << 16;
		case 2: k1 ^= static_cast<u64>(tail[1]) << 8;
		case 1: k1 ^= static_cast<u64>(tail[0]) << 0;

			k1 *= c1;
			k1 = ROTL64(k1, 31);
			k1 *= c2;
			h1 ^= k1;
	}

	h1 ^= length_;
	h2 ^= length_;

	h1 += h2;
	h2 += h1;

	h1 = fmix_64(h1);
	h2 = fmix_64(h2);

	h1 += h2;
	h2 += h1;

	((u64*)dst_)[0] = h1;
	((u64*)dst_)[1] = h2;
}
