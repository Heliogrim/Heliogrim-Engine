#include "pch.h"

/**/

#include <Engine.Common/__macro.hpp>

import Engine.Common;

/* Config headers stay textual and may follow the import. */
#include <Engine.Common/GLM.hpp>

/* Macro-only std headers stay textual and may follow the import. */
#include <cassert>
#include <climits>

#if defined(GLM_SETUP_INCLUDED)
#error "GLM.hpp must import glm outside of a module interface, not include it."
#endif

#if not defined(assert) || not defined(INT_MAX)
#error "Macro-only std headers must still define their macros after an import."
#endif

namespace Common {
	TEST(ModuleImport, InternalLinkageNames) {
		EXPECT_EQ(hg::s32_max, 2147483647);
		EXPECT_DOUBLE_EQ(hg::math::pi, 3.1415926535897932384626433832795028841971);
		hg::discard = hg::Vector<int> { 1 };

		EXPECT_FLOAT_EQ(hg::math::vec3_up.y, 1.F);
		EXPECT_FLOAT_EQ(hg::math::vec3_forward.z, 1.F);
		EXPECT_FLOAT_EQ(hg::math::vec3_left.x, -1.F);
		EXPECT_FLOAT_EQ(hg::math::vec2_down.y, -1.F);
		EXPECT_FLOAT_EQ(hg::math::vec2_right.x, 1.F);

		const auto view = hg::math::lookAt(
			hg::math::vec3 { 0.F, 0.F, -2.F },
			hg::math::vec3_zero,
			hg::math::vec3_up
		);
		EXPECT_FLOAT_EQ(view[0][0], -1.F);
		EXPECT_FLOAT_EQ(view[1][1], 1.F);
		EXPECT_FLOAT_EQ(view[2][2], -1.F);
		EXPECT_FLOAT_EQ(view[3][2], -2.F);

		const auto ortho = hg::math::ortho(0.F, 2.F, 0.F, 2.F, 0.F, 1.F);
		EXPECT_FLOAT_EQ(ortho[0][0], 1.F);
		EXPECT_FLOAT_EQ(ortho[3][0], -1.F);
		EXPECT_FLOAT_EQ(ortho[2][2], -1.F);

		const auto ortho2d = hg::math::ortho(-1.F, 1.F, -1.F, 1.F);
		EXPECT_FLOAT_EQ(ortho2d[1][1], 1.F);
		EXPECT_FLOAT_EQ(ortho2d[3][1], 0.F);

		EXPECT_TRUE((std::is_same_v<decltype(hg::traits::nothrow), const hg::nothrow_t>));
		EXPECT_NE(&hg::traits::nothrow, nullptr);
	}

	TEST(ModuleImport, ImportThenConfigHeader) {
		EXPECT_FLOAT_EQ(glm::length(glm::vec3 { 3.F, 4.F, 0.F }), 5.F);
	}

	TEST(ModuleImport, TemplatesAndGuides) {
		auto o = hg::Opt { hg::s32 { 3 } };
		o = hg::None;
		EXPECT_FALSE(o.has_value());
	}

	TEST(ModuleImport, StdThroughCommon) {
		std::vector<int> v { 1, 2 };
		EXPECT_EQ(std::ranges::find(v, 2) - v.begin(), 1);
		EXPECT_EQ(std::format("{}", 7), "7");
	}

	TEST(ModuleImport, VendorOperatorsThroughCommon) {
		hg::Opt<int> none = hg::None;
		hg::Opt<int> some = 3;
		EXPECT_FALSE(none.has_value());
		EXPECT_EQ(some.value(), 3);
		EXPECT_TRUE(none == hg::None);
		EXPECT_FALSE(some == hg::None);

		ankerl::unordered_dense::map<int, int> m;
		m[1] = 2;
		EXPECT_EQ(m.size(), 1);
	}

	TEST(ModuleImport, AssertsThroughCommon) {
		hg::assertrt(true);
		EXPECT_DEATH(hg::assertrt(false), "");
	}

	TEST(ModuleImport, MacroHeaderAfterImport) {
		EXPECT_EQ(INT_MAX, std::numeric_limits<int>::max());
	}
}
