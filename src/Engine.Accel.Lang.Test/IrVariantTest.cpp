#include "pch.h"

#include <Engine.Accel.Lang/Intermediate.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Variant.hpp>
#include <Engine.Accel.Lang/Rep/Annotation.hpp>
#include <Engine.Common/Make.hpp>

using namespace hg;
using namespace hg::engine::accel::lang;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] Module parsed(StringView source_) {
			auto res = parseModule("M", source_, EntryKind::eVertex, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? std::move(res).value() : Module {};
		}

		constexpr auto source =
			"toggle Mode : { Forward, Deferred } = Forward;\n"
			"@input (mode = bind, id = \"model\", class = storage) block model { f32mat4[] model; };\n"
			"@input (mode = forward, id = \"vertex-position\") f32vec3 local_position;\n"
			"fn main() {\n"
			"\tlet pos = model.model[instance_index] * f32vec4(local_position, 1.0);\n"
			"\tposition = pos;\n"
			"}\n";

		/* same content, but a dead type at arena index 0 (every live type index shifted) and dead code nodes */
		[[nodiscard]] Module withDeadNodes(Module module_) {
			const auto shift = [](ref<TypeRef> ref_) {
				if (ref_.index != ~0uL) {
					++ref_.index;
				}
			};
			for (auto& type : module_.types) {
				shift(type.element);
				for (auto& field : type.fields) {
					shift(field.type);
				}
			}
			for (auto& binding : module_.bindings) {
				shift(binding.type);
			}
			for (auto& function : module_.functions) {
				shift(function.ret);
				for (auto& param : function.params) {
					shift(param.type);
				}
			}
			for (auto& expr : module_.exprs) {
				shift(expr.type);
			}
			for (auto& stmt : module_.stmts) {
				shift(stmt.type);
			}
			auto& deadType = *module_.types.emplace(module_.types.begin());
			deadType.kind = TypeKind::eStruct;
			deadType.name = "Dead";
			module_.exprs.emplace_back().name = "42";
			module_.stmts.emplace_back().kind = StmtKind::eReturn;
			return module_;
		}

		[[nodiscard]] Intermediate handBuilt(StringView symbolId_) {
			Intermediate im {};
			im.lang.dialect = Dialect::eVulkanGlsl460;
			im.lang.text.emplace_back(TextBlock { "void main() {}" });

			auto var = make_uptr<Variable>();
			var->type = Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32 };
			var->annotation = make_uptr<SymbolIdAnnotation>(
				String { symbolId_ },
				make_uptr<VkBindLocationAnnotation>(-1L, 0L, 12L, make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>())
			);
			im.rep.globalScope.inbound.emplace_back(std::move(var));
			return im;
		}
	}

	TEST(VariantKeyTest, SameContentDifferentAddressSameKey) {
		const auto a = parsed(source);
		const auto b = parsed(source);
		const auto dead = withDeadNodes(a);

		EXPECT_EQ(contentHash(a), contentHash(b));
		EXPECT_EQ(contentHash(a), contentHash(dead));
		EXPECT_EQ(makeVariantKey(a, {}, {}), makeVariantKey(b, {}, {}));

		auto changed = a;
		changed.bindings.front().id = "other";
		EXPECT_NE(contentHash(a), contentHash(changed));
	}

	TEST(VariantKeyTest, ToggleOrderAndOutputOrderIrrelevant) {
		const auto module = parsed(source);
		const ToggleSet ab { { "A", "x" }, { "B", "y" } };
		const ToggleSet ba { { "B", "y" }, { "A", "x" } };
		const Vector<String> o1 { "color", "depth" };
		const Vector<String> o2 { "depth", "color" };

		EXPECT_EQ(makeVariantKey(module, ab, o1), makeVariantKey(module, ba, o2));
		EXPECT_EQ(std::hash<VariantKey> {}(makeVariantKey(module, ab, o1)), std::hash<VariantKey> {}(makeVariantKey(module, ba, o2)));
	}

	TEST(VariantKeyTest, TessSpacingChangesKey) {
		const auto eval = [](StringView spacing_) {
			const String source = String { "@tess_eval (domain = triangles, spacing = " } + String { spacing_ } +
				", winding = ccw)\nfn main() {\n\tposition = f32vec4(tess_coord, 1.0);\n}\n";
			auto res = parseModule("M", source, EntryKind::eTessEval, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? std::move(res).value() : Module {};
		};
		EXPECT_NE(contentHash(eval("equal")), contentHash(eval("fractional_even")));
		EXPECT_EQ(contentHash(eval("equal")), contentHash(eval("equal")));
	}

	TEST(VariantKeyTest, SpecDefaultChangesKey) {
		const auto a = parsed("@spec (id = \"s\") const S: u32 = 1u;\nfn main() {}\n");
		const auto b = parsed("@spec (id = \"s\") const S: u32 = 2u;\nfn main() {}\n");
		const auto c = parsed("@spec (id = \"t\") const S: u32 = 1u;\nfn main() {}\n");
		EXPECT_NE(makeVariantKey(a, {}, {}), makeVariantKey(b, {}, {}));
		EXPECT_NE(makeVariantKey(a, {}, {}), makeVariantKey(c, {}, {}));
		EXPECT_EQ(makeVariantKey(a, {}, {}), makeVariantKey(parsed("@spec (id = \"s\") const S: u32 = 1u;\nfn main() {}\n"), {}, {}));
	}

	TEST(VariantKeyTest, DifferentToggleOrOutputChangesKey) {
		const auto module = parsed(source);
		const Vector<String> color { "color" };
		const Vector<String> depth { "depth" };
		const auto base = makeVariantKey(module, { { "Mode", "Forward" } }, color);

		EXPECT_NE(base, makeVariantKey(module, { { "Mode", "Deferred" } }, color));
		EXPECT_NE(base, makeVariantKey(module, { { "Mode", "Forward" } }, depth));
		EXPECT_NE(base, makeVariantKey(module, { { "Mode", "Forward" } }, {}));
	}

	TEST(VariantCacheTest, SecondLookupDoesNotBuild) {
		VariantCache cache {};
		const auto key = makeVariantKey(parsed(source), {}, {});
		EXPECT_EQ(cache.find(key), nullptr);

		auto builds = 0;
		const auto build = [&builds]() {
			++builds;
			return make_smr<const Resolved>(new Resolved {});
		};

		const auto first = cache.getOrCreate(key, build);
		const auto second = cache.getOrCreate(key, build);

		EXPECT_EQ(builds, 1);
		ASSERT_NE(first, nullptr);
		EXPECT_EQ(first.get(), second.get());
		EXPECT_EQ(cache.find(key).get(), first.get());
	}

	TEST(IntermediateHashTest, ContentNotAddress) {
		const auto a = handBuilt("vertex-position");
		const auto b = handBuilt("vertex-position");
		const auto c = handBuilt("other");

		EXPECT_EQ(a.contentHash(), b.contentHash());
		EXPECT_NE(a.contentHash(), c.contentHash());

		auto withIr = handBuilt("vertex-position");
		withIr.ir = make_smr<const Module>(new Module { parsed(source) });
		EXPECT_NE(a.contentHash(), withIr.contentHash());
	}

	TEST(VariantKeyTest, RefKindAndMutabilityChangeKey) {
		const auto key = [](const char* param_, const char* local_) {
			return makeVariantKey(parsed(String { "fn F(x: " } + param_ + ") {}\nfn main() {\n	" + local_ + " k = 1.0;\n}\n"), {}, {});
		};
		EXPECT_NE(key("f32", "let"), key("&f32", "let"));
		EXPECT_NE(key("&f32", "let"), key("&mut f32", "let"));
		EXPECT_NE(key("f32", "let"), key("f32", "const"));
	}

	TEST(VariantKeyTest, CompoundOperatorChangesKey) {
		const auto key = [](const char* op_) {
			return makeVariantKey(parsed(String { "fn main() {\n	let k = 1.0;\n	k " } + op_ + " 2.0;\n	position = f32vec4(k);\n}\n"), {}, {});
		};
		EXPECT_NE(key("+="), key("-="));
		EXPECT_NE(key("+="), key("="));
	}
}
