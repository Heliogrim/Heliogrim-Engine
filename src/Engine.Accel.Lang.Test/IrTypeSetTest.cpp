#include "pch.h"

/**/

#include <array>
#include <map>
#include <set>
#include <string>

#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Lower.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/References.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Slice.hpp>
#include <Engine.Accel.Lang/Ir/Tier.hpp>
#include <Engine.Accel.Lang/Ir/Spirv/SpirvEmitter.hpp>
#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <spirv-tools/libspirv.hpp>

#include "IrSpan.hpp"

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] Module resolved(StringView source_, EntryKind entry_, std::span<const String> required_ = {}) {
			auto parsed = parseModule("M", source_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(parsed.has_value()) << (parsed.has_value() || parsed.error().empty() ? "" : parsed.error().front().message());
			if (not parsed.has_value()) {
				return {};
			}
			auto res = resolveToggles(*parsed, ToggleSet {});
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? slice(*res, required_) : Module {};
		}

		[[nodiscard]] Layout layoutOf(ref<const Module> mod_) {
			return assignLayout(computeInterface(mod_), mod_.types, {});
		}

		struct Emission {
			Diagnostics diagnostics;
			Vector<u32> words;
			/* disassembly; empty when there are diagnostics */
			String text;

			[[nodiscard]] bool has(StringView capability_) const {
				return text.contains("OpCapability " + String { capability_ } + "\n");
			}
		};

		/* lowers references, lays out and emits at the Dev tier (which validates) */
		[[nodiscard]] Emission emitModule(ref<const Module> mod_) {
			auto lowered = lowerReferences(mod_);
			if (not lowered.has_value()) {
				return Emission { std::move(lowered).error(), {}, {} };
			}
			auto layout = layoutOf(*lowered);
			layout.specConstants = assignSpecConstants(lowered->constants);
			auto res = spirv::emit(*lowered, layout, Tier::eDev);
			if (not res.diagnostics.empty()) {
				return Emission { std::move(res.diagnostics), std::move(res.words), {} };
			}
			spvtools::SpirvTools tools { SPV_ENV_VULKAN_1_3 };
			std::string text {};
			EXPECT_TRUE(tools.Disassemble(res.words, &text, SPV_BINARY_TO_TEXT_OPTION_NO_HEADER));
			return Emission { {}, std::move(res.words), String { text } };
		}

		[[nodiscard]] Emission emit(StringView source_, EntryKind entry_ = EntryKind::eFragment) {
			return emitModule(resolved(source_, entry_));
		}

		/* a uniform `x` of `type_`, read by `main` so the emitter keeps it */
		[[nodiscard]] String uniformOf(StringView type_) {
			return "@input (mode = bind, id = \"x\", class = uniform) " + String { type_ } + " x;\nfn main() {\n\tlet y = x;\n}\n";
		}

		/**
		 * The numeric types a module declares, in source spelling: `u8` (OpTypeInt 8 0), `s16`, `f32` (OpTypeFloat 32),
		 * `u8vec4` (OpTypeVector of u8, 4) and `f32mat3x4` (OpTypeMatrix of 3 f32vec4 columns).
		 */
		[[nodiscard]] std::set<String> numericTypes(ref<const Vector<u32>> words_) {
			std::map<u32, String> names {};
			std::set<String> out {};
			for (auto i = 5uz; i < words_.size();) {
				const auto count = words_[i] >> 16u;
				const auto op = words_[i] & 0xFFFFu;
				if (count == 0u || i + count > words_.size()) {
					break;
				}
				const auto* operands = words_.data() + i + 1uz;
				String name {};
				if (op == 21u) {
					name = (operands[2] != 0u ? "s" : "u") + std::to_string(operands[1]);
				} else if (op == 22u) {
					name = "f" + std::to_string(operands[1]);
				} else if (op == 23u && names.contains(operands[1])) {
					name = names[operands[1]] + "vec" + std::to_string(operands[2]);
				} else if (op == 24u && names.contains(operands[1])) {
					// column `<s>vec<r>` -> `<s>mat<c>x<r>`
					const auto& column = names[operands[1]];
					const auto at = column.find("vec");
					name = column.substr(0uz, at) + "mat" + std::to_string(operands[2]) + "x" + column.substr(at + 3uz);
				}
				if (not name.empty()) {
					names[operands[0]] = name;
					out.insert(name);
				}
				i += count;
			}
			return out;
		}

		/* the source spelling with the square matrix shorthand written out (`f32mat4` -> `f32mat4x4`) */
		[[nodiscard]] String canonical(StringView source_) {
			const auto at = source_.find("mat");
			if (at == StringView::npos || source_.find('x') != StringView::npos) {
				return String { source_ };
			}
			return String { source_ } + "x" + String { source_.substr(at + 3uz) };
		}

		struct TypeCase {
			String source;
			String glsl;
		};

		/* source spelling -> GLSL spelling for every required numeric type, written out by hand */
		[[nodiscard]] Vector<TypeCase> typeCases() {
			struct Family {
				const char* source;
				const char* scalar;
				const char* vector;
			};
			constexpr std::array<Family, 11> families {
				{
					{ "f16", "float16_t", "f16vec" }, { "f32", "float", "vec" }, { "f64", "double", "dvec" },
					{ "u8", "uint8_t", "u8vec" }, { "u16", "uint16_t", "u16vec" }, { "u32", "uint", "uvec" },
					{ "u64", "uint64_t", "u64vec" },
					{ "s8", "int8_t", "i8vec" }, { "s16", "int16_t", "i16vec" }, { "s32", "int", "ivec" }, { "s64", "int64_t", "i64vec" }
				}
			};
			constexpr std::array<const char*, 3> matrices { "f16mat", "mat", "dmat" };

			Vector<TypeCase> cases {};
			for (const auto& family : families) {
				cases.emplace_back(TypeCase { family.source, family.scalar });
				for (auto n = 2; n <= 4; ++n) {
					cases.emplace_back(
						TypeCase { String { family.source } + "vec" + std::to_string(n), String { family.vector } + std::to_string(n) }
					);
				}
			}
			for (auto f = 0uz; f < 3uz; ++f) {
				for (auto c = 2; c <= 4; ++c) {
					for (auto r = 2; r <= 4; ++r) {
						const auto source = String { families[f].source } + "mat" + std::to_string(c) + "x" + std::to_string(r);
						const auto shape = c == r ? std::to_string(c) : std::to_string(c) + "x" + std::to_string(r);
						cases.emplace_back(TypeCase { source, String { matrices[f] } + shape });
					}
				}
			}
			return cases;
		}

		class TypeSetTest :
			public testing::TestWithParam<TypeCase> {};
	}

	TEST_P(TypeSetTest, EmitsExactSpirvType) {
		const auto& [source, glsl] = GetParam();

		const auto emission = emit(uniformOf(source));

		ASSERT_TRUE(emission.diagnostics.empty()) << source << ": " << emission.diagnostics.front().message();
		const auto types = numericTypes(emission.words);
		EXPECT_TRUE(types.contains(canonical(source))) << source << "\n" << emission.text;
	}

	INSTANTIATE_TEST_SUITE_P(
		Required,
		TypeSetTest,
		testing::ValuesIn(typeCases()),
		[](const testing::TestParamInfo<TypeCase>& info_) { return info_.param.source; }
	);

	TEST(TypeSetTest, RequiredSetHasSeventyOneTypes) {
		EXPECT_EQ(typeCases().size(), 71uz);
	}

	TEST(TypeSetTest, EveryShapeLowersToItsOwnLegacyType) {
		std::set<u8> seen {};
		for (const auto& [source, glsl] : typeCases()) {
			const auto mod = resolved(uniformOf(source), EntryKind::eFragment);
			auto res = lowerToIntermediate(mod, layoutOf(mod), Tier::eDev);
			ASSERT_TRUE(res.has_value()) << source;

			const StringView id { "x" };
			const auto symbol = std::ranges::find_if(
				res->rep.symbolTable,
				[&](const auto& s_) { return s_->symbolId == SymbolId::from(id); }
			);
			ASSERT_NE(symbol, res->rep.symbolTable.end()) << source;
			const auto& type = (*symbol)->var.data->type;

			EXPECT_EQ(type.category, TypeCategory::eScalar) << source;
			seen.insert(static_cast<u8>(type.scalarType));
		}
		EXPECT_EQ(seen.size(), 71uz);
	}

	TEST(TypeSetTest, LegacyAccessorsRoundTrip) {
		EXPECT_EQ(vectorOf(ScalarType::eU8, 2), ScalarType::eU8Vec2);
		EXPECT_EQ(vectorOf(ScalarType::eF32, 4), ScalarType::eF32Vec4);
		EXPECT_EQ(vectorOf(ScalarType::eS16, 3), ScalarType::eS16Vec3);
		EXPECT_EQ(matrixOf(ScalarType::eF16, 3, 4), ScalarType::eF16Mat3x4);
		EXPECT_EQ(matrixOf(ScalarType::eF64, 2, 2), ScalarType::eF64Mat2x2);
		EXPECT_EQ(matrixOf(ScalarType::eS32, 2, 2), ScalarType::eS32) << "integers have no matrices";
		EXPECT_EQ(elementOf(ScalarType::eS16Vec3), ScalarType::eS16);
		EXPECT_EQ(elementOf(ScalarType::eF64Mat4x3), ScalarType::eF64);
		EXPECT_EQ(componentCount(ScalarType::eU64Vec4), 4);
		EXPECT_EQ(componentCount(ScalarType::eF32Mat4x2), 4);
		EXPECT_EQ(rowCount(ScalarType::eF32Mat4x2), 2);
		EXPECT_TRUE(isVector(ScalarType::eU8Vec2));
		EXPECT_FALSE(isVector(ScalarType::eU8));
		EXPECT_TRUE(isMatrix(ScalarType::eF16Mat2x2));
	}

	TEST(TypeSetTest, IntegerMatrixInSourceIsLocatedDiagnostic) {
		const auto parsed = parseModule(
			"M",
			"\n@input (mode = bind, id = \"x\", class = uniform) s32mat4 x;\n",
			EntryKind::eFragment,
			ModuleResolver { noIncludes }
		);

		ASSERT_FALSE(parsed.has_value());
		const auto& diag = parsed.error().front();
		EXPECT_EQ(diag.message(), "GLSL has no integer matrix type 's32mat4'");
		ASSERT_EQ(diag.notes().size(), 1uz);
		EXPECT_EQ(diag.notes().front(), "matrices are float only");
		EXPECT_EQ(spanOf(diag).line, 2u);
	}

	TEST(TypeSetTest, RecursiveStructUsedOnlyAsLocalIsDiagnostic) {
		auto mod = resolved("struct S { f32 a; }\nfn main() {\n\tlet s: S;\n}\n", EntryKind::eCompute);
		const auto layout = layoutOf(mod);
		for (auto i = 0uz; i < mod.types.size(); ++i) {
			if (mod.types[i].kind == TypeKind::eStruct) {
				mod.types[i].fields.emplace_back(FieldDesc { "self", TypeRef { static_cast<u32>(i) } });
			}
		}

		const auto result = spirv::emit(mod, layout, Tier::eDev);

		ASSERT_FALSE(result.diagnostics.empty());
		EXPECT_EQ(result.diagnostics.front().code()->key, "recursive-struct");
		EXPECT_EQ(spanOf(result.diagnostics.front()).line, 3u);
		EXPECT_TRUE(result.words.empty());
	}

	TEST(TypeSetTest, HalfFloatConstantRoundsOnceFromDouble) {
		// 1 + 2^-11 + 2^-24: through f32 it ties to 1.0, straight from f64 it rounds up to 1 + 2^-10
		const auto emission = emit(
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tlet h: f16 = 1.000488340854644775390625;\n\tcolor = f32vec4(f32(h));\n}\n"
		);

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.text.contains("0x1.004p+0")) << emission.text;
	}

	TEST(TypeSetTest, IntegerMatrixInModuleIsDiagnosticNotWrongWords) {
		Module mod {};
		TypeDesc matrix {};
		matrix.kind = TypeKind::eMatrix;
		matrix.scalar = ScalarKind::eInt;
		matrix.n = 3uL;
		matrix.m = 3uL;
		mod.types = { matrix };
		mod.bindings = {
			BindingDecl { "x", TypeRef { 0u }, BindMode::eBind, BindClass::eUniformBuffer, false, true, {}, "x" }
		};

		const auto result = spirv::emit(mod, layoutOf(mod), Tier::eDev);

		EXPECT_FALSE(result.diagnostics.empty());
		EXPECT_TRUE(result.words.empty());
	}

	/* Capabilities */

	TEST(SpirvCapabilityTest, FloatOnlyNeedsNoExtraCapability) {
		const auto emission = emit(uniformOf("f32mat4"));

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_EQ(emission.text.find("OpCapability "), emission.text.rfind("OpCapability ")) << emission.text;
		EXPECT_TRUE(emission.has("Shader"));
		EXPECT_FALSE(emission.text.contains("OpExtension")) << emission.text;
	}

	TEST(SpirvCapabilityTest, EightBitStorageNeedsInt8AndStorageOnly) {
		const auto emission = emit(uniformOf("u8vec4"));

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.has("Int8")) << emission.text;
		EXPECT_TRUE(emission.has("UniformAndStorageBuffer8BitAccess")) << emission.text;
		EXPECT_FALSE(emission.has("StorageBuffer8BitAccess")) << emission.text;
		EXPECT_FALSE(emission.has("StoragePushConstant8")) << emission.text;
	}

	TEST(SpirvCapabilityTest, StorageCapabilityFollowsTheBindingClass) {
		const auto buffer = emit(
			"@input (mode = bind, id = \"x\", class = storage) block x { s8[] values; };\nfn main() {\n\tlet y = x.values[0];\n}\n"
		);
		const auto push = emit("@input (mode = bind, id = \"x\", class = push) block x { u8 flag; };\nfn main() {\n\tlet y = x.flag;\n}\n");

		ASSERT_TRUE(buffer.diagnostics.empty()) << buffer.diagnostics.front().message();
		ASSERT_TRUE(push.diagnostics.empty()) << push.diagnostics.front().message();
		EXPECT_TRUE(buffer.has("StorageBuffer8BitAccess")) << buffer.text;
		EXPECT_FALSE(buffer.has("StoragePushConstant8")) << buffer.text;
		EXPECT_TRUE(push.has("StoragePushConstant8")) << push.text;
		EXPECT_FALSE(push.has("StorageBuffer8BitAccess")) << push.text;
		EXPECT_FALSE(push.has("UniformAndStorageBuffer8BitAccess")) << push.text;
	}

	TEST(SpirvCapabilityTest, StorageIsReachedThroughNamedStructs) {
		const auto emission = emit(
			"struct Inner { u16vec2 uv; }\n"
			"struct Outer { Inner[2] items; }\n"
			"@input (mode = bind, id = \"x\", class = uniform) block x { Outer data; };\n"
			"fn main() {\n\tlet y = x.data.items[1].uv;\n}\n"
		);

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.has("Int16")) << emission.text;
		EXPECT_TRUE(emission.has("UniformAndStorageBuffer16BitAccess") || emission.has("StorageUniform16")) << emission.text;
	}

	TEST(SpirvCapabilityTest, LocalOnlyWideTypeNeedsNoStorageCapability) {
		const auto emission = emit(
			"fn twice(a: f16) -> f16 { return a + a; }\n"
			"fn main() { let x = twice(1.0); }\n"
		);

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.has("Float16")) << emission.text;
		EXPECT_FALSE(emission.text.contains("16BitAccess")) << emission.text;
		EXPECT_FALSE(emission.text.contains("Storage")) << emission.text;
	}

	TEST(SpirvCapabilityTest, DoubleNeedsFloat64) {
		const auto emission = emit(uniformOf("f64vec3"));

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.has("Float64")) << emission.text;
		EXPECT_FALSE(emission.text.contains("OpExtension")) << emission.text;
	}

	TEST(SpirvCapabilityTest, SixtyFourBitIntegerNeedsInt64) {
		const auto emission = emit(uniformOf("s64vec2"));

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.has("Int64")) << emission.text;
	}

	TEST(SpirvCapabilityTest, HalfFloatMatrixNeedsFloat16AndItsStorage) {
		const auto emission = emit(uniformOf("f16mat3x4"));

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.has("Float16")) << emission.text;
		EXPECT_TRUE(emission.has("UniformAndStorageBuffer16BitAccess") || emission.has("StorageUniform16")) << emission.text;
	}

	TEST(SpirvCapabilityTest, EightBitForwardVaryingIsDiagnostic) {
		const auto emission = emit("@input (mode = forward, id = \"v\") u8vec4 v;\nfn main() {\n\tlet y = v;\n}\n");

		ASSERT_FALSE(emission.diagnostics.empty());
		EXPECT_TRUE(emission.words.empty());
		EXPECT_EQ(emission.diagnostics.front().code()->key, "unsupported-feature");
		EXPECT_TRUE(emission.diagnostics.front().message().contains("8-bit")) << emission.diagnostics.front().message();
	}

	TEST(SpirvCapabilityTest, SixteenBitForwardVaryingNeedsInterfaceCapability) {
		const auto emission = emit("@output (mode = forward, id = \"v\") f16vec2 v;\nfn main() {\n\tv = f16vec2(f16(1.0));\n}\n");

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.has("StorageInputOutput16")) << emission.text;
	}

	namespace {
		[[nodiscard]] Emission emitSliced(StringView source_) {
			const std::array<String, 1> required { "color" };
			return emitModule(resolved(source_, EntryKind::eFragment, required));
		}

		constexpr StringView halfHelper = "fn twice(x: f16) -> f16 {\n\treturn x * f16(2.0);\n}\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n";
	}

	TEST(SpirvCapabilityTest, UnusedHalfFloatFunctionIsSlicedWithItsCapability) {
		const auto emission = emitSliced(String { halfHelper } + "fn main() {\n\tcolor = f32vec4(1.0);\n}\n");

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_FALSE(emission.text.contains("\"twice")) << emission.text;
		EXPECT_FALSE(emission.has("Float16")) << emission.text;
	}

	TEST(SpirvCapabilityTest, UsedHalfFloatFunctionKeepsItsCapability) {
		const auto emission = emitSliced(String { halfHelper } + "fn main() {\n\tcolor = f32vec4(f32(twice(f16(1.0))));\n}\n");

		ASSERT_TRUE(emission.diagnostics.empty()) << emission.diagnostics.front().message();
		EXPECT_TRUE(emission.text.contains("\"twice")) << emission.text;
		EXPECT_TRUE(emission.has("Float16")) << emission.text;

		EXPECT_TRUE(emission.text.contains("0x1p+1")) << emission.text;
	}
}
