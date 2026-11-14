#include "pch.h"

/**/

#include <Engine.Diagnostics/Code.hpp>

using namespace hg::engine::diag;
using namespace hg;

namespace HgDiag {
	TEST(Code, FormatsFourDigits) {
		EXPECT_EQ(formatCode({ { 'A', 'S', 'L', 0 }, 42 }), "ASL0042");
		EXPECT_EQ(formatCode({ { 'S', 'T', 'O', 'R' }, 7 }), "STOR0007");
	}

	TEST(Code, SlugFromCode) {
		const DiagnosticCodeInfo info { { { 'A', 'S', 'L', 0 }, 42 }, "k", "s" };
		EXPECT_EQ(slugOf(info), "asl/0042");
	}
}
