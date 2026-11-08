#include "pch.h"

#include <array>
#include <regex>
#include <string>

#include <Engine.Accel.Lang/Emitter/GlslEmitter.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Lower.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Slice.hpp>
#include <Engine.Accel.Lang/Rep/Annotation.hpp>
#include <Engine.Accel.Lang/Rep/Symbol.hpp>

#include "IrSame.hpp"

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] Module resolved(StringView source_, EntryKind entry_, std::span<const String> outputs_ = {}) {
			auto parsed = parseModule("M", source_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(parsed.has_value()) << (parsed.has_value() || parsed.error().empty() ? "" : parsed.error().front().message);
			if (not parsed.has_value()) {
				return {};
			}
			auto res = resolveToggles(*parsed, ToggleSet {});
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? slice(*res, outputs_) : Module {};
		}

		[[nodiscard]] Layout layoutOf(ref<const Module> module_) {
			auto layout = assignLayout(computeInterface(module_), module_.types, {});
			layout.specConstants = assignSpecConstants(module_.constants);
			return layout;
		}

		[[nodiscard]] Intermediate lower(ref<const Module> module_, ref<const Layout> layout_) {
			const DenseMap<String, u32> offsets { { "vertex-position", 12u } };
			auto res = lowerToIntermediate(module_, layout_, Dialect::eVulkanGlsl460, offsets);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Intermediate {};
		}

		[[nodiscard]] String textOf(ref<const Intermediate> im_) {
			return im_.lang.text.empty() ? String {} : im_.lang.text.front().data;
		}

		[[nodiscard]] const Variable* variableOf(ref<const Intermediate> im_, StringView id_) {
			const auto symbol = std::ranges::find_if(
				im_.rep.symbolTable,
				[&](const auto& s_) { return s_->symbolId == SymbolId::from(id_); }
			);
			return symbol != im_.rep.symbolTable.end() ? (*symbol)->var.data.get() : nullptr;
		}

		[[nodiscard]] Vector<AnnotationType> chainOf(const Variable* var_) {
			Vector<AnnotationType> kinds {};
			for (auto* cur = var_ != nullptr ? var_->annotation.get() : nullptr; cur != nullptr; cur = cur->next.get()) {
				kinds.emplace_back(cur->type);
			}
			return kinds;
		}

		constexpr auto shadowVertex =
			"@input (mode = bind, id = \"view\", class = storage) block view { f32mat4[4] cascadeViewProj; f32vec4 cascadeDepth; };\n"
			"@input (mode = bind, id = \"model\", class = storage) block model { f32mat4[] model; };\n"
			"@input (mode = bind, id = \"block\", class = push) block push { u32 cascade; };\n"
			"@input (mode = forward, id = \"vertex-position\") f32vec3 local_position;\n"
			"fn main() {\n"
			"\tlet pos = model.model[instance_index] * f32vec4(local_position, 1.0);\n"
			"\tposition = view.cascadeViewProj[push.cascade] * pos;\n"
			"}\n";

		constexpr auto shadowFragment = "@output (mode = forward, id = \"depth\") implicit f32 depth;\n";
	}

	TEST(LowerTest, EmitsConstantIdFromLayout) {
		const auto module = resolved(
			"@spec (id = \"b\") const B: u32 = 2u;\n"
			"@spec (id = \"a\") const A: u32 = 1u;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tcolor = f32vec4(f32(A + B));\n}\n",
			EntryKind::eFragment
		);
		const auto layout = layoutOf(module);
		ASSERT_EQ(layout.specConstants.size(), 2uz);
		const auto im = lower(module, layout);
		const auto text = textOf(im);
		EXPECT_NE(text.find("layout(constant_id = 1) const uint B = 2u;"), String::npos) << text;
		EXPECT_NE(text.find("layout(constant_id = 0) const uint A = 1u;"), String::npos) << text;
		EXPECT_NE(variableOf(im, "a"), nullptr);
		EXPECT_NE(variableOf(im, "b"), nullptr);
	}

	TEST(LowerTest, EmitsStd140Packing) {
		const auto module = resolved(
			"@input (mode = bind, id = \"m\", class = uniform, packing = std140) block m { f32vec4 a; };\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tcolor = m.a;\n}\n",
			EntryKind::eFragment
		);
		const auto text = textOf(lower(module, layoutOf(module)));
		EXPECT_NE(text.find("std140"), String::npos) << text;
	}

	TEST(LowerTest, EmitsLayoutNumbersFromLayoutPass) {
		const auto module = resolved(shadowVertex, EntryKind::eVertex);
		const auto layout = layoutOf(module);
		ASSERT_FALSE(layout.slots.empty());
		ASSERT_EQ(layout.slots.front().set, 0u);
		ASSERT_EQ(layout.slots.front().binding, 0u);

		const auto text = textOf(lower(module, layout));

		EXPECT_TRUE(text.contains("layout(set = 0, binding = 0) readonly buffer")) << text;
		EXPECT_TRUE(text.contains("layout(push_constant) uniform")) << text;

		// every set/binding pair is one of the layout's slots, each slot appears once
		const std::regex slotPattern { R"(set = (\d+), binding = (\d+))" };
		auto slots = 0uz;
		for (auto it = std::sregex_iterator(text.begin(), text.end(), slotPattern); it != std::sregex_iterator {}; ++it, ++slots) {
			const auto set = static_cast<u32>(std::stoul((*it)[1].str()));
			const auto binding = static_cast<u32>(std::stoul((*it)[2].str()));
			EXPECT_TRUE(std::ranges::any_of(layout.slots, [&](ref<const SlotAssignment> s_) { return s_.set == set && s_.binding == binding; }))
				<< set << ", " << binding;
		}
		EXPECT_EQ(slots, layout.slots.size());

		// input locations come from the layout
		const std::regex locationPattern { R"(location = (\d+))" };
		auto locations = 0uz;
		for (auto it = std::sregex_iterator(text.begin(), text.end(), locationPattern); it != std::sregex_iterator {}; ++it, ++locations) {
			EXPECT_TRUE(std::ranges::contains(layout.inputLocations, static_cast<u32>(std::stoul((*it)[1].str()))));
		}
		EXPECT_EQ(locations, layout.inputLocations.size());
	}

	TEST(LowerTest, EmitsShadowVertexBody) {
		const auto module = resolved(shadowVertex, EntryKind::eVertex);
		const auto text = textOf(lower(module, layoutOf(module)));

		EXPECT_TRUE(text.starts_with("#version 460\n")) << text;
		EXPECT_TRUE(text.contains("buffer model_block {\n\tmat4 model[];\n} model;")) << text;
		EXPECT_TRUE(text.contains("layout(location = 0) in vec3 local_position;")) << text;
		EXPECT_TRUE(text.contains("vec4 pos = (model.model[gl_InstanceIndex] * vec4(local_position, 1.0));")) << text;
		EXPECT_TRUE(text.contains("gl_Position = (view.cascadeViewProj[push.cascade] * pos);")) << text;
		EXPECT_TRUE(text.contains("void main()")) << text;
	}

	TEST(LowerTest, AnnotationChainKindsMatchLegacyForPushBlock) {
		const auto module = resolved(shadowVertex, EntryKind::eVertex);
		const auto im = lower(module, layoutOf(module));

		using enum AnnotationType;
		const auto* push = variableOf(im, "block");
		ASSERT_NE(push, nullptr);
		EXPECT_EQ(chainOf(push), (Vector<AnnotationType> { eSymbolId, eReadonly, eConstant, eExternalLinkage }));
		EXPECT_EQ(static_cast<const SymbolIdAnnotation*>(push->annotation.get())->symbolId, "block");
		EXPECT_EQ(push->type, (Type { .category = TypeCategory::eObject, .objectType = ObjectType::eUnknown }));

		EXPECT_EQ(chainOf(variableOf(im, "view")), (Vector<AnnotationType> { eSymbolId, eStorage, eExternalLinkage }));

		const auto* position = variableOf(im, "vertex-position");
		ASSERT_NE(position, nullptr);
		EXPECT_EQ(chainOf(position), (Vector<AnnotationType> { eVkBindLocation, eSymbolId, eForwardLinkage }));
		EXPECT_EQ(position->type, (Type { .category = TypeCategory::eScalar, .scalarType = ScalarType::eF32Vec3 }));
		const auto* location = static_cast<const VkBindLocationAnnotation*>(position->annotation.get());
		EXPECT_EQ(location->vkSet, -1);
		EXPECT_EQ(location->vkLocation, 0);
		EXPECT_EQ(location->vkOffset, 12);

		EXPECT_EQ(im.rep.globalScope.inbound.size(), 4uz);
		EXPECT_EQ(im.rep.symbolTable.size(), 4uz);
	}

	TEST(LowerTest, VertexInputWithoutOffsetIsDiagnostic) {
		const auto module = resolved(shadowVertex, EntryKind::eVertex);

		const auto res = lowerToIntermediate(module, layoutOf(module), Dialect::eVulkanGlsl460);

		ASSERT_FALSE(res.has_value());
		EXPECT_TRUE(res.error().front().message.contains("'vertex-position'"));
	}

	TEST(LowerTest, LoweringTwiceGivesIndependentIntermediates) {
		const auto module = resolved(shadowVertex, EntryKind::eVertex);
		const auto copy = module;
		const auto layout = layoutOf(module);

		auto first = lower(module, layout);
		const auto second = lower(module, layout);

		auto* firstVar = const_cast<Variable*>(variableOf(first, "block"));
		const auto* secondVar = variableOf(second, "block");
		ASSERT_NE(firstVar, nullptr);
		ASSERT_NE(secondVar, nullptr);
		EXPECT_NE(firstVar, secondVar);

		static_cast<SymbolIdAnnotation*>(firstVar->annotation.get())->symbolId = "changed";
		firstVar->annotation->next = nullptr;

		EXPECT_EQ(static_cast<const SymbolIdAnnotation*>(secondVar->annotation.get())->symbolId, "block");
		EXPECT_EQ(chainOf(secondVar).size(), 4uz);
		EXPECT_TRUE(same(module, copy));
		ASSERT_NE(first.ir, nullptr);
		ASSERT_NE(second.ir, nullptr);
		EXPECT_TRUE(same(*first.ir, module));
		EXPECT_TRUE(same(*second.ir, module));
	}

	TEST(LowerTest, ImplicitOutputHasSymbolButNoGlslVariable) {
		const auto outputs = std::array { String("depth") };
		const auto module = resolved(shadowFragment, EntryKind::eFragment, outputs);

		const auto im = lower(module, layoutOf(module));

		ASSERT_EQ(im.rep.globalScope.outbound.size(), 1uz);
		const auto* depth = variableOf(im, "depth");
		ASSERT_NE(depth, nullptr);
		EXPECT_EQ(depth, im.rep.globalScope.outbound.front().get());
		EXPECT_EQ(chainOf(depth), (Vector<AnnotationType> { AnnotationType::eSymbolId, AnnotationType::eForwardLinkage }));

		const auto text = textOf(im);
		EXPECT_FALSE(text.contains("depth")) << text;
		EXPECT_TRUE(text.contains("void main() {\n}")) << text;
	}

	TEST(LowerTest, ForwardOutputsAreNumberedById) {
		const auto module = resolved(
			"@output (mode = forward, id = \"b\") f32mat4 second;\n"
			"@output (mode = forward, id = \"a\") f32vec4 first;\n"
			"@output (mode = forward, id = \"c\") f32 third;\n",
			EntryKind::eFragment
		);
		const auto text = textOf(lower(module, layoutOf(module)));

		EXPECT_TRUE(text.contains("layout(location = 0) out vec4 first;")) << text;
		EXPECT_TRUE(text.contains("layout(location = 1) out mat4 second;")) << text;
		EXPECT_TRUE(text.contains("layout(location = 5) out float third;")) << text;
	}

	TEST(LowerTest, SampleMapsToCombinedSampler) {
		const auto outputs = std::array { String("color") };
		const auto module = resolved(
			"@input (mode = bind, id = \"albedo\", class = texture) texture2d albedo;\n"
			"@input (mode = bind, id = \"smp\", class = sampler) sampler smp;\n"
			"@input (mode = forward, id = \"uv\") f32vec2 uv;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() { color = sample(albedo, smp, uv); }\n",
			EntryKind::eFragment,
			outputs
		);
		const auto text = textOf(lower(module, layoutOf(module)));

		EXPECT_TRUE(text.contains("uniform texture2D albedo;")) << text;
		EXPECT_TRUE(text.contains("uniform sampler smp;")) << text;
		EXPECT_TRUE(text.contains("color = texture(sampler2D(albedo, smp), uv);")) << text;
	}

	TEST(LowerTest, NamedStructIsEmittedOnceBeforeUse) {
		const auto module = resolved(
			"struct Light { f32vec4 color; }\n"
			"@input (mode = bind, id = \"lights\", class = uniform) block lights { Light[4] items; };\n"
			"@input (mode = bind, id = \"sun\", class = uniform) Light sun;\n",
			EntryKind::eFragment
		);
		const auto text = textOf(lower(module, layoutOf(module)));

		const auto def = text.find("struct Light {\n\tvec4 color;\n};");
		ASSERT_NE(def, String::npos) << text;
		EXPECT_EQ(text.find("struct Light", def + 1uz), String::npos) << text;
		EXPECT_LT(def, text.find("lights_block")) << text;
		EXPECT_TRUE(text.contains("Light items[4];")) << text;
		EXPECT_TRUE(text.contains("uniform sun_block {\n\tLight sun;\n};")) << text;
	}

	TEST(LowerTest, ConflictingStructNamesAreDiagnostic) {
		Module module {};
		module.entry = EntryKind::eFragment;
		TypeDesc f32 {};
		TypeDesc u32t {};
		u32t.scalar = ScalarKind::eUInt;
		TypeDesc a {};
		a.kind = TypeKind::eStruct;
		a.name = "S";
		a.fields = { FieldDesc { "x", TypeRef { 0u } } };
		TypeDesc b = a;
		b.fields = { FieldDesc { "x", TypeRef { 1u } } };
		module.types = { f32, u32t, a, b };
		module.bindings = {
			BindingDecl { "a", TypeRef { 2u }, BindMode::eBind, BindClass::eUniformBuffer, false, true, {}, "a" },
			BindingDecl { "b", TypeRef { 3u }, BindMode::eBind, BindClass::eUniformBuffer, false, true, {}, "b" }
		};

		Language target {};
		const auto diags = GlslEmitter {}.emit(module, layoutOf(module), target).diagnostics;

		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(diags[0].message.contains("'S'")) << diags[0].message;
		EXPECT_TRUE(target.text.empty());
	}

	TEST(LowerTest, PushBlockThatCannotBeLaidOutIsDiagnostic) {
		const auto module = resolved("@input (mode = bind, id = \"p\", class = push) block p { u32[] tail; };\n", EntryKind::eVertex);
		const auto layout = layoutOf(module);
		ASSERT_EQ(layout.push.size(), 1uz);
		ASSERT_EQ(layout.push.front().size, 0u);

		const auto res = lowerToIntermediate(module, layout, Dialect::eVulkanGlsl460);

		ASSERT_FALSE(res.has_value());
		EXPECT_TRUE(res.error().front().message.contains("'p'")) << res.error().front().message;
	}

	/* the lexer rejects `1u` and `0x1E`, so the literals are hand-built */
	TEST(LowerTest, LiteralSuffixAndHexInferTheirType) {
		Module module {};
		module.entry = EntryKind::eFragment;
		const auto literals = std::array<String, 5> { "1u", "0x1E", "0x1Eu", "1.5", "1e3" };
		const auto names = std::array<String, 5> { "a", "b", "c", "d", "e" };
		Stmt block {};
		block.kind = StmtKind::eBlock;
		for (auto i = 0uz; i < literals.size(); ++i) {
			Expr literal {};
			literal.name = literals[i];
			module.exprs.emplace_back(std::move(literal));
			Stmt local {};
			local.kind = StmtKind::eDeclareLocal;
			local.name = names[i];
			local.lhs = ExprRef { static_cast<u32>(i) };
			module.stmts.emplace_back(std::move(local));
			block.children.emplace_back(StmtRef { static_cast<u32>(i) });
		}
		module.stmts.emplace_back(std::move(block));
		FunctionDecl main {};
		main.name = "main";
		main.body = BodyRef { StmtRef { static_cast<u32>(literals.size()) } };
		module.functions.emplace_back(std::move(main));

		const auto text = textOf(lower(module, layoutOf(module)));

		EXPECT_TRUE(text.contains("uint a = 1u;")) << text;
		EXPECT_TRUE(text.contains("int b = 0x1E;")) << text;
		EXPECT_TRUE(text.contains("uint c = 0x1Eu;")) << text;
		EXPECT_TRUE(text.contains("float d = 1.5;")) << text;
		EXPECT_TRUE(text.contains("float e = 1e3;")) << text;
	}

	TEST(LowerTest, RefMutParamEmitsInout) {
		const auto module = resolved(
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"fn F(x: &mut f32, y: &f32, z: f32) { x = y + z; }\n"
			"fn main() { let a = 1.0; let b = 2.0; F(a, b, b); color = a; }\n",
			EntryKind::eFragment,
			std::array { String("color") }
		);
		const auto text = textOf(lower(module, layoutOf(module)));
		EXPECT_TRUE(text.contains("void F(inout float x, const in float y, const in float z)")) << text;
	}

	TEST(LowerTest, LocalRefEmitsNoVariableAndSubstitutesPlace) {
		const auto module = resolved(
			"@input (mode = bind, id = \"lights\", class = storage, access = read_write) block lights { f32[] lights; };\n"
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"fn main() { let l = &mut lights.lights[2]; l = 1.0; color = l; }\n",
			EntryKind::eFragment,
			std::array { String("color") }
		);
		const auto text = textOf(lower(module, layoutOf(module)));
		EXPECT_FALSE(text.contains("float l =") || text.contains("float l;")) << text;
		EXPECT_TRUE(text.contains("lights.lights[2] = 1.0;")) << text;
		EXPECT_TRUE(text.contains("color = lights.lights[2];")) << text;
	}

	TEST(LowerTest, LocalRefHoistsIndex) {
		const auto module = resolved(
			"@input (mode = bind, id = \"lights\", class = storage, access = read_write) block lights { f32[] lights; };\n"
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"fn main() { u32 i = 0; let l = &mut lights.lights[i]; i = i + 1; l = 1.0; color = l; }\n",
			EntryKind::eFragment,
			std::array { String("color") }
		);
		const auto text = textOf(lower(module, layoutOf(module)));
		const auto hoist = text.find("const uint _hg_ref0 = i;");
		ASSERT_NE(hoist, String::npos) << text;
		EXPECT_LT(hoist, text.find("i = (i + 1);")) << text;
		EXPECT_TRUE(text.contains("lights.lights[_hg_ref0] = 1.0;")) << text;
	}

TEST(LowerTest, EmitsCompoundAssignAndTernary) {
	const auto module = resolved(
		"@input (mode = forward) f32 bias;\n"
		"@output (mode = forward, id = \"color\") f32 color;\n"
		"fn main() {\n\tlet a = 0.0;\n\tfor (let i = 0u; i < 4u; ++i) { a += bias; if (a > 3.0) { break; } }\n"
		"\tcolor = bias > 0.0 ? a : -a;\n}\n",
		EntryKind::eFragment,
		std::array { String("color") }
	);
	const auto text = textOf(lower(module, layoutOf(module)));
	EXPECT_TRUE(text.contains("for (uint i = 0u; (i < 4u); i += 1)")) << text;
	EXPECT_TRUE(text.contains("a += ")) << text;
	EXPECT_TRUE(text.contains("break;")) << text;
	EXPECT_TRUE(text.contains(" ? ")) << text;
}

TEST(LowerTest, ForWithOmittedInitAndStepHasValidHeader) {
	const auto module = resolved(
		"@input (mode = forward) f32 bias;\n"
		"@output (mode = forward, id = \"color\") f32 color;\n"
		"fn main() {\n\tlet a = 0.0;\n\tlet c = bias > 1.0;\n\tfor (; c; ) { a += bias; break; }\n"
		"\tlet i = 0u;\n\tfor (; i < 4u; ) { a += bias; i++; }\n"
		"\tfor (let j = 0u; j < 4u; ) { a += bias; j++; }\n\tcolor = a;\n}\n",
		EntryKind::eFragment,
		std::array { String("color") }
	);
	const auto text = textOf(lower(module, layoutOf(module)));
	EXPECT_TRUE(text.contains("for (; c; )")) << text;
	EXPECT_TRUE(text.contains("for (; (i < 4u); )")) << text;
	EXPECT_TRUE(text.contains("for (uint j = 0u; (j < 4u); )")) << text;
	EXPECT_FALSE(text.contains("for ({")) << text;
}

TEST(LowerTest, SparseAddsExtensions) {
	const auto outputs = std::array { String("color") };
	const auto module = resolved(
		"@input (mode = bind, id = \"t2\", class = texture) texture2darray t2;\n"
		"@input (mode = bind, id = \"smp\", class = sampler) sampler smp;\n"
		"@input (mode = forward, id = \"uv\") f32vec3 uv;\n"
		"@output (mode = forward, id = \"color\") f32vec4 color;\n"
		"fn main() {\n"
		"\tlet a = f32vec4(0.0);\n\tlet b = f32vec4(0.0);\n"
		"\tlet r = sample_sparse(t2, smp, uv, a);\n"
		"\tlet q = sample_sparse_clamp(t2, smp, uv, 1.0, b);\n"
		"\tif (sparse_resident(r) && sparse_resident(q)) { color = a + b; }\n}\n",
		EntryKind::eFragment,
		outputs
	);
	const auto text = textOf(lower(module, layoutOf(module)));
	EXPECT_TRUE(text.contains("#extension GL_ARB_sparse_texture2 : require")) << text;
	EXPECT_TRUE(text.contains("#extension GL_ARB_sparse_texture_clamp : require")) << text;
	EXPECT_TRUE(text.contains("sparseTextureARB(sampler2DArray(t2, smp), uv, a)")) << text;
}

TEST(LowerTest, ViewIndexAddsMultiview) {
	const auto module = resolved("fn main() {\n\tposition = f32vec4(f32(view_index));\n}\n", EntryKind::eVertex);
	const auto text = textOf(lower(module, layoutOf(module)));
	EXPECT_TRUE(text.contains("GL_EXT_multiview")) << text;
	EXPECT_TRUE(text.contains("gl_ViewIndex")) << text;
}
}
