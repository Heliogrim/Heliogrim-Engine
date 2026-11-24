#include "pch.h"

/**/
#include <array>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

import Engine.Resource.Archive;
import Engine.Serialization;

/**/

using namespace hg::engine::resource;
using namespace hg::engine::serialization;
using namespace hg;

namespace BoundedReaderModule {
	using raw_bytes = std::vector<u8>;

	void putS64(ref<raw_bytes> out_, const s64 value_) {
		u8 tmp[sizeof(s64)];
		std::memcpy(tmp, &value_, sizeof(s64));
		out_.insert(out_.end(), tmp, tmp + sizeof(s64));
	}

	raw_bytes str(const std::string& value_, const s64 size_) {
		raw_bytes out { 0x0F };
		putS64(out, size_);
		out.insert(out.end(), value_.begin(), value_.end());
		return out;
	}

	raw_bytes str(const std::string& value_) {
		return str(value_, static_cast<s64>(value_.size()));
	}

	raw_bytes structOf(const std::vector<std::pair<std::string, raw_bytes>>& entries_) {
		raw_bytes payload {};
		for (const auto& [key, value] : entries_) {
			const auto k = str(key);
			payload.insert(payload.end(), k.begin(), k.end());
			payload.insert(payload.end(), value.begin(), value.end());
		}
		raw_bytes out { 0xB0 };
		putS64(out, static_cast<s64>(payload.size()));
		out.insert(out.end(), payload.begin(), payload.end());
		return out;
	}

	/* Owns the bytes for the whole lifetime of the archive. */
	struct Fixture {
		explicit Fixture(cref<raw_bytes> raw_) :
			bytes(raw_.size()),
			view(bytes),
			archive(view),
			ctx(raw_.size()) {
			std::memcpy(bytes.data(), raw_.data(), raw_.size());
		}

		Vector<_::byte> bytes;
		std::span<_::byte> view;
		MemoryReadonlyArchive archive;
		LoadContext ctx;

		[[nodiscard]] StructScopedSlot root() {
			return RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx }.asStruct();
		}

		[[nodiscard]] ReadError::Kind kind() const {
			return ctx.firstError() ? ctx.firstError()->kind : ReadError::Kind::eMigrationFailed;
		}
	};

	TEST(BoundedReader, HappyPathAndWidening) {
		Fixture fx { structOf({ { "name", str("abc") }, { "v", raw_bytes { 0x04, 7, 0 } } }) };
		const auto root = fx.root();

		String name {};
		root.getSlot<String>("name") >> name;
		u64 wide = 0;
		root.getSlot<u64>("v") >> wide;

		EXPECT_FALSE(fx.ctx.failed());
		EXPECT_EQ(name, "abc");
		EXPECT_EQ(wide, 7uLL);
	}

	TEST(BoundedReader, NegativeStringSizeIsMalformed) {
		Fixture fx { structOf({ { "name", str("", -1) } }) };
		String name { "keep" };
		fx.root().getSlot<String>("name") >> name;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(name, "keep");
	}

	TEST(BoundedReader, TagZeroIsMalformed) {
		Fixture fx { structOf({ { "v", raw_bytes { 0x00, 0 } } }) };
		u16 v = 42;
		fx.root().getSlot<u16>("v") >> v;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(v, 42u);
	}

	TEST(BoundedReader, IntegralFromStringIsTypeMismatch) {
		Fixture fx { structOf({ { "v", str("abcd") } }) };
		u32 v = 42;
		fx.root().getSlot<u32>("v") >> v;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eTypeMismatch);
		EXPECT_EQ(v, 42u);
	}

	TEST(BoundedReader, BoolByteTwoIsInvalid) {
		Fixture fx { structOf({ { "b", raw_bytes { 0x01, 2 } } }) };
		bool b = false;
		fx.root().getSlot<bool>("b") >> b;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eInvalidValue);
		EXPECT_FALSE(b);
	}

	TEST(BoundedReader, DuplicateKey) {
		Fixture fx { structOf({ { "v", raw_bytes { 0x04, 1, 0 } }, { "v", raw_bytes { 0x04, 2, 0 } } }) };
		u16 v = 42;
		fx.root().getSlot<u16>("v") >> v;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eDuplicateKey);
		EXPECT_EQ(v, 42u);
	}

	TEST(BoundedReader, MissingKeyKeepsDefault) {
		Fixture fx { structOf({ { "v", raw_bytes { 0x04, 1, 0 } } }) };
		u16 v = 42;
		fx.root().getSlot<u16>("nope") >> v;
		EXPECT_FALSE(fx.ctx.failed());
		EXPECT_EQ(v, 42u);
	}

	TEST(BoundedReader, NullContextDoesNotCrash) {
		const auto raw = structOf({ { "name", str("", -1) } });
		Vector<_::byte> bytes(raw.size());
		std::memcpy(bytes.data(), raw.data(), raw.size());
		std::span<_::byte> view { bytes };
		MemoryReadonlyArchive archive { view };

		String name { "keep" };
		RootScopedSlot { archive, ScopedSlotStateFlag::eClean }.asStruct().getSlot<String>("name") >> name;
		EXPECT_EQ(name, "keep");
	}

	raw_bytes nested(const u32 depth_) {
		return depth_ == 1 ? structOf({}) : structOf({ { "a", nested(depth_ - 1) } });
	}

	TEST(BoundedReader, ValidateDepth) { {
			Fixture fx { nested(64) };
			EXPECT_TRUE(validate(fx.archive, 0, fx.ctx).has_value());
		} {
			Fixture fx { nested(65) };
			EXPECT_FALSE(validate(fx.archive, 0, fx.ctx).has_value());
			EXPECT_EQ(fx.kind(), ReadError::Kind::eDepth);
		}
	}

	TEST(BoundedReader, MemoryArchiveReadPastEndSetsError) {
		Vector<_::byte> bytes(4);
		std::span<_::byte> view { bytes };
		MemoryReadonlyArchive archive { view };
		u64 v = 0;
		archive >> v;
		EXPECT_TRUE(archive.hasError());
	}
}

namespace BoundedReaderModule {
	TEST(BoundedReader, NonEmptyRecordTagIsMalformed) {
		raw_bytes record { 0xA0 };
		putS64(record, 1);
		record.push_back(0);
		Fixture fx { structOf({ { "v", record } }) };
		u16 v = 42;
		fx.root().getSlot<u16>("v") >> v;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(v, 42u);
	}

	TEST(BoundedReader, EmptyRecordPlaceholderRoundTrips) {
		BufferArchive archive {}; {
			StructuredArchive writer { archive };
			auto slot = writer.insertRootSlot().intoStruct();
			std::ignore = slot.insertRecordSlot("known");
		}
		archive.seek(0);

		LoadContext ctx { archive.totalSize() };
		const auto root = RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx }.asStruct();
		EXPECT_TRUE(root.hasRecordSlot("known"));
		EXPECT_FALSE(ctx.failed());
	}
}

namespace BoundedReaderModule {
	template <typename Type_, typename...>
	using Array3 = std::array<Type_, 3>;

	raw_bytes seqOfU64Count(const u64 count_) {
		raw_bytes out { 0xC0 };
		putS64(out, static_cast<s64>(sizeof(u64)));
		u8 tmp[sizeof(u64)];
		std::memcpy(tmp, &count_, sizeof(u64));
		out.insert(out.end(), tmp, tmp + sizeof(u64));
		return out;
	}

	raw_bytes sliceOfU32(const u64 storedCount_, const u32 entries_) {
		raw_bytes payload {};
		u8 tmp[sizeof(u64)];
		std::memcpy(tmp, &storedCount_, sizeof(u64));
		payload.insert(payload.end(), tmp, tmp + sizeof(u64));
		for (u32 i = 0; i < entries_; ++i) {
			payload.push_back(0x06);
			const u32 value = 100u + i;
			std::memcpy(tmp, &value, sizeof(u32));
			payload.insert(payload.end(), tmp, tmp + sizeof(u32));
		}
		raw_bytes out { 0x10 };
		putS64(out, static_cast<s64>(payload.size()));
		out.insert(out.end(), payload.begin(), payload.end());
		return out;
	}

	TEST(BoundedReader, Int64MaxStringSizeIsMalformed) {
		Fixture fx { structOf({ { "name", str("", (std::numeric_limits<s64>::max)()) } }) };
		String name { "keep" };
		fx.root().getSlot<String>("name") >> name;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(name, "keep");
	}

	TEST(BoundedReader, GiBStringInSmallArchiveIsMalformed) {
		Fixture fx { structOf({ { "name", str("abc", 1LL << 30) } }) };
		ASSERT_LE(fx.bytes.size(), 100uLL);
		String name { "keep" };
		fx.root().getSlot<String>("name") >> name;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(name, "keep");
	}

	TEST(BoundedReader, StringOverBudgetIsBudget) {
		Fixture fx { structOf({ { "s", str("abcdefgh") } }) };
		ASSERT_TRUE(fx.ctx.charge(LoadContext::minBudget - 4uLL));
		String value { "keep" };
		fx.root().getSlot<String>("s") >> value;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eBudget);
		EXPECT_EQ(value, "keep");
	}

	TEST(BoundedReader, SeqCountUint64MaxIsMalformed) {
		Fixture fx { structOf({ { "s", seqOfU64Count(~0uLL) } }) };
		const auto seq = fx.root().getSlot<void>("s").asSeq();
		EXPECT_LT(seq.getRecordCount(), 0LL);
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);

		u16 v = 42;
		seq.getRecordSlot(0).asStruct().getSlot<u16>("x") >> v;
		EXPECT_EQ(v, 42u);
	}

	TEST(BoundedReader, SeqIndexPastCountReadsEmpty) {
		raw_bytes seq = seqOfU64Count(1uLL);
		const auto record = structOf({ { "x", raw_bytes { 0x04, 7, 0 } } });
		seq.insert(seq.end(), record.begin(), record.end());
		const s64 size = static_cast<s64>(seq.size() - 9uLL);
		std::memcpy(seq.data() + 1, &size, sizeof(s64));

		Fixture fx { structOf({ { "s", seq } }) };
		const auto slot = fx.root().getSlot<void>("s").asSeq();
		ASSERT_EQ(slot.getRecordCount(), 1LL);

		u16 v = 42;
		slot.getRecordSlot(5).asStruct().getSlot<u16>("x") >> v;
		EXPECT_EQ(v, 42u);
		slot.getRecordSlot(0).asStruct().getSlot<u16>("x") >> v;
		EXPECT_EQ(v, 7u);
	}

	TEST(BoundedReader, ChildLargerThanParentIsMalformed) {
		raw_bytes inner { 0xB0 };
		putS64(inner, 40);
		auto bytes = structOf({ { "inner", inner } });
		bytes.resize(bytes.size() + 64uLL, 0);

		Fixture fx { bytes };
		u16 v = 42;
		fx.root().getSlot<u16>("inner") >> v;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(v, 42u);
	}

	TEST(BoundedReader, UnknownTagIsSkipped) {
		raw_bytes unknown { 0x7E };
		putS64(unknown, 2);
		unknown.push_back(0xAA);
		unknown.push_back(0xBB);
		Fixture fx { structOf({ { "x", unknown }, { "v", raw_bytes { 0x04, 7, 0 } } }) };
		u16 v = 0;
		fx.root().getSlot<u16>("v") >> v;
		EXPECT_FALSE(fx.ctx.failed());
		EXPECT_EQ(v, 7u);
	}

	TEST(BoundedReader, NonStringKeyIsMalformed) {
		raw_bytes payload { 0x04, 1, 0, 0x04, 7, 0 };
		raw_bytes bytes { 0xB0 };
		putS64(bytes, static_cast<s64>(payload.size()));
		bytes.insert(bytes.end(), payload.begin(), payload.end());

		Fixture fx { bytes };
		u16 v = 42;
		fx.root().getSlot<u16>("v") >> v;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(v, 42u);
	}

	TEST(BoundedReader, FixedArrayWithMoreStoredIsMalformed) {
		Fixture fx { structOf({ { "a", sliceOfU32(4uLL, 4u) } }) };
		Array3<u32> target { 1u, 2u, 3u };
		fx.root().getSlot<void>("a").intoSlice<u32, Array3>() >> target;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(target, (Array3<u32> { 1u, 2u, 3u }));
	}

	TEST(BoundedReader, FixedArrayWithFewerStoredIsMalformed) {
		Fixture fx { structOf({ { "a", sliceOfU32(2uLL, 2u) } }) };
		Array3<u32> target { 1u, 2u, 3u };
		fx.root().getSlot<void>("a").intoSlice<u32, Array3>() >> target;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eMalformed);
		EXPECT_EQ(target, (Array3<u32> { 1u, 2u, 3u }));
	}

	TEST(BoundedReader, TruncatedMidHeaderIsTruncated) {
		auto bytes = structOf({ { "v", raw_bytes { 0x0F, 3, 0, 0 } } });

		Fixture fx { bytes };
		String v { "keep" };
		fx.root().getSlot<String>("v") >> v;
		EXPECT_EQ(fx.kind(), ReadError::Kind::eTruncated);
		EXPECT_EQ(v, "keep");
	}

	TEST(BoundedReader, EmptyStructTypeKeyKeepsDefault) {
		Fixture fx { structOf({}) };
		u64 type = 42uLL;
		fx.root().getSlot<u64>("@type") >> type;
		EXPECT_FALSE(fx.ctx.failed());
		EXPECT_EQ(type, 42uLL);
	}

	[[nodiscard]] String validateDetail(cref<raw_bytes> raw_, const ReadError::Kind kind_) {
		Fixture fx { raw_ };
		const auto result = validate(fx.archive, 0, fx.ctx);
		EXPECT_FALSE(result.has_value());
		EXPECT_EQ(fx.kind(), kind_);
		if (result.has_value()) {
			return {};
		}
		EXPECT_EQ(result.error().kind, kind_);
		return result.error().detail;
	}

	TEST(BoundedReader, ValidateAcceptsContainers) {
		auto map = sliceOfU32(1uLL, 0u);
		map[0] = 0x20;
		raw_bytes entry { 0x30 };
		putS64(entry, 10);
		entry.insert(entry.end(), { 0x06, 1, 0, 0, 0, 0x06, 2, 0, 0, 0 });
		map.insert(map.end(), entry.begin(), entry.end());
		const s64 mapSize = static_cast<s64>(map.size() - 9uLL);
		std::memcpy(map.data() + 1, &mapSize, sizeof(s64));

		raw_bytes emptySeq { 0xC0 };
		putS64(emptySeq, 0);

		raw_bytes seq = seqOfU64Count(1uLL);
		const auto record = structOf({ { "x", raw_bytes { 0x04, 7, 0 } } });
		seq.insert(seq.end(), record.begin(), record.end());
		const s64 seqSize = static_cast<s64>(seq.size() - 9uLL);
		std::memcpy(seq.data() + 1, &seqSize, sizeof(s64));

		Fixture fx { structOf({ { "a", sliceOfU32(2uLL, 2u) }, { "m", map }, { "s", emptySeq }, { "q", seq }, { "n", str("x") } }) };
		EXPECT_TRUE(validate(fx.archive, 0, fx.ctx).has_value());
		EXPECT_FALSE(fx.ctx.failed());
	}

	TEST(BoundedReader, ValidateCountTooSmall) {
		raw_bytes slice { 0x10 };
		putS64(slice, 4);
		slice.insert(slice.end(), { 0, 0, 0, 0 });
		EXPECT_EQ(validateDetail(structOf({ { "a", slice } }), ReadError::Kind::eMalformed), "container too small for its count");
	}

	TEST(BoundedReader, ValidateCountExceedsBytes) {
		EXPECT_EQ(
			validateDetail(structOf({ { "a", sliceOfU32(11uLL, 2u) } }), ReadError::Kind::eMalformed),
			"container count exceeds its bytes"
		);
	}

	TEST(BoundedReader, ValidateNonStringKey) {
		raw_bytes payload { 0x04, 1, 0, 0x04, 7, 0 };
		raw_bytes bytes { 0xB0 };
		putS64(bytes, static_cast<s64>(payload.size()));
		bytes.insert(bytes.end(), payload.begin(), payload.end());
		EXPECT_EQ(validateDetail(bytes, ReadError::Kind::eMalformed), "struct key is not a string");
	}

	TEST(BoundedReader, ValidateTruncatedKeyHeader) {
		raw_bytes bytes { 0xB0 };
		putS64(bytes, 4);
		bytes.insert(bytes.end(), { 0x0F, 3, 0, 0 });
		EXPECT_EQ(validateDetail(bytes, ReadError::Kind::eTruncated), "slot size crosses its parent");
	}

	TEST(BoundedReader, ValidateDuplicateKey) {
		const auto bytes = structOf({ { "v", raw_bytes { 0x04, 1, 0 } }, { "v", raw_bytes { 0x04, 2, 0 } } });
		EXPECT_EQ(validateDetail(bytes, ReadError::Kind::eDuplicateKey), "duplicate struct key");
	}

	TEST(BoundedReader, ValidateTruncatedChild) {
		auto bytes = structOf({ { "v", raw_bytes { 0x0F, 3, 0, 0 } } });
		EXPECT_EQ(validateDetail(bytes, ReadError::Kind::eTruncated), "slot size crosses its parent");
	}

	TEST(BoundedReader, RecordPlaceholderTypedReadKeepsDefault) {
		raw_bytes record { 0xA0 };
		putS64(record, 0);
		Fixture fx { structOf({ { "p", record } }) };
		u64 v = 42uLL;
		fx.root().getSlot<u64>("p") >> v;
		EXPECT_FALSE(fx.ctx.failed());
		EXPECT_EQ(v, 42uLL);
	}
}
