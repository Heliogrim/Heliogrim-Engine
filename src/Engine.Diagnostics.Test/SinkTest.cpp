#include "pch.h"

/**/

import Engine.Diagnostics;

using namespace hg::engine::diag;
using namespace hg;

namespace HgDiag {
	namespace {
		inline constexpr DiagnosticCodeInfo testTable[] = {
			{ { { "TST" }, 1 }, "first-kind", "first" }
		};
	}

	TEST(Sink, CollectingKeepsOrderAndFlagsErrors) {
		CollectingSink sink {};
		EXPECT_FALSE(sink.hasErrors());

		sink.emit(Diagnostic::warning(findCode(testTable, "first-kind"), "w"));
		EXPECT_FALSE(sink.hasErrors());
		sink.emit(Diagnostic::error(findCode(testTable, "first-kind"), "e"));
		EXPECT_TRUE(sink.hasErrors());
		EXPECT_EQ(sink.items().size(), 2uz);

		const auto taken = sink.take();
		ASSERT_EQ(taken.size(), 2uz);
		EXPECT_EQ(taken[0].message(), "w");
		EXPECT_EQ(taken[1].message(), "e");

		EXPECT_TRUE(sink.items().empty());
		EXPECT_FALSE(sink.hasErrors());
	}
}
