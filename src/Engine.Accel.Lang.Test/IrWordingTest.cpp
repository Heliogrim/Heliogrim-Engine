#include "pch.h"

/**/

#include "IrWording.hpp"

#include <gtest/gtest-spi.h>

using namespace hg::engine::accel::lang::ir;
using namespace hg;

namespace AccelLang {
	namespace {
		void checkBad() {
			const Diagnostic bad = Diagnostic::error(hg::engine::accel::lang::codes::asl("reserved-name"), "Foo is reserved");
			expectWording(bad);
		}
	}

	TEST(IrWording, LeadingQuotedNameMayBeUpperCase) {
		const Diagnostic ok = Diagnostic::error(hg::engine::accel::lang::codes::asl("reserved-name"), "'Foo' is reserved in GLSL");
		expectWording(ok);
		EXPECT_NONFATAL_FAILURE(checkBad(), "starts upper-case");
	}
}
