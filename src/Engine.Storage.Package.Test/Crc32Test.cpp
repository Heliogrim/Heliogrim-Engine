#include "pch.h"

/**/

import Engine.Common;
import Engine.Crypto;

using namespace hg::engine::crypto;
using namespace hg;

namespace Crc32Module {
	namespace {
		const auto table = [] {
			std::array<u32, 256uLL> result {};
			for (u32 i = 0; i < 256uL; ++i) {
				u32 value = i;
				for (auto bit = 0; bit < 8; ++bit) {
					value = (value & 1uL) ? (0xEDB88320uL ^ (value >> 1)) : (value >> 1);
				}
				result[i] = value;
			}
			return result;
		}();

		u32 advance(u32 state_, const u8 byte_) {
			return table[(state_ ^ byte_) & 0xFFuL] ^ (state_ >> 8);
		}

		u32 reference(const std::span<const u8> bytes_) {
			u32 state = 0xFFFFFFFFuL;
			for (const auto byte : bytes_) {
				state = advance(state, byte);
			}
			return state ^ 0xFFFFFFFFuL;
		}

		std::span<const _::byte> asBytes(const std::span<const u8> bytes_) {
			return { reinterpret_cast<ptr<const _::byte>>(bytes_.data()), bytes_.size() };
		}
	}

	TEST(Crc32, KnownVector) {
		constexpr std::array<u8, 9uLL> text { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

		auto crc = Crc32 {};
		crc.update(asBytes(text));
		EXPECT_EQ(std::move(crc).finalize(), 0xCBF43926uL);
	}

	TEST(Crc32, SplitEqualsWhole) {
		std::array<u8, 64uLL> data {};
		for (auto i = 0uLL; i < data.size(); ++i) {
			data[i] = static_cast<u8>(i * 37uLL + 11uLL);
		}

		const auto expected = reference(data);
		for (auto split = 0uLL; split <= data.size(); ++split) {
			auto crc = Crc32 {};
			crc.update(asBytes(std::span<const u8> { data }.first(split)));
			crc.update(asBytes(std::span<const u8> { data }.subspan(split)));
			EXPECT_EQ(std::move(crc).finalize(), expected) << "split at " << split;
		}
	}

	TEST(Crc32, ContinuesThroughZero) {

		// Note: Four bytes map bijectively onto the crc, so exactly one prefix has the crc zero.
		// Note: Given the first three bytes, the fourth is determined by the top byte of the final state.
		u8 topIndex = 0;
		for (auto i = 0uLL; i < 256uLL; ++i) {
			if ((table[i] >> 24) == 0xFFuL) { topIndex = static_cast<u8>(i); }
		}

		std::array<u8, 64uLL> data {};
		bool found = false;
		for (u32 head = 0; head < (1uL << 24) && not found; ++head) {
			const u8 b0 = static_cast<u8>(head), b1 = static_cast<u8>(head >> 8), b2 = static_cast<u8>(head >> 16);
			const auto state = advance(advance(advance(0xFFFFFFFFuL, b0), b1), b2);
			const u8 b3 = static_cast<u8>(topIndex ^ (state & 0xFFuL));
			if (advance(state, b3) == 0xFFFFFFFFuL) {
				data[0] = b0;
				data[1] = b1;
				data[2] = b2;
				data[3] = b3;
				found = true;
			}
		}
		ASSERT_TRUE(found);

		for (auto i = 4uLL; i < data.size(); ++i) {
			data[i] = static_cast<u8>(i + 1uLL);
		}

		/**/

		auto prefix = Crc32 {};
		prefix.update(asBytes(std::span<const u8> { data }.first(4uLL)));
		ASSERT_EQ(std::move(prefix).finalize(), 0uL);

		auto crc = Crc32 {};
		crc.update(asBytes(std::span<const u8> { data }.first(4uLL)));
		crc.update(asBytes(std::span<const u8> { data }.subspan(4uLL)));
		EXPECT_EQ(std::move(crc).finalize(), reference(data));
	}

	TEST(Crc32, EmptyUpdateKeepsRunningValue) {
		constexpr std::array<u8, 9uLL> text { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

		auto crc = Crc32 {};
		crc.update({});
		crc.update(asBytes(std::span<const u8> { text }.first(4uz)));
		crc.update({});
		crc.update(asBytes(std::span<const u8> { text }.subspan(4uz)));
		crc.update({});
		EXPECT_EQ(std::move(crc).finalize(), 0xCBF43926uL);
	}

	TEST(Crc32, LargeUnalignedBufferMatchesReference) {
		// Note: Odd length from an odd offset exercises the word-wise path and its byte tails.
		auto data = std::vector<u8>((1uz << 20) + 7uz);
		for (auto i = 0uz; i < data.size(); ++i) {
			data[i] = static_cast<u8>((i * 2654435761uLL) >> 13);
		}
		const auto bytes = std::span<const u8> { data }.subspan(1uz);

		auto crc = Crc32 {};
		crc.update(asBytes(bytes.first(333uz)));
		crc.update(asBytes(bytes.subspan(333uz)));
		EXPECT_EQ(std::move(crc).finalize(), reference(bytes));
	}
}
