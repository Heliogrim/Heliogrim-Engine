#include "pch.h"

/**/
#include <vector>
#include <Engine.Diagnostics/Sink.hpp>
#include <Engine.Resource.Archive/Archive.hpp>
#include <Engine.Resource.Archive/ArchiveStreamMode.hpp>
#include <Engine.Resource.Archive/BufferArchive.hpp>

/**/
#include <Engine.Serialization/Archive/StructuredArchive.hpp>
#include <Engine.Serialization/Load/LoadContext.hpp>
#include <Engine.Serialization/Schema/Migration.hpp>
#include <Engine.Serialization/Structure/FloatScopedSlot.hpp>
#include <Engine.Serialization/Structure/IntegralScopedSlot.hpp>
#include <Engine.Serialization/Structure/RootScopedSlot.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::resource;
using namespace hg::engine::serialization;
using namespace hg::engine::diag;
using namespace hg;

namespace {
	constexpr type_id heroType = "Hero"_typeId;

	void writeHeroFields(ref<StructScopedSlot> slot_, const u16 rev_) {
		if (rev_ > 0) {
			slot_.insertSlot<u16>("@rev") << rev_;
		}
		slot_.insertSlot<String>("name") << String { "abc" };
		slot_.insertSlot<u32>("hp") << 7u;
		{
			auto pos = slot_.insertStructSlot("pos");
			pos.insertSlot<float>("x") << 1.5F;
		}
	}

	[[nodiscard]] BufferArchive writeHero(const u16 rev_) {
		BufferArchive archive {};
		{
			StructuredArchive writer { archive };
			auto slot = writer.insertRootSlot().intoStruct();
			writeHeroFields(slot, rev_);
		}
		return archive;
	}

	[[nodiscard]] std::vector<u8> bytesOf(ref<Archive> archive_) {
		std::vector<u8> out(static_cast<size_t>(archive_.totalSize()));
		archive_.seek(0);
		archive_.serializeBytes(out.data(), out.size(), ArchiveStreamMode::eLoad);
		return out;
	}

	struct Loaded {
		explicit Loaded(mref<BufferArchive> archive_) :
			archive(std::move(archive_)),
			ctx(static_cast<u64>(archive.totalSize())) {
			archive.seek(0);
		}

		BufferArchive archive;
		LoadContext ctx;

		[[nodiscard]] StructScopedSlot root() {
			return RootScopedSlot { archive, ScopedSlotStateFlag::eClean, &ctx }.asStruct();
		}
	};

	Result<void, MigrateError> renameName(ref<const StructScopedSlot> old_, ref<StructScopedSlot> new_) {
		forwardEntries(old_, new_, { "name" });
		String name {};
		old_.getSlot<String>("name") >> name;
		new_.insertSlot<String>("label") << name;
		return {};
	}

	Result<void, MigrateError> forwardAll(ref<const StructScopedSlot> old_, ref<StructScopedSlot> new_) {
		forwardEntries(old_, new_, {});
		return {};
	}

	Result<void, MigrateError> failing(ref<const StructScopedSlot>, ref<StructScopedSlot>) {
		return Unexpected<MigrateError> { MigrateError { "boom" } };
	}

}

namespace MigrationModule {
	TEST(Migration, RevisionZeroWritesNothing) {
		MigrationRegistry reg {};
		EXPECT_EQ(reg.revision(heroType), 0u);

		const auto writeWith = [&reg](const bool revision_) {
			BufferArchive archive {};
			{
				StructuredArchive writer { archive };
				auto slot = writer.insertRootSlot().intoStruct();
				if (revision_) {
					reg.writeRevision(slot, heroType);
				}
				slot.insertSlot<u32>("hp") << 7u;
			}
			return archive;
		};

		auto plain = writeWith(false);
		auto tagged = writeWith(true);
		EXPECT_EQ(bytesOf(plain), bytesOf(tagged));

		Loaded loaded { std::move(tagged) };
		EXPECT_FALSE(loaded.root().hasRecordSlot("@rev"));
		EXPECT_EQ(loaded.root().revision(), 0u);

		const Migration steps[] { { heroType, 0, &renameName } };
		reg.add(steps);
		EXPECT_EQ(reg.revision(heroType), 1u);

		Loaded revised { writeWith(true) };
		EXPECT_EQ(revised.root().revision(), 1u);
		EXPECT_FALSE(revised.ctx.failed());
	}

	TEST(Migration, BytesStepRenamesKey) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &renameName } };
		reg.add(steps);

		Loaded loaded { writeHero(0) };
		auto migrated = reg.migrateToCurrent(heroType, loaded.root(), loaded.ctx);
		ASSERT_TRUE(migrated.has_value());
		ASSERT_FALSE(loaded.ctx.failed());

		/* Moving the result must not dangle the slot into the scratch archive. */
		auto moved = std::move(migrated);
		const auto& slot = moved->slot();
		EXPECT_EQ(slot.revision(), 1u);
		EXPECT_FALSE(slot.hasRecordSlot("name"));

		String label {};
		slot.getSlot<String>("label") >> label;
		EXPECT_EQ(label, "abc");
		u32 hp = 0;
		slot.getSlot<u32>("hp") >> hp;
		EXPECT_EQ(hp, 7u);

		/* r1 written natively: @rev, the forwarded entries in order, then the renamed key. */
		BufferArchive expected {};
		{
			StructuredArchive writer { expected };
			auto out = writer.insertRootSlot().intoStruct();
			out.insertSlot<u16>("@rev") << u16 { 1 };
			out.insertSlot<u32>("hp") << 7u;
			{
				auto pos = out.insertStructSlot("pos");
				pos.insertSlot<float>("x") << 1.5F;
			}
			out.insertSlot<String>("label") << String { "abc" };
		}
		EXPECT_EQ(bytesOf(*slot.slot()->getState().root->archive), bytesOf(expected));
	}

	TEST(Migration, NativeStepSeesStoredRevision) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &renameName }, { heroType, 1, nullptr } };
		reg.add(steps);
		EXPECT_EQ(reg.revision(heroType), 2u);

		Loaded r0 { writeHero(0) };
		const auto m0 = reg.migrateToCurrent(heroType, r0.root(), r0.ctx);
		ASSERT_TRUE(m0.has_value());
		EXPECT_EQ(m0->slot().revision(), 1u);
		EXPECT_TRUE(m0->slot().hasRecordSlot("label"));

		Loaded r1 { writeHero(1) };
		const auto m1 = reg.migrateToCurrent(heroType, r1.root(), r1.ctx);
		ASSERT_TRUE(m1.has_value());
		EXPECT_EQ(m1->slot().revision(), 1u);
		EXPECT_TRUE(m1->slot().hasRecordSlot("name"));
		EXPECT_FALSE(r1.ctx.failed());
	}

	TEST(MigrationDeathTest, BytesAfterNativeAsserts) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, nullptr }, { heroType, 1, &renameName } };
		EXPECT_DEATH({ reg.add(steps); }, "MigrationRegistry::add");
	}

	TEST(MigrationDeathTest, GapAsserts) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &renameName }, { heroType, 2, &renameName } };
		EXPECT_DEATH({ reg.add(steps); }, "MigrationRegistry::add");
	}

	TEST(Migration, StepFailureIsMigrationFailed) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &failing } };
		reg.add(steps);

		Loaded loaded { writeHero(0) };
		EXPECT_FALSE(reg.migrateToCurrent(heroType, loaded.root(), loaded.ctx).has_value());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->detail, "boom");

		CollectingSink sink {};
		const auto res = loaded.ctx.finish(sink, "while loading 'Hero'");
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().front().code()->key, "migration-failed");
		EXPECT_EQ(res.error().front().code()->code.number, 14u);
	}

	TEST(Migration, NewerNestedSkippedWithWarning) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &renameName } };
		reg.add(steps);

		BufferArchive archive {};
		{
			StructuredArchive writer { archive };
			auto slot = writer.insertRootSlot().intoStruct();
			auto child = slot.insertStructSlot("child");
			writeHeroFields(child, 5);
		}
		Loaded loaded { std::move(archive) };
		EXPECT_FALSE(reg.migrateToCurrent(heroType, loaded.root().getStructSlot("child"), loaded.ctx).has_value());
		EXPECT_FALSE(loaded.ctx.failed());

		CollectingSink sink {};
		ASSERT_TRUE(loaded.ctx.finish(sink, "while loading 'Hero'").has_value());
		ASSERT_EQ(sink.items().size(), 1uLL);
		EXPECT_EQ(sink.items().front().severity(), Severity::eWarning);
		EXPECT_EQ(sink.items().front().code()->key, "revision-newer");
		EXPECT_EQ(sink.items().front().code()->code.number, 12u);
	}

	TEST(Migration, NewerRootRejected) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &renameName } };
		reg.add(steps);

		Loaded loaded { writeHero(5) };
		EXPECT_FALSE(reg.migrateToCurrent(heroType, loaded.root(), loaded.ctx).has_value());
		ASSERT_TRUE(loaded.ctx.failed());

		CollectingSink sink {};
		const auto res = loaded.ctx.finish(sink, "while loading 'Hero'");
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(res.error().front().code()->key, "revision-newer");
		EXPECT_EQ(res.error().front().code()->code.number, 12u);
		EXPECT_EQ(res.error().front().severity(), Severity::eError);
	}

	TEST(Migration, ScratchChargesBudget) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &forwardAll } };
		reg.add(steps);

		Loaded loaded { writeHero(0) };
		ASSERT_TRUE(loaded.ctx.charge(LoadContext::minBudget - 8uLL));
		EXPECT_FALSE(reg.migrateToCurrent(heroType, loaded.root(), loaded.ctx).has_value());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eBudget);
	}

	TEST(Migration, ReadTrackingKeyedByArchive) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &forwardAll } };
		reg.add(steps);

		Loaded loaded { writeHero(0) };
		const auto real = loaded.root();
		const auto migrated = reg.migrateToCurrent(heroType, real, loaded.ctx);
		ASSERT_TRUE(migrated.has_value());

		const auto& scratchState = migrated->slot().slot()->getState();
		const auto& realState = real.slot()->getState();
		ASSERT_EQ(scratchState.offset, realState.offset);

		const void* scratchId = std::addressof(*scratchState.root->archive);
		const void* realId = std::addressof(*realState.root->archive);
		ASSERT_NE(scratchId, realId);

		loaded.ctx.markRead(realId, realState.offset, "hp");
		const Vector<String> present { "hp" };
		EXPECT_TRUE(loaded.ctx.unreadKeys(realId, realState.offset, present).empty());
		EXPECT_EQ(loaded.ctx.unreadKeys(scratchId, scratchState.offset, present).size(), 1uLL);
	}

	TEST(Migration, ErrorInMigratedRecordReportsOrigin) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &renameName } };
		reg.add(steps);

		BufferArchive archive {};
		{
			StructuredArchive writer { archive };
			auto slot = writer.insertRootSlot().intoStruct();
			slot.insertSlot<u32>("pad") << 1u;
			auto child = slot.insertStructSlot("child");
			writeHeroFields(child, 0);
		}
		Loaded loaded { std::move(archive) };
		const auto child = loaded.root().getStructSlot("child");
		const s64 childOffset = child.slot()->getState().offset;
		ASSERT_GT(childOffset, 0LL);

		const auto migrated = reg.migrateToCurrent(heroType, child, loaded.ctx);
		ASSERT_TRUE(migrated.has_value());

		u32 wrong = 99;
		migrated->slot().getSlot<u32>("label") >> wrong;
		EXPECT_EQ(wrong, 99u);
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eTypeMismatch);
		EXPECT_EQ(loaded.ctx.firstError()->offset, childOffset);
		EXPECT_EQ(loaded.ctx.firstError()->length, child.slot()->payloadEnd() - childOffset);
	}
}

namespace {
	Result<void, MigrateError> readsNameAsInteger(ref<const StructScopedSlot> old_, ref<StructScopedSlot> new_) {
		u32 wrong = 0;
		old_.getSlot<u32>("name") >> wrong;
		new_.insertSlot<u32>("hp") << wrong;
		return {};
	}

	[[nodiscard]] BufferArchive rawArchive(const std::vector<u8>& bytes_) {
		BufferArchive archive {};
		archive.serializeBytes(const_cast<u8*>(bytes_.data()), bytes_.size(), ArchiveStreamMode::eStore);
		archive.seek(0);
		return archive;
	}

	[[nodiscard]] std::vector<u8> structBytes(const std::vector<u8>& payload_) {
		std::vector<u8> out { 0xB0 };
		const s64 size = static_cast<s64>(payload_.size());
		out.insert(out.end(), reinterpret_cast<const u8*>(&size), reinterpret_cast<const u8*>(&size) + sizeof(s64));
		out.insert(out.end(), payload_.begin(), payload_.end());
		return out;
	}

	[[nodiscard]] std::vector<u8> keyBytes(const std::string& key_) {
		std::vector<u8> out { 0x0F };
		const s64 size = static_cast<s64>(key_.size());
		out.insert(out.end(), reinterpret_cast<const u8*>(&size), reinterpret_cast<const u8*>(&size) + sizeof(s64));
		out.insert(out.end(), key_.begin(), key_.end());
		return out;
	}

	/* Runs forwardEntries over a hand-built source record and returns the target bytes. */
	[[nodiscard]] std::vector<u8> forwardFrom(ref<Loaded> source_) {
		BufferArchive target {};
		{
			StructuredArchive writer { target };
			auto out = writer.insertRootSlot().intoStruct();
			forwardEntries(source_.root(), out, {});
		}
		return bytesOf(target);
	}

	[[nodiscard]] std::vector<u8> emptyRecordBytes() {
		BufferArchive empty {};
		{
			StructuredArchive writer { empty };
			std::ignore = writer.insertRootSlot().intoStruct();
		}
		return bytesOf(empty);
	}
}

namespace MigrationModule {
	TEST(Migration, UnreadableRevisionRejected) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &renameName } };
		reg.add(steps);

		BufferArchive archive {};
		{
			StructuredArchive writer { archive };
			auto slot = writer.insertRootSlot().intoStruct();
			slot.insertSlot<String>("@rev") << String { "one" };
			slot.insertSlot<u32>("hp") << 7u;
		}
		Loaded loaded { std::move(archive) };
		EXPECT_FALSE(reg.migrateToCurrent(heroType, loaded.root(), loaded.ctx).has_value());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eTypeMismatch);
	}

	TEST(Migration, StepReadErrorStopsChain) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &readsNameAsInteger }, { heroType, 1, &forwardAll } };
		reg.add(steps);

		Loaded loaded { writeHero(0) };
		EXPECT_FALSE(reg.migrateToCurrent(heroType, loaded.root(), loaded.ctx).has_value());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eTypeMismatch);
	}

	TEST(Migration, TwoBytesStepsChain) {
		MigrationRegistry reg {};
		// Two registrations extend one chain.
		const Migration first[] { { heroType, 0, &renameName } };
		const Migration second[] { { heroType, 1, &forwardAll } };
		reg.add(first);
		reg.add(second);
		EXPECT_EQ(reg.revision(heroType), 2u);

		Loaded loaded { writeHero(0) };
		const auto migrated = reg.migrateToCurrent(heroType, loaded.root(), loaded.ctx);
		ASSERT_TRUE(migrated.has_value());
		ASSERT_FALSE(loaded.ctx.failed());
		EXPECT_EQ(migrated->slot().revision(), 2u);

		String label {};
		migrated->slot().getSlot<String>("label") >> label;
		EXPECT_EQ(label, "abc");
		u32 hp = 0;
		migrated->slot().getSlot<u32>("hp") >> hp;
		EXPECT_EQ(hp, 7u);
		EXPECT_FALSE(loaded.ctx.failed());
	}

	TEST(Migration, ForwardRejectsNonStringKey) {
		Loaded loaded { rawArchive(structBytes({ 0x04, 1, 0, 0x04, 7, 0 })) };
		EXPECT_EQ(forwardFrom(loaded), emptyRecordBytes());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eMalformed);
		EXPECT_EQ(loaded.ctx.firstError()->detail, "struct key is not a string");
	}

	TEST(Migration, ForwardRejectsTruncatedValue) {
		auto payload = keyBytes("v");
		payload.insert(payload.end(), { 0x0F, 3, 0, 0 });
		Loaded loaded { rawArchive(structBytes(payload)) };
		EXPECT_EQ(forwardFrom(loaded), emptyRecordBytes());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eTruncated);
	}

	TEST(Migration, ForwardRejectsTruncatedKeyHeader) {
		Loaded loaded { rawArchive(structBytes({ 0x0F, 3, 0, 0 })) };
		EXPECT_EQ(forwardFrom(loaded), emptyRecordBytes());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eTruncated);
	}

	TEST(Migration, ForwardChargesKeyBudget) {
		Loaded loaded { writeHero(0) };
		ASSERT_TRUE(loaded.ctx.charge(LoadContext::minBudget - 2uLL));
		EXPECT_EQ(forwardFrom(loaded), emptyRecordBytes());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eBudget);
	}

	TEST(Migration, ForwardWithoutContextStaysSilent) {
		auto archive = rawArchive(structBytes({ 0x04, 1, 0, 0x04, 7, 0 }));
		BufferArchive target {};
		{
			StructuredArchive writer { target };
			auto out = writer.insertRootSlot().intoStruct();
			forwardEntries(RootScopedSlot { archive, ScopedSlotStateFlag::eClean }.asStruct(), out, {});
		}
		EXPECT_EQ(bytesOf(target), emptyRecordBytes());
	}

	TEST(Migration, ForwardCopiesEveryEntry) {
		Loaded loaded { writeHero(3) };
		BufferArchive target {};
		{
			StructuredArchive writer { target };
			auto out = writer.insertRootSlot().intoStruct();
			forwardEntries(loaded.root(), out, { "hp" });
		}
		EXPECT_FALSE(loaded.ctx.failed());
		target.seek(0);
		LoadContext ctx { static_cast<u64>(target.totalSize()) };
		const auto copy = RootScopedSlot { target, ScopedSlotStateFlag::eClean, &ctx }.asStruct();
		EXPECT_FALSE(copy.hasRecordSlot("@rev"));
		EXPECT_FALSE(copy.hasRecordSlot("hp"));
		String name {};
		copy.getSlot<String>("name") >> name;
		EXPECT_EQ(name, "abc");
		EXPECT_TRUE(copy.hasRecordSlot("pos"));
		EXPECT_FALSE(ctx.failed());
	}
}

namespace {
	Result<void, MigrateError> writesOnly(ref<const StructScopedSlot>, ref<StructScopedSlot> new_) {
		new_.insertSlot<u32>("hp") << 1u;
		return {};
	}

	/* A root struct that claims more bytes than the archive holds; `tail_` is all that follows its header. */
	[[nodiscard]] std::vector<u8> oversizedStruct(const std::vector<u8>& tail_) {
		std::vector<u8> out { 0xB0 };
		const s64 size = 4096;
		out.insert(out.end(), reinterpret_cast<const u8*>(&size), reinterpret_cast<const u8*>(&size) + sizeof(s64));
		out.insert(out.end(), tail_.begin(), tail_.end());
		return out;
	}
}

namespace MigrationModule {
	TEST(Migration, ScratchOverBudgetRefused) {
		MigrationRegistry reg {};
		const Migration steps[] { { heroType, 0, &writesOnly } };
		reg.add(steps);

		Loaded loaded { writeHero(0) };
		ASSERT_TRUE(loaded.ctx.charge(LoadContext::minBudget - 8uLL));
		EXPECT_FALSE(reg.migrateToCurrent(heroType, loaded.root(), loaded.ctx).has_value());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eBudget);
		EXPECT_EQ(loaded.ctx.firstError()->detail, "migration scratch exceeds the allocation budget");
	}

	// A root that claims more bytes than the archive holds is refused before any entry is copied.
	TEST(Migration, ForwardRejectsRootLargerThanArchive) {
		auto tail = keyBytes("v");
		tail.insert(tail.end(), { 0x0F, 64, 0, 0, 0, 0, 0, 0, 0, 'a', 'b' });
		Loaded loaded { rawArchive(oversizedStruct(tail)) };
		EXPECT_EQ(forwardFrom(loaded), emptyRecordBytes());
		ASSERT_TRUE(loaded.ctx.failed());
		EXPECT_EQ(loaded.ctx.firstError()->kind, ReadError::Kind::eMalformed);
	}
}
