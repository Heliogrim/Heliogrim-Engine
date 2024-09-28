#include "pch.h"

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>

using namespace hg::engine::resource;
using namespace hg;

namespace ResourceArchiveModule {
	template <typename ValueType_>
	void simpleBufferTest() {

		BufferArchive archive {};

		using type = ValueType_;
		const type testValue { static_cast<type>(0xA2A2'A2A2'A2A2'A2A2) };

		type write { testValue };
		archive << write;

		archive.seek(0);

		type read { 0 };
		archive >> read;

		EXPECT_EQ(testValue, read);
	}

	TEST(BufferArchive, ReadWriteByte) {
		simpleBufferTest<u8>();
	}

	TEST(BufferArchive, ReadWriteU16) {
		simpleBufferTest<u16>();
	}

	TEST(BufferArchive, ReadWriteU32) {
		simpleBufferTest<u32>();
	}

	TEST(BufferArchive, ReadWriteU64) {
		simpleBufferTest<u64>();
	}

	TEST(BufferArchive, ReadWriteFloat) {
		simpleBufferTest<float>();
	}

	TEST(BufferArchive, ReadWriteDouble) {
		simpleBufferTest<double>();
	}

	TEST(BufferArchive, ReadWriteBool) {

		BufferArchive archive {};

		bool write { false };
		archive << write;

		archive.seek(0);

		bool read { true };
		archive >> read;

		EXPECT_FALSE(read);
		archive.seek(0);

		/**/

		write = true;
		archive << write;

		archive.seek(0);

		read = false;
		archive >> read;

		EXPECT_TRUE(read);
	}

	enum class EnumValueType {
		eEntryZero,
		eEntryOne,
		eEntryTwo
	};

	enum class EnumValueU8 : u8 {
		eEntryZero = 0x0u,
		eEntryOne = 0x1u,
		eEntryTwo = 0x2u
	};

	enum class EnumValueU16 : u16 {
		eEntryZero = 0xFF00uL,
		eEntryOne = 0xFF01uL,
		eEntryTwo = 0xFF02uL
	};

	enum class EnumValueU32 : u32 {
		eEntryZero = 0xFFFF'0000uL,
		eEntryOne = 0xFFFF'0001uL,
		eEntryTwo = 0xFFFF'0002uL
	};

	enum class EnumValueU64 : u64 {
		eEntryZero = 0xFFFF'0000'0000'0000uLL,
		eEntryOne = 0xFFFF'0000'0000'0001uLL,
		eEntryTwo = 0xFFFF'0000'0000'0002uLL
	};

	template <typename EnumType>
	void simpleEnumTest() {

		BufferArchive archive {};

		using type = EnumType;

		type write { EnumType::eEntryTwo };
		archive << write;

		archive.seek(0);

		type read { EnumType::eEntryZero };
		archive >> read;

		EXPECT_EQ(read, EnumType::eEntryTwo);
		archive.seek(0);

		/**/

		Array<type, 3> writeN { EnumType::eEntryOne, EnumType::eEntryTwo, EnumType::eEntryZero };
		archive << writeN[0];
		archive << writeN[1];
		archive << writeN[2];

		archive.seek(0);
		read = EnumType::eEntryZero;

		archive >> read;
		EXPECT_EQ(read, EnumType::eEntryOne);

		archive >> read;
		EXPECT_EQ(read, EnumType::eEntryTwo);

		archive >> read;
		EXPECT_EQ(read, EnumType::eEntryZero);

	}

	TEST(BufferArchive, ReadWriteEnumType) {
		simpleEnumTest<EnumValueType>();
	}

	TEST(BufferArchive, ReadWriteEnumU8) {
		simpleEnumTest<EnumValueU8>();
	}

	TEST(BufferArchive, ReadWriteEnumU16) {
		simpleEnumTest<EnumValueU16>();
	}

	TEST(BufferArchive, ReadWriteEnumU32) {
		simpleEnumTest<EnumValueU32>();
	}

	TEST(BufferArchive, ReadWriteEnumU64) {
		simpleEnumTest<EnumValueU64>();
	}
}
