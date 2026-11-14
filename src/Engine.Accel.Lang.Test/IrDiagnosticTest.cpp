#include "pch.h"

/**/

#include "IrSpan.hpp"

#include <Engine.Accel.Lang/Ir/Merge.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Diagnostics/Render.hpp>
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

		/* applies every edit of `fix_`, back to front by `begin`, so earlier offsets stay valid */
		[[nodiscard]] String applyFix(StringView source_, ref<const diag::Fix> fix_) {
			auto edits = fix_.edits;
			std::ranges::sort(edits, std::ranges::greater {}, [](const auto& e_) { return e_.range.begin; });
			String out { source_ };
			for (const auto& edit : edits) {
				out.replace(edit.range.begin, edit.range.end - edit.range.begin, edit.replacement);
			}
			return out;
		}

		class ModuleSource final : public diag::SourceLookup {
		public:
			explicit ModuleSource(StringView text_) :
				_text(text_) {}

			[[nodiscard]] Optional<StringView> find(StringView file_) const override {
				if (file_ == "M") {
					return _text;
				}
				return {};
			}

		private:
			StringView _text;
		};
	}

	TEST(IrDiagnostic, OverlapNamesBothSpans) {
		constexpr StringView source = "fn F(a: &mut f32, b: &mut f32) {}\nfn main() {\n\tlet x = 1.0;\n\tF(x, x);\n}\n";
		const auto res = parse(source);
		ASSERT_FALSE(res.has_value());
		const auto& d = res.error().front();
		EXPECT_EQ(d.message(), "'&mut' argument 'x' overlaps argument 'x' of call to 'F'");

		const auto primary = spanOf(d);
		EXPECT_EQ(primary.begin, source.find("F(x, x)") + 2uz);
		EXPECT_EQ(textOf(source, primary), "x");

		ASSERT_EQ(d.labels().size(), 1uz);
		ASSERT_TRUE(d.labels()[0].at.is<SourceSpan>());
		const auto& second = d.labels()[0].at.as<SourceSpan>();
		EXPECT_EQ(second.begin, source.find("F(x, x)") + 5uz);
		EXPECT_EQ(textOf(source, second), "x");
		EXPECT_EQ(d.labels()[0].message, "overlaps the '&mut' argument");

		const ModuleSource lookup { source };
		const auto rendered = diag::render(d, &lookup);
		EXPECT_EQ(std::ranges::count(rendered, '^'), 2) << rendered;
	}

	TEST(IrDiagnostic, BorrowImmutableFixRecompiles) {
		constexpr StringView source =
			"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
			"fn main() {\n\tconst a = 0.0;\n\tlet l = &mut a;\n\tl = 1.0;\n\tdepth = a;\n}\n";
		const auto res = parse(source);
		ASSERT_FALSE(res.has_value());
		const auto& d = res.error().front();
		EXPECT_EQ(d.message(), "cannot borrow immutable 'a' as mutable");

		ASSERT_EQ(d.fixes().size(), 1uz);
		const auto& fix = d.fixes().front();
		EXPECT_EQ(fix.title, "declare 'a' with 'let'");
		EXPECT_EQ(fix.applicability, diag::Applicability::eMachineApplicable);

		ASSERT_EQ(fix.edits.size(), 1uz);
		EXPECT_EQ(textOf(source, fix.edits.front().range), "const");

		const auto fixed = applyFix(source, fix);
		EXPECT_NE(fixed.find("let a = 0.0;"), String::npos) << fixed;
		const auto again = parse(fixed);
		EXPECT_TRUE(again.has_value()) << fixed << (again.has_value() || again.error().empty() ? "" : again.error().front().message());
	}

	TEST(IrDiagnostic, BorrowImmutableInIncludeFixesTheInclude) {
		constexpr StringView include = "fn G() {\n\tconst a = 0.0;\n\tlet l = &mut a;\n\tl = 1.0;\n}\n";
		const auto resolver = [include](StringView name_) -> Opt<String> {
			return name_ == "Inc" ? Opt<String> { String(include) } : None;
		};
		const auto res = parseModule("M", "#include <Inc>\nfn main() {\n\tG();\n}\n", EntryKind::eFragment, ModuleResolver { resolver });
		ASSERT_FALSE(res.has_value());
		const auto& d = res.error().front();
		EXPECT_NE(d.message().find("cannot borrow immutable 'a' as mutable"), String::npos) << d.message();
		const auto primary = spanOf(d);
		EXPECT_EQ(primary.file, "Inc");
		EXPECT_EQ(primary.begin, include.find("&mut a") + 5uz);

		ASSERT_EQ(d.fixes().size(), 1uz);
		ASSERT_EQ(d.fixes().front().edits.size(), 1uz);
		const auto& edit = d.fixes().front().edits.front();
		EXPECT_EQ(edit.range.file, "Inc");
		EXPECT_EQ(textOf(include, edit.range), "const");
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
