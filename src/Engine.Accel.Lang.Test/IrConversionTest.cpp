#include "pch.h"

/**/

#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Typecheck.hpp>

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] std::expected<Module, Diagnostics> parse(StringView body_) {
			return parseModule("M", "fn main() {\n" + String { body_ } + "\n}\n", EntryKind::eFragment, ModuleResolver { noIncludes });
		}

		/* the code keys of the errors, space separated; "" when the module checks */
		[[nodiscard]] String codesOf(StringView body_) {
			const auto res = parse(body_);
			String keys {};
			for (const auto& d : res.has_value() ? Diagnostics {} : res.error()) {
				keys += (d.code() != nullptr ? String { d.code()->key } : String { "?" }) + " ";
			}
			return keys;
		}

		[[nodiscard]] String typeOfLocal(StringView body_, StringView name_) {
			const auto res = parse(body_);
			if (not res.has_value()) {
				ADD_FAILURE() << res.error().front().message();
				return {};
			}
			for (const auto& stmt : res->stmts) {
				if (stmt.kind == StmtKind::eDeclareLocal && stmt.name == name_) {
					return describeType(res->types, stmt.type);
				}
			}
			return {};
		}
	}

	TEST(IrConversionTest, MixedFloatIntIsError) {
		const auto res = parse("let a: f32 = 1.0;\nlet i: s32 = 1;\nlet x: f32 = a * i;");
		ASSERT_FALSE(res.has_value());
		ASSERT_EQ(res.error().size(), 1uz);
		const auto& d = res.error().front();
		ASSERT_NE(d.code(), nullptr);
		EXPECT_EQ(d.code()->key, "implicit-conversion");
		ASSERT_FALSE(d.fixes().empty());
		EXPECT_NE(d.fixes().front().title.find("f32(i)"), String::npos) << d.fixes().front().title;
	}

	TEST(IrConversionTest, SignedUnsignedIsError) {
		EXPECT_EQ(codesOf("let u: u32 = 1u;\nlet s: s32 = 1;\nlet x = u + s;"), "implicit-conversion ");
	}

	TEST(IrConversionTest, WideningIsError) {
		EXPECT_EQ(codesOf("let h: u16 = 1;\nlet y: u32 = h;"), "implicit-conversion ");
	}

	TEST(IrConversionTest, ConstructorConverts) {
		EXPECT_EQ(codesOf("let i: s32 = 1;\nlet x = f32(i);"), "");
		EXPECT_EQ(codesOf("let uv: s32vec2 = s32vec2(1, 2);\nlet x = u32vec2(uv);"), "");
		EXPECT_EQ(codesOf("let b = true;\nlet x = u32(b);"), "");
		EXPECT_EQ(typeOfLocal("let i: s32 = 1;\nlet x = f32(i);", "x"), "f32");
	}

	TEST(IrConversionTest, BoolFromNumberIsError) {
		EXPECT_EQ(codesOf("let x: u32 = 1u;\nlet b = bool(x);"), "type-mismatch ");
	}

	TEST(IrConversionTest, LiteralFromContext) {
		EXPECT_EQ(codesOf("let a: f32 = 1;"), "");
		EXPECT_EQ(codesOf("let b: u8 = 300;"), "literal-out-of-range ");
		EXPECT_EQ(codesOf("let c: s32 = 1.5;"), "literal-out-of-range ");
	}

	TEST(IrConversionTest, LiteralRangeAtFinalType) {
		EXPECT_EQ(codesOf("let a: u32 = -1;"), "literal-out-of-range ");
		EXPECT_EQ(codesOf("let b: s8 = -128;"), "");
		EXPECT_EQ(codesOf("let c: s8 = 128;"), "literal-out-of-range ");
		EXPECT_EQ(codesOf("let d: s32 = 3000000000;"), "literal-out-of-range ");
		EXPECT_EQ(codesOf("let e = 3000000000;"), "literal-out-of-range ");
		EXPECT_EQ(codesOf("let f: u32 = 5000000000u;"), "literal-out-of-range ");
		EXPECT_EQ(codesOf("let g: f32 = 100000.0;"), "");
		EXPECT_EQ(codesOf("let h: f16 = 100000.0;"), "literal-out-of-range ");
		EXPECT_EQ(codesOf("let s: s32 = -2147483648;"), "");
	}

	TEST(IrConversionTest, LiteralRangeAtReturnAndArgument) {
		const auto ret = parseModule("M", "fn f() -> s32 { return 3000000000; }\nfn main() { let x = f(); }\n", EntryKind::eFragment, ModuleResolver { noIncludes });
		ASSERT_FALSE(ret.has_value());
		EXPECT_EQ(ret.error().front().code()->key, "literal-out-of-range");

		const auto arg = parseModule("M", "fn f(a: s32) -> s32 { return a; }\nfn main() { let x = f(3000000000); }\n", EntryKind::eFragment, ModuleResolver { noIncludes });
		ASSERT_FALSE(arg.has_value());
		EXPECT_EQ(arg.error().front().code()->key, "literal-out-of-range");
	}

	TEST(IrConversionTest, CompoundAssignConvertsRhs) {
		const auto res = parse("let f: f32 = 1.0;\nlet i: s32 = 1;\ni += f;");
		ASSERT_FALSE(res.has_value());
		ASSERT_EQ(res.error().size(), 1uz);
		const auto& d = res.error().front();
		EXPECT_EQ(d.code()->key, "implicit-conversion");
		ASSERT_FALSE(d.fixes().empty());
		EXPECT_NE(d.fixes().front().title.find("s32(f)"), String::npos) << d.fixes().front().title;
	}

	TEST(IrConversionTest, IntegerWidthsWidenNarrower) {
		const auto res = parse("let h: u16 = 1;\nlet w: u32 = 1;\nlet x = w + h;");
		ASSERT_FALSE(res.has_value());
		const auto& d = res.error().front();
		ASSERT_FALSE(d.fixes().empty());
		EXPECT_NE(d.fixes().front().title.find("u32(h)"), String::npos) << d.fixes().front().title;
	}

	TEST(IrConversionTest, ConstructorArgumentsConvertExplicitly) {
		EXPECT_EQ(codesOf("let f: f32 = 1.0;\nlet v = s32vec2(f, 1);"), "implicit-conversion ");
		EXPECT_EQ(codesOf("let f: f32 = 1.0;\nlet v = s32vec2(s32(f), 1);"), "");
	}

	TEST(IrConversionTest, BitcastIsReserved) {
		EXPECT_EQ(codesOf("let bitcast = 1;"), "reserved-name ");
	}

	TEST(IrConversionTest, BitcastSameSize) {
		EXPECT_EQ(codesOf("let f: f32 = 1.0;\nlet x = bitcast<u32>(f);"), "");
		EXPECT_EQ(typeOfLocal("let f: f32 = 1.0;\nlet x = bitcast<u32>(f);", "x"), "u32");
		EXPECT_EQ(codesOf("let f: f32 = 1.0;\nlet x = bitcast<u64>(f);"), "bitcast-size ");
	}

	TEST(IrConversionTest, VectorTimesScalarSameKind) {
		EXPECT_EQ(codesOf("let v = f32vec3(1.0);\nlet x = v * 2.0;"), "");
	}
}
