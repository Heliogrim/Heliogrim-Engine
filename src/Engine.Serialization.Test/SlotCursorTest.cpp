#include "pch.h"

/**/

#include <cstring>
#include <limits>
#include <span>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Resource.Archive/MemoryReadonlyArchive.hpp>
#include <Engine.Serialization/Structure/SlotCursor.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::resource;
using namespace hg;

namespace HgTestSlotCursor {
	namespace {
		/* tag, s64 size, `size_` payload bytes (zeroed) */
		Vector<_::byte> stringSlot(const s64 declared_, const size_t payload_) {
			Vector<_::byte> bytes(1uLL + sizeof(s64) + payload_);
			bytes[0] = static_cast<_::byte>(static_cast<u8>(StructureSlotType::eString));
			std::memcpy(bytes.data() + 1, &declared_, sizeof(s64));
			return bytes;
		}
	}

	TEST(SlotCursor, FitsBoundaries) {
		constexpr SlotCursor cursor { 8, 16 };
		EXPECT_TRUE(cursor.fits(8, 0));
		EXPECT_TRUE(cursor.fits(8, 8));
		EXPECT_TRUE(cursor.fits(16, 0));
		EXPECT_FALSE(cursor.fits(8, 9));
		EXPECT_FALSE(cursor.fits(7, 1));
		EXPECT_FALSE(cursor.fits(17, 0));
		EXPECT_FALSE(cursor.fits(8, -1));
	}

	TEST(SlotCursor, FitsRejectsOverflowingLengths) {
		constexpr SlotCursor cursor { 0, 16 };
		EXPECT_FALSE(cursor.fits(8, (std::numeric_limits<s64>::max)()));
		EXPECT_FALSE(cursor.fits((std::numeric_limits<s64>::max)(), 1));
		EXPECT_FALSE(cursor.fits(0, (std::numeric_limits<s64>::min)()));
	}

	TEST(SlotCursor, ReadHeaderAtExactBoundary) {
		auto bytes = stringSlot(4, 4);
		std::span<_::byte> view { bytes };
		MemoryReadonlyArchive archive { view };

		StructureSlotHeader header {};
		const auto error = readSlotHeader(archive, SlotCursor { 0, static_cast<s64>(bytes.size()) }, 0, header);
		EXPECT_FALSE(error.has_value());
		EXPECT_EQ(header.type, StructureSlotType::eString);
		EXPECT_EQ(header.size, 4);
	}

	TEST(SlotCursor, ReadHeaderOneByteShort) {
		auto bytes = stringSlot(4, 4);
		std::span<_::byte> view { bytes };
		MemoryReadonlyArchive archive { view };

		StructureSlotHeader header {};
		const auto error = readSlotHeader(archive, SlotCursor { 0, static_cast<s64>(bytes.size()) - 1 }, 0, header);
		ASSERT_TRUE(error.has_value());
		EXPECT_EQ(error->kind, ReadError::Kind::eMalformed);
		EXPECT_EQ(header.type, StructureSlotType::eUndefined);
	}

	TEST(SlotCursor, ReadHeaderRejectsOverflowingAndNegativeLengths) {
		for (const s64 declared : { (std::numeric_limits<s64>::max)(), (std::numeric_limits<s64>::max)() - 8, s64 { -1 } }) {
			auto bytes = stringSlot(declared, 4);
			std::span<_::byte> view { bytes };
			MemoryReadonlyArchive archive { view };

			StructureSlotHeader header {};
			const auto error = readSlotHeader(archive, SlotCursor { 0, static_cast<s64>(bytes.size()) }, 0, header);
			ASSERT_TRUE(error.has_value());
			EXPECT_EQ(error->kind, ReadError::Kind::eMalformed);
		}
	}

	TEST(SlotCursor, ReadHeaderTagOutsideCursor) {
		auto bytes = stringSlot(0, 0);
		std::span<_::byte> view { bytes };
		MemoryReadonlyArchive archive { view };

		StructureSlotHeader header {};
		const auto error = readSlotHeader(archive, SlotCursor { 0, 0 }, 0, header);
		ASSERT_TRUE(error.has_value());
		EXPECT_EQ(error->kind, ReadError::Kind::eTruncated);
	}
}
