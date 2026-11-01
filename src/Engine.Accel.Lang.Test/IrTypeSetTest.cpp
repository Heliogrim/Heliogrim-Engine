#include "pch.h"

#include <array>
#include <set>
#include <string>

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Lower.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Slice.hpp>
#include <Engine.Accel.Lang/Rep/Symbol.hpp>

using namespace hg;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] Module resolved(StringView source_, EntryKind entry_) {
			auto parsed = parseModule("M", source_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(parsed.has_value()) << (parsed.has_value() || parsed.error().empty() ? "" : parsed.error().front().message);
			if (not parsed.has_value()) {
				return {};
			}
			auto res = resolveToggles(*parsed, ToggleSet {});
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? slice(*res, {}) : Module {};
		}

		[[nodiscard]] Layout layoutOf(ref<const Module> module_) {
			return assignLayout(computeInterface(module_), module_.types, {});
		}

		struct Emission {
			GlslEmitResult result;
			String text;
		};

		[[nodiscard]] Emission emit(StringView source_, EntryKind entry_ = EntryKind::eFragment) {
			const auto module = resolved(source_, entry_);
			Language target {};
			auto result = GlslEmitter {}.emit(module, layoutOf(module), target);
			return Emission { std::move(result), target.text.empty() ? String {} : target.text.front().data };
		}

		[[nodiscard]] String uniformOf(StringView type_) {
			return "@input (mode = bind, id = \"x\", class = uniform) " + String { type_ } + " x;\n";
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
					{ "u8", "uint8_t", "u8vec" }, { "u16", "uint16_t", "u16vec" }, { "u32", "uint", "uvec" }, { "u64", "uint64_t", "u64vec" },
					{ "s8", "int8_t", "i8vec" }, { "s16", "int16_t", "i16vec" }, { "s32", "int", "ivec" }, { "s64", "int64_t", "i64vec" }
				}
			};
			constexpr std::array<const char*, 3> matrices { "f16mat", "mat", "dmat" };

			Vector<TypeCase> cases {};
			for (const auto& family : families) {
				cases.emplace_back(TypeCase { family.source, family.scalar });
				for (auto n = 2; n <= 4; ++n) {
					cases.emplace_back(TypeCase { String { family.source } + "vec" + std::to_string(n), String { family.vector } + std::to_string(n) });
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

	TEST_P(TypeSetTest, EmitsExactGlslTypeName) {
		const auto& [source, glsl] = GetParam();

		const auto emission = emit(uniformOf(source));

		ASSERT_TRUE(emission.result.diagnostics.empty()) << source << ": " << emission.result.diagnostics.front().message;
		EXPECT_TRUE(emission.text.contains("\t" + glsl + " x;\n")) << source << " -> " << glsl << "\n" << emission.text;
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
			const auto module = resolved(uniformOf(source), EntryKind::eFragment);
			auto res = lowerToIntermediate(module, layoutOf(module), Dialect::eVulkanGlsl460);
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
		EXPECT_TRUE(diag.message.contains("integer matrix")) << diag.message;
		EXPECT_TRUE(diag.message.contains("s32mat4")) << diag.message;
		EXPECT_EQ(diag.line, 2u);
	}

	TEST(TypeSetTest, IntegerMatrixInModuleIsDiagnosticNotWrongText) {
		Module module {};
		TypeDesc matrix {};
		matrix.kind = TypeKind::eMatrix;
		matrix.scalar = ScalarKind::eInt;
		matrix.n = 3uL;
		matrix.m = 3uL;
		module.types = { matrix };
		module.bindings = {
			BindingDecl { "x", TypeRef { 0u }, BindMode::eBind, BindClass::eUniformBuffer, false, true, {}, "x" }
		};

		Language target {};
		const auto result = GlslEmitter {}.emit(module, layoutOf(module), target);

		EXPECT_FALSE(result.diagnostics.empty());
		EXPECT_TRUE(target.text.empty());
	}

	/* Extensions */

	TEST(GlslExtensionTest, FloatOnlyEmitsNoExtensionLine) {
		const auto emission = emit(uniformOf("f32mat4"));

		EXPECT_TRUE(emission.result.requirements.extensions.empty());
		EXPECT_EQ(emission.result.requirements.features, 0u);
		EXPECT_FALSE(emission.text.contains("#extension")) << emission.text;
	}

	TEST(GlslExtensionTest, EightBitStorageNeedsInt8AndStorageOnly) {
		const auto emission = emit(uniformOf("u8vec4"));

		const Vector<String> expected { "GL_EXT_shader_8bit_storage", "GL_EXT_shader_explicit_arithmetic_types_int8" };
		EXPECT_EQ(emission.result.requirements.extensions, expected);
		EXPECT_TRUE(emission.result.requirements.has(GlslFeature::eShaderInt8));
		EXPECT_TRUE(emission.result.requirements.has(GlslFeature::eUniformAndStorageBuffer8BitAccess));
		EXPECT_FALSE(emission.result.requirements.has(GlslFeature::eStorageBuffer8BitAccess));
		EXPECT_FALSE(emission.text.contains("GL_EXT_shader_explicit_arithmetic_types :")) << emission.text;
	}

	TEST(GlslExtensionTest, StorageExtensionFollowsTheBindingClass) {
		const auto buffer = emit("@input (mode = bind, id = \"x\", class = storage) block x { s8[] values; };\n");
		const auto push = emit("@input (mode = bind, id = \"x\", class = push) block x { u8 flag; };\n");

		EXPECT_TRUE(buffer.result.requirements.has(GlslFeature::eStorageBuffer8BitAccess));
		EXPECT_FALSE(buffer.result.requirements.has(GlslFeature::eStoragePushConstant8));
		EXPECT_TRUE(push.result.requirements.has(GlslFeature::eStoragePushConstant8));
		EXPECT_FALSE(push.result.requirements.has(GlslFeature::eStorageBuffer8BitAccess));
	}

	TEST(GlslExtensionTest, StorageIsReachedThroughNamedStructs) {
		const auto emission = emit(
			"struct Inner { u16vec2 uv; }\n"
			"struct Outer { Inner[2] items; }\n"
			"@input (mode = bind, id = \"x\", class = uniform) block x { Outer data; };\n"
		);

		const Vector<String> expected { "GL_EXT_shader_16bit_storage", "GL_EXT_shader_explicit_arithmetic_types_int16" };
		EXPECT_EQ(emission.result.requirements.extensions, expected) << emission.text;
	}

	TEST(GlslExtensionTest, LocalOnlyWideTypeNeedsNoStorageExtension) {
		const auto emission = emit(
			"fn twice(a: f16) -> f16 { return a + a; }\n"
			"fn main() { let x = twice(1.0); }\n"
		);

		ASSERT_TRUE(emission.result.diagnostics.empty()) << emission.result.diagnostics.front().message;
		const Vector<String> expected { "GL_EXT_shader_explicit_arithmetic_types_float16" };
		EXPECT_EQ(emission.result.requirements.extensions, expected) << emission.text;
	}

	TEST(GlslExtensionTest, DoubleNeedsNoExtensionButTheDeviceFeature) {
		const auto emission = emit(uniformOf("f64vec3"));

		EXPECT_TRUE(emission.result.requirements.extensions.empty());
		EXPECT_TRUE(emission.result.requirements.has(GlslFeature::eShaderFloat64));
	}

	TEST(GlslExtensionTest, SixtyFourBitIntegerNeedsInt64) {
		const auto emission = emit(uniformOf("s64vec2"));

		const Vector<String> expected { "GL_EXT_shader_explicit_arithmetic_types_int64" };
		EXPECT_EQ(emission.result.requirements.extensions, expected);
		EXPECT_TRUE(emission.result.requirements.has(GlslFeature::eShaderInt64));
	}

	TEST(GlslExtensionTest, HalfFloatMatrixNeedsFloat16AndItsStorage) {
		const auto emission = emit(uniformOf("f16mat3x4"));

		const Vector<String> expected { "GL_EXT_shader_16bit_storage", "GL_EXT_shader_explicit_arithmetic_types_float16" };
		EXPECT_EQ(emission.result.requirements.extensions, expected);
		EXPECT_TRUE(emission.result.requirements.has(GlslFeature::eShaderFloat16));
	}

	TEST(GlslExtensionTest, ExtensionLinesAreSortedAndPrecedeCode) {
		const auto emission = emit(
			"@input (mode = bind, id = \"x\", class = uniform) block x { s64 a; u8 b; f16 c; u16 d; };\n"
		);

		const auto at = [&](StringView name_) { return emission.text.find("#extension " + String { name_ } + " : require\n"); };
		const auto first = at("GL_EXT_shader_16bit_storage");
		const auto second = at("GL_EXT_shader_8bit_storage");
		const auto third = at("GL_EXT_shader_explicit_arithmetic_types_float16");
		const auto fourth = at("GL_EXT_shader_explicit_arithmetic_types_int16");
		const auto fifth = at("GL_EXT_shader_explicit_arithmetic_types_int64");
		const auto sixth = at("GL_EXT_shader_explicit_arithmetic_types_int8");

		ASSERT_NE(first, String::npos) << emission.text;
		EXPECT_LT(first, second);
		EXPECT_LT(second, third);
		EXPECT_LT(third, fourth);
		EXPECT_LT(fourth, fifth);
		EXPECT_LT(fifth, sixth);
		EXPECT_LT(sixth, emission.text.find("layout(std430)"));
		EXPECT_FALSE(emission.text.contains("GL_EXT_shader_explicit_arithmetic_types :"));
	}

	TEST(GlslExtensionTest, EightBitForwardVaryingIsDiagnostic) {
		const auto emission = emit("@input (mode = forward, id = \"v\") u8vec4 v;\n");

		ASSERT_FALSE(emission.result.diagnostics.empty());
		EXPECT_TRUE(emission.result.diagnostics.front().message.contains("8-bit")) << emission.result.diagnostics.front().message;
		EXPECT_TRUE(emission.text.empty());
	}

	TEST(GlslExtensionTest, SixteenBitForwardVaryingNeedsInterfaceFeature) {
		const auto emission = emit("@output (mode = forward, id = \"v\") f16vec2 v;\n");

		ASSERT_TRUE(emission.result.diagnostics.empty()) << emission.result.diagnostics.front().message;
		EXPECT_TRUE(emission.result.requirements.has(GlslFeature::eStorageInputOutput16));
	}
}
