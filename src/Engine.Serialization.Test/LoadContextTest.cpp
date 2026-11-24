#include "pch.h"

/**/

#include <variant>

import Engine.Diagnostics;
import Engine.Serialization;

using namespace hg::engine::serialization;
using namespace hg::engine::diag;
using namespace hg;

namespace HgTestLoadContext {
	using enum ReadError::Kind;

	TEST(LoadContext, FirstFailureLatches) {
		LoadContext ctx { 100 };
		EXPECT_FALSE(ctx.failed());
		ctx.fail({ eTruncated, 1, 2, {} });
		ctx.fail({ eMalformed, 3, 4, "later" });
		EXPECT_TRUE(ctx.failed());

		CollectingSink sink {};
		const auto res = ctx.finish(sink, "while loading level 'X'");
		ASSERT_FALSE(res.has_value());
		ASSERT_EQ(res.error().size(), 1uLL);
		EXPECT_EQ(res.error().front().code()->key, "truncated");
		EXPECT_EQ(res.error().front().notes().size(), 2uLL);
	}

	TEST(LoadContext, FinishTwiceReportsOnce) {
		LoadContext ctx { 100 };
		ctx.warn({ eUnknownField, 0, 0, "k" });
		ctx.fail({ eTruncated, 1, 2, {} });

		CollectingSink sink {};
		EXPECT_FALSE(ctx.finish(sink, "first").has_value());
		ASSERT_EQ(sink.items().size(), 1uLL);

		const auto again = ctx.finish(sink, "second");
		EXPECT_TRUE(again.has_value());
		EXPECT_FALSE(ctx.failed());
		EXPECT_EQ(ctx.firstError(), nullptr);
		EXPECT_EQ(sink.items().size(), 1uLL);
	}

	TEST(LoadContext, FinishWithoutFailureEmitsNothing) {
		LoadContext ctx { 100 };
		CollectingSink sink {};
		EXPECT_TRUE(ctx.finish(sink, "clean").has_value());
		EXPECT_TRUE(sink.items().empty());
	}

	TEST(LoadContext, EveryKindMapsToItsCode) {
		struct Case {
			ReadError::Kind kind;
			StringView key;
			bool bytes;
		};
		const Case cases[] {
			{ eMalformed, "malformed-slot", true }, { eTruncated, "truncated", true }, { eDepth, "nesting-too-deep", true },
			{ eBudget, "limit-exceeded", true }, { eTypeMismatch, "type-mismatch", false }, { eInvalidValue, "invalid-value", false },
			{ eDuplicateKey, "duplicate-key", false }, { eMissingField, "missing-field", false },
			{ eDuplicateIdentity, "duplicate-identity", false }, { eUnknownTypeRoot, "unknown-type", false },
			{ eRevisionNewerRoot, "revision-newer", false }, { eUnknownType, "unknown-type", false },
			{ eUnknownField, "unknown-field", false }, { eRevisionNewer, "revision-newer", false },
			{ eUnresolvedReference, "unresolved-reference", false }, { eMigrationFailed, "migration-failed", false }
		};
		for (const auto& c : cases) {
			LoadContext ctx { 100, "a.lvl" };
			ctx.fail({ c.kind, 5, 1, {} });
			CollectingSink sink {};
			const auto res = ctx.finish(sink, "ctx");
			ASSERT_FALSE(res.has_value());
			EXPECT_EQ(res.error().front().code()->key, c.key);
			EXPECT_EQ(std::holds_alternative<ByteOffset>(res.error().front().primary()), c.bytes) << c.key;
		}
	}

	TEST(LoadContext, PathJoins) {
		LoadContext ctx { 100 };
		ctx.pushPath("Level");
		ctx.pushPath("@components[3] (CameraActor)");
		ctx.pushPath("fov");
		EXPECT_EQ(ctx.recordPath(), "Level / @components[3] (CameraActor) / fov");
		ctx.popPath();
		EXPECT_EQ(ctx.recordPath(), "Level / @components[3] (CameraActor)");
	}

	TEST(LoadContext, DepthLimit) {
		LoadContext ctx { 100 };
		for (int i = 0; i < 64; ++i) {
			ASSERT_TRUE(ctx.enterDepth());
		}
		EXPECT_FALSE(ctx.enterDepth());
		ctx.leaveDepth();
		EXPECT_TRUE(ctx.enterDepth());
	}

	TEST(LoadContext, BudgetFloor) {
		LoadContext ctx { 100 };
		EXPECT_TRUE(ctx.charge(64uLL * 1024uLL * 1024uLL));
		EXPECT_FALSE(ctx.charge(1));

		LoadContext big { 100uLL * 1024uLL * 1024uLL };
		EXPECT_TRUE(big.charge(400uLL * 1024uLL * 1024uLL));
		EXPECT_FALSE(big.charge(1));
	}

	TEST(LoadContext, UnreadKeysAcrossAliases) {
		LoadContext ctx { 100 };
		ctx.markRead(&ctx, 40, "a");
		ctx.markRead(&ctx, 40, String { "b" });
		const Vector<String> present { "a", "b", "c" };
		const auto unread = ctx.unreadKeys(&ctx, 40, present);
		ASSERT_EQ(unread.size(), 1uLL);
		EXPECT_EQ(unread.front(), "c");
		EXPECT_EQ(ctx.unreadKeys(&ctx, 8, present).size(), 3uLL);
		EXPECT_EQ(ctx.unreadKeys(&present, 40, present).size(), 3uLL);
	}

	TEST(LoadContext, ForgetArchiveDropsOnlyItsReadSets) {
		LoadContext ctx { 100 };
		int other {};
		ctx.markRead(&ctx, 40, "a");
		ctx.markRead(&ctx, 8, "a");
		ctx.markRead(&other, 40, "a");
		ctx.forgetArchive(&ctx);
		const Vector<String> present { "a" };
		EXPECT_EQ(ctx.unreadKeys(&ctx, 40, present).size(), 1uLL);
		EXPECT_EQ(ctx.unreadKeys(&ctx, 8, present).size(), 1uLL);
		EXPECT_TRUE(ctx.unreadKeys(&other, 40, present).empty());
	}

	TEST(LoadContext, WarningsReachSinkAndFinishMapsMalformed) {
		LoadContext ctx { 100 };
		ctx.warn({ eUnknownField, 0, 0, "k" });
		EXPECT_EQ(ctx.warningCount(), 1uLL);
		ctx.fail({ eMalformed, 12, 4, {} });

		CollectingSink sink {};
		const auto res = ctx.finish(sink, "while loading level 'X'");
		ASSERT_FALSE(res.has_value());
		EXPECT_EQ(sink.items().size(), 1uLL);
		EXPECT_EQ(sink.items().front().severity(), Severity::eWarning);

		const auto& d = res.error().front();
		EXPECT_EQ(d.code()->key, "malformed-slot");
		EXPECT_EQ(d.severity(), Severity::eError);
		ASSERT_TRUE(std::holds_alternative<ByteOffset>(d.primary()));
		EXPECT_EQ(std::get<ByteOffset>(d.primary()).offset, 12uLL);
		EXPECT_EQ(d.notes().back(), "while loading level 'X'");
	}
}
