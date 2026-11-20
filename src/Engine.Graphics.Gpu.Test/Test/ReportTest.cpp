#include <ryml/ryml.hpp>
#include <ryml/ryml_std.hpp>

/**/

#include "../pch.h"

/**/

#include "../Harness/Contract.hpp"
#include "../Harness/Report.hpp"

import std;

using namespace hg::test::gpu;
using namespace hg;

namespace {
	Raster make(const u32 w_, const u32 h_, const f32 v_ = 0.F) {
		return Raster { math::uivec2 { w_, h_ }, Vector<math::fvec4>(static_cast<size_t>(w_) * h_, math::fvec4 { v_ }) };
	}

	const InputsAt describe = [](const u32 x_, const u32 y_) { return "x=" + std::to_string(x_) + " y=" + std::to_string(y_); };
}

TEST(Report, ListsFailingInputs) {
	const auto expected = make(4, 4, 0.5F);
	auto actual = expected;
	actual.texels[2 * 4 + 1] = math::fvec4 { 0.75F };

	const auto contract = Contract { { absEpsilon(1e-3F) } };
	const auto report = makeReport("ListsFailingInputs", actual, expected, check(contract, actual, expected), describe, nullptr);

	EXPECT_FALSE(report.passed());
	const auto text = report.text();
	EXPECT_NE(text.find("FAIL"), String::npos);
	EXPECT_NE(text.find("vs expected: max"), String::npos);
	EXPECT_NE(text.find("(1, 2) x=1 y=2"), String::npos);
}

TEST(Report, NoPreviousRun) {
	const auto a = make(4, 4, 0.5F);
	const auto report = makeReport("NoPreviousRun", a, a, check(Contract { { absEpsilon(1e-3F) } }, a, a), describe, nullptr);

	EXPECT_TRUE(report.passed());
	EXPECT_FALSE(report.vsPrevious.has_value());
	EXPECT_NE(report.text().find("no stored run"), String::npos);
}

TEST(Report, DiffAgainstPrevious) {
	const auto a = make(4, 4, 0.5F);
	auto previous = a;
	previous.texels[0] = math::fvec4 { 0.75F };

	const auto report = makeReport("DiffAgainstPrevious", a, a, check(Contract { { absEpsilon(1e-3F) } }, a, a), describe, &previous);

	EXPECT_TRUE(report.passed());
	EXPECT_EQ(report.changedTexels, 1u);
	ASSERT_TRUE(report.vsPrevious.has_value());
	EXPECT_EQ(report.vsPrevious->maxAt, (math::uivec2 { 0, 0 }));
	EXPECT_FLOAT_EQ(report.vsPrevious->maxError, 0.25F);
	ASSERT_EQ(report.inputs.size(), 1u);
	EXPECT_EQ(report.inputs[0].first, (math::uivec2 { 0, 0 }));
}

TEST(Report, StoreRoundTrip) {
	auto a = make(3, 2, 0.25F);
	a.texels[4] = math::fvec4 { 0.1F, 0.2F, 0.3F, 0.4F };

	store::savePassing("StoreRoundTrip", a);
	const auto loaded = store::loadPrevious("StoreRoundTrip");

	ASSERT_TRUE(loaded.has_value());
	EXPECT_EQ(loaded->extent, a.extent);
	ASSERT_EQ(loaded->texels.size(), a.texels.size());
	for (size_t i = 0; i < a.texels.size(); ++i) {
		EXPECT_EQ(loaded->texels[i], a.texels[i]);
	}
}

TEST(Report, PreviousOfOtherExtentIsIgnored) {
	const auto a = make(8, 8, 0.5F);
	const auto previous = make(4, 4, 0.5F);

	const auto report = makeReport("OtherExtent", a, a, check(Contract { { absEpsilon(1e-3F) } }, a, a), describe, &previous);

	EXPECT_FALSE(report.vsPrevious.has_value());
	EXPECT_NE(report.text().find("no stored run"), String::npos);
}

TEST(Report, YamlIsWritten) {
	const auto a = make(2, 2, 0.5F);
	const auto report = makeReport("YamlIsWritten", a, a, check(Contract { { absEpsilon(1e-3F) } }, a, a), describe, nullptr);

	store::writeReport(report);

	const auto path = store::resultDir() / "YamlIsWritten.report.yaml";
	ASSERT_TRUE(std::filesystem::exists(path));

	std::ifstream in { path };
	std::stringstream buffer;
	buffer << in.rdbuf();
	auto content = buffer.str();

	auto tree = ryml::parse_in_arena(ryml::to_csubstr(content));
	EXPECT_TRUE(tree.rootref().has_child("test"));
	EXPECT_TRUE(tree.rootref().has_child("passed"));
	EXPECT_EQ((String { tree["test"].val().str, tree["test"].val().len }), "YamlIsWritten");
	EXPECT_EQ((String { tree["passed"].val().str, tree["passed"].val().len }), report.passed() ? "1" : "0");
}

TEST(Report, CorruptStoreIsIgnored) { {
		std::ofstream out { store::resultDir() / "CorruptStore.last.f32", std::ios::binary | std::ios::trunc };
		const u32 header[2] { 0xFFFFFFFFu, 0xFFFFFFFFu };
		out.write(reinterpret_cast<ptr<const char>>(header), sizeof(header));
		out.write("abcd", 4);
	}

	EXPECT_FALSE(store::loadPrevious("CorruptStore").has_value());
}

TEST(Report, ExtentMismatchSaysNa) {
	const auto a = make(8, 8, 0.5F);
	const auto e = make(4, 4, 0.5F);
	const auto report = makeReport("ExtentMismatch", a, e, check(Contract { { absEpsilon(1e-3F) } }, a, e), describe, nullptr);

	EXPECT_FALSE(report.passed());
	EXPECT_NE(report.text().find("n/a (extent differs)"), String::npos);
}
