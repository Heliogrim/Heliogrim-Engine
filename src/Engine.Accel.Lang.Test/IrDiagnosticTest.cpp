#include "pch.h"

/**/

#include "IrSpan.hpp"

#include <Engine.Accel.Lang/Ir/Merge.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <algorithm>

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace diag = hg::engine::diag;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] std::expected<Module, Diagnostics> parse(StringView source_) {
			return parseModule("M", source_, EntryKind::eFragment, ModuleResolver { noIncludes });
		}

		[[nodiscard]] StringView textOf(StringView source_, ref<const SourceSpan> span_) {
			return span_.end <= source_.size() && span_.begin <= span_.end ? source_.substr(span_.begin, span_.end - span_.begin) : StringView {};
		}

	}

	TEST(IrDiagnostic, NestedIncludeErrorsNameTheirOwnFile) {
		constexpr StringView bad = "\tconst a = 0.0;\n\tlet l = &mut a;\n\tl = 1.0;\n";
		const auto run = [&](bool innerBad_) {
			const String inner = String("fn H() {\n") + String(innerBad_ ? bad : "") + "}\n";
			const String middle = String("#include <Inner>\nfn G() {\n\tH();\n") + String(innerBad_ ? "" : bad) + "}\n";
			const auto resolver = [&](StringView name_) -> Opt<String> {
				return name_ == "Inner" ? Opt<String> { inner } : name_ == "Middle" ? Opt<String> { middle } : None;
			};
			const auto res = parseModule("M", "#include <Middle>\nfn main() {\n\tG();\n}\n", EntryKind::eFragment, ModuleResolver { resolver });
			EXPECT_FALSE(res.has_value());
			if (res.has_value()) {
				return;
			}
			const auto& d = res.error().front();
			EXPECT_NE(d.message().find("cannot borrow immutable 'a' as mutable"), String::npos) << d.message();
			const auto primary = spanOf(d);
			EXPECT_EQ(primary.file, innerBad_ ? "Inner" : "Middle");
			EXPECT_EQ(primary.begin, (innerBad_ ? inner : middle).find("&mut a") + 5uz);
		};
		run(true);
		run(false);
	}

	TEST(IrDiagnostic, MergedNodesKeepTheirFile) {
		constexpr StringView include = "fn G() {}\n";
		constexpr StringView profile = "fn F() {}\n";
		const auto resolver = [include](StringView name_) -> Opt<String> {
			return name_ == "Inc" ? Opt<String> { String(include) } : None;
		};
		const auto p = parseModule("P", profile, EntryKind::eFragment, ModuleResolver { noIncludes });
		const auto s = parseModule("S", "#include <Inc>\nfn main() {\n\tG();\n}\n", EntryKind::eFragment, ModuleResolver { resolver });
		ASSERT_TRUE(p.has_value());
		ASSERT_TRUE(s.has_value());
		const Vector<const Module*> list { &*p, &*s };
		const auto merged = merge(list);
		ASSERT_TRUE(merged.has_value());
		EXPECT_EQ(merged->sources, (Vector<String> { "P", "S", "Inc" }));

		const auto fn = [&](StringView name_) { return spanIn(*merged, std::ranges::find(merged->functions, name_, &FunctionDecl::name)->loc); };
		EXPECT_EQ(fn("F").file, "P");
		EXPECT_EQ(textOf(profile, fn("F")), "F");
		EXPECT_EQ(fn("G").file, "Inc");
		EXPECT_EQ(textOf(include, fn("G")), "G");
		EXPECT_EQ(fn("main").file, "S");
	}
}
