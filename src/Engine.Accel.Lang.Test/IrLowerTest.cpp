#include "pch.h"

/**/

#include <spirv-tools/libspirv.hpp>

/**/

#include "IrSame.hpp"
#include "IrSpirvText.hpp"
#include "IrWording.hpp"

import Engine.Accel.Lang;
import std;

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
			EXPECT_TRUE(parsed.has_value()) << (parsed.has_value() || parsed.error().empty() ? "" : parsed.error().front().message());
			if (not parsed.has_value()) {
				return {};
			}
			auto res = resolveToggles(*parsed, ToggleSet {});
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? slice(*res, outputs_) : Module {};
		}

		[[nodiscard]] Layout layoutOf(ref<const Module> mod_) {
			auto layout = assignLayout(computeInterface(mod_), mod_.types, {});
			layout.specConstants = assignSpecConstants(mod_.constants);
			return layout;
		}

		[[nodiscard]] Intermediate lower(ref<const Module> mod_, ref<const Layout> layout_) {
			const DenseMap<String, u32> offsets { { "vertex-position", 12u } };
			auto res = lowerToIntermediate(mod_, layout_, Tier::eDev, offsets);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message());
			if (not res.has_value()) {
				return Intermediate {};
			}
			return std::move(res).value();
		}

		/* validated disassembly of the production spir-v */
		[[nodiscard]] String textOf(ref<const Intermediate> im_) {
			EXPECT_FALSE(im_.spirv.empty());
			spvtools::SpirvTools tools { SPV_ENV_VULKAN_1_3 };
			EXPECT_TRUE(tools.Validate(im_.spirv));
			std::string text {};
			EXPECT_TRUE(
				tools.Disassemble(im_.spirv, &text, SPV_BINARY_TO_TEXT_OPTION_NO_HEADER | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES)
			);
			return String { text };
		}

		[[nodiscard]] size_t count(StringView text_, StringView needle_) {
			auto n = 0uz;
			for (auto at = text_.find(needle_); at != StringView::npos; at = text_.find(needle_, at + 1uz)) {
				++n;
			}
			return n;
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
		const auto mod = resolved(
			"@spec (id = \"b\") const B: u32 = 2u;\n"
			"@spec (id = \"a\") const A: u32 = 1u;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tcolor = f32vec4(f32(A + B));\n}\n",
			EntryKind::eFragment
		);
		const auto layout = layoutOf(mod);
		ASSERT_EQ(layout.specConstants.size(), 2uz);
		const auto im = lower(mod, layout);
		const auto text = textOf(im);
		EXPECT_TRUE(text.contains("OpDecorate %B SpecId 1\n")) << text;
		EXPECT_TRUE(text.contains("OpDecorate %A SpecId 0\n")) << text;
		EXPECT_TRUE(text.contains("%B = OpSpecConstant %uint 2\n")) << text;
		EXPECT_TRUE(text.contains("%A = OpSpecConstant %uint 1\n")) << text;
		EXPECT_NE(variableOf(im, "a"), nullptr);
		EXPECT_NE(variableOf(im, "b"), nullptr);
	}

	TEST(LowerTest, EmitsStd140Packing) {
		const auto mod = resolved(
			"@input (mode = bind, id = \"m\", class = uniform, packing = std140) block m { f32[2] a; f32vec4 b; };\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tcolor = m.b * m.a[1];\n}\n",
			EntryKind::eFragment
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));
		// std140 rounds the array stride and the next member up to 16 bytes
		EXPECT_TRUE(text.contains("ArrayStride 16\n")) << text;
		EXPECT_TRUE(text.contains(" 1 Offset 32\n")) << text;
	}

	TEST(LowerTest, EmitsLayoutNumbersFromLayoutPass) {
		const auto mod = resolved(shadowVertex, EntryKind::eVertex);
		const auto layout = layoutOf(mod);
		ASSERT_FALSE(layout.slots.empty());
		ASSERT_EQ(layout.slots.front().set, 0u);
		ASSERT_EQ(layout.slots.front().binding, 0u);

		const auto text = textOf(lower(mod, layout));

		EXPECT_TRUE(text.contains(" PushConstant ")) << text;
		EXPECT_TRUE(text.contains(" StorageBuffer ")) << text;
		EXPECT_TRUE(text.contains(" NonWritable\n")) << text;

		// every set/binding pair is one of the layout's slots, each slot appears once
		const std::regex slotPattern { R"(OpDecorate (%\S+) DescriptorSet (\d+)\n)" };
		auto slots = 0uz;
		for (auto it = std::sregex_iterator(text.begin(), text.end(), slotPattern); it != std::sregex_iterator {}; ++it, ++slots) {
			const auto set = static_cast<u32>(std::stoul((*it)[2].str()));
			std::smatch bound {};
			const std::regex bindingPattern { "OpDecorate " + (*it)[1].str() + R"( Binding (\d+)\n)" };
			ASSERT_TRUE(std::regex_search(text, bound, bindingPattern)) << text;
			const auto binding = static_cast<u32>(std::stoul(bound[1].str()));
			EXPECT_TRUE(
				std::ranges::any_of(layout.slots, [&](ref<const SlotAssignment> s_) { return s_.set == set && s_.binding == binding; })
			)
				<< set << ", " << binding;
		}
		EXPECT_EQ(slots, layout.slots.size());

		// input locations come from the layout
		const std::regex locationPattern { R"(OpDecorate %\S+ Location (\d+)\n)" };
		auto locations = 0uz;
		for (auto it = std::sregex_iterator(text.begin(), text.end(), locationPattern); it != std::sregex_iterator {}; ++it, ++locations) {
			EXPECT_TRUE(std::ranges::contains(layout.inputLocations, static_cast<u32>(std::stoul((*it)[1].str()))));
		}
		EXPECT_EQ(locations, layout.inputLocations.size());
	}

	TEST(LowerTest, EmitsShadowVertexBody) {
		const auto mod = resolved(shadowVertex, EntryKind::eVertex);
		const auto text = textOf(lower(mod, layoutOf(mod)));

		EXPECT_TRUE(text.contains("OpEntryPoint Vertex %main \"main\"")) << text;
		EXPECT_TRUE(text.contains("BuiltIn InstanceIndex\n")) << text;
		EXPECT_TRUE(text.contains("BuiltIn Position\n")) << text;
		EXPECT_TRUE(text.contains("OpTypeRuntimeArray %mat4v4float\n")) << text;
		EXPECT_TRUE(text.contains("OpDecorate %local_position Location 0\n")) << text;
		// model * vec4 and cascadeViewProj * pos
		EXPECT_EQ(count(text, "OpMatrixTimesVector"), 2uz) << text;
	}

	TEST(LowerTest, AnnotationChainKindsMatchLegacyForPushBlock) {
		const auto mod = resolved(shadowVertex, EntryKind::eVertex);
		const auto im = lower(mod, layoutOf(mod));

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
		const auto mod = resolved(shadowVertex, EntryKind::eVertex);

		const auto res = lowerToIntermediate(mod, layoutOf(mod), Tier::eDev);

		ASSERT_FALSE(res.has_value());
		expectWording(res.error().front());
		EXPECT_TRUE(res.error().front().message().contains("'vertex-position'"));
	}

	TEST(LowerTest, LoweringTwiceGivesIndependentIntermediates) {
		const auto mod = resolved(shadowVertex, EntryKind::eVertex);
		const auto copy = mod;
		const auto layout = layoutOf(mod);

		auto first = lower(mod, layout);
		const auto second = lower(mod, layout);

		auto* firstVar = const_cast<Variable*>(variableOf(first, "block"));
		const auto* secondVar = variableOf(second, "block");
		ASSERT_NE(firstVar, nullptr);
		ASSERT_NE(secondVar, nullptr);
		EXPECT_NE(firstVar, secondVar);

		static_cast<SymbolIdAnnotation*>(firstVar->annotation.get())->symbolId = "changed";
		firstVar->annotation->next = nullptr;

		EXPECT_EQ(static_cast<const SymbolIdAnnotation*>(secondVar->annotation.get())->symbolId, "block");
		EXPECT_EQ(chainOf(secondVar).size(), 4uz);
		EXPECT_TRUE(same(mod, copy));
		ASSERT_NE(first.ir, nullptr);
		ASSERT_NE(second.ir, nullptr);
		EXPECT_TRUE(same(*first.ir, mod));
		EXPECT_TRUE(same(*second.ir, mod));
	}

	TEST(LowerTest, ImplicitOutputHasSymbolButNoGlslVariable) {
		const auto outputs = std::array { String("depth") };
		const auto mod = resolved(shadowFragment, EntryKind::eFragment, outputs);

		const auto im = lower(mod, layoutOf(mod));

		ASSERT_EQ(im.rep.globalScope.outbound.size(), 1uz);
		const auto* depth = variableOf(im, "depth");
		ASSERT_NE(depth, nullptr);
		EXPECT_EQ(depth, im.rep.globalScope.outbound.front().get());
		EXPECT_EQ(chainOf(depth), (Vector<AnnotationType> { AnnotationType::eSymbolId, AnnotationType::eForwardLinkage }));

		const auto text = textOf(im);
		EXPECT_FALSE(text.contains("depth")) << text;
		EXPECT_TRUE(text.contains("OpLabel\nOpReturn\nOpFunctionEnd\n")) << text;
	}

	TEST(LowerTest, ForwardOutputsAreNumberedById) {
		const auto mod = resolved(
			"@output (mode = forward, id = \"b\") f32mat4 second;\n"
			"@output (mode = forward, id = \"a\") f32vec4 first;\n"
			"@output (mode = forward, id = \"c\") f32 third;\n"
			"fn main() {\n\tfirst = f32vec4(0.0);\n\tsecond = f32mat4(first, first, first, first);\n\tthird = 0.0;\n}\n",
			EntryKind::eFragment
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));

		EXPECT_TRUE(text.contains("OpDecorate %first Location 0\n")) << text;
		EXPECT_TRUE(text.contains("OpDecorate %second Location 1\n")) << text;
		EXPECT_TRUE(text.contains("OpDecorate %third Location 5\n")) << text;
	}

	TEST(LowerTest, SampleMapsToCombinedSampler) {
		const auto outputs = std::array { String("color") };
		const auto mod = resolved(
			"@input (mode = bind, id = \"albedo\", class = texture) texture2d albedo;\n"
			"@input (mode = bind, id = \"smp\", class = sampler) sampler smp;\n"
			"@input (mode = forward, id = \"uv\") f32vec2 uv;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() { color = sample(albedo, smp, uv); }\n",
			EntryKind::eFragment,
			outputs
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));

		EXPECT_TRUE(text.contains("OpTypeSampler\n")) << text;
		EXPECT_TRUE(text.contains("OpSampledImage ")) << text;
		EXPECT_TRUE(text.contains("OpImageSampleImplicitLod ")) << text;
	}

	TEST(LowerTest, NamedStructIsEmittedOnceBeforeUse) {
		const auto mod = resolved(
			"struct Light { f32vec4 color; }\n"
			"@input (mode = bind, id = \"lights\", class = uniform) block lights { Light[4] items; };\n"
			"@input (mode = bind, id = \"sun\", class = uniform) Light sun;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tcolor = lights.items[1].color + sun.color;\n}\n",
			EntryKind::eFragment
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));

		// one plain Light struct for the array element; the sun block needs its own Block-decorated copy (a Block struct cannot nest in another block)
		EXPECT_EQ(count(text, "= OpTypeStruct %v4float\n"), 2uz) << text;
		EXPECT_TRUE(text.contains("OpDecorate %Light_0 Block\n")) << text;
		EXPECT_FALSE(text.contains("OpDecorate %Light Block\n")) << text;
		EXPECT_TRUE(text.contains("OpTypeArray %Light ")) << text;
	}

	TEST(LowerTest, ConflictingStructNamesAreDiagnostic) {
		Module mod {};
		mod.entry = EntryKind::eFragment;
		TypeDesc f32 {};
		TypeDesc u32t {};
		u32t.scalar = ScalarKind::eUInt;
		TypeDesc a {};
		a.kind = TypeKind::eStruct;
		a.name = "S";
		a.fields = { FieldDesc { "x", TypeRef { 0u } } };
		TypeDesc b = a;
		b.fields = { FieldDesc { "x", TypeRef { 1u } } };
		mod.types = { f32, u32t, a, b };
		mod.bindings = {
			BindingDecl { "a", TypeRef { 2u }, BindMode::eBind, BindClass::eUniformBuffer, false, true, {}, "a" },
			BindingDecl { "b", TypeRef { 3u }, BindMode::eBind, BindClass::eUniformBuffer, false, true, {}, "b" }
		};

		const auto diags = AccelLang::spirvOf(mod).diagnostics;

		ASSERT_EQ(diags.size(), 1uz);
		expectWording(diags[0]);
		EXPECT_TRUE(diags[0].message().contains("'S'")) << diags[0].message();
	}

	TEST(LowerTest, PushBlockThatCannotBeLaidOutIsDiagnostic) {
		const auto mod = resolved(
			"@input (mode = bind, id = \"p\", class = push) block p { u32[] tail; };\nfn main() { position = f32vec4(f32(p.tail[0])); }\n",
			EntryKind::eVertex
		);
		const auto layout = layoutOf(mod);
		ASSERT_EQ(layout.push.size(), 1uz);
		ASSERT_EQ(layout.push.front().size, 0u);

		const auto res = lowerToIntermediate(mod, layout, Tier::eDev);

		ASSERT_FALSE(res.has_value());
		const auto push = std::ranges::find_if(res.error(), [](const auto& d_) { return d_.message().contains("'p'"); });
		ASSERT_NE(push, res.error().end()) << res.error().front().message();
		expectWording(*push);
	}

	/* a wrong literal type fails the typed declarations; the constants carry the inferred type */
	TEST(LowerTest, LiteralSuffixAndHexInferTheirType) {
		const auto mod = resolved(
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tu32 a = 7u;\n\ts32 b = 0x1E;\n\tu32 c = 0x1Eu;\n\tf32 d = 1.5;\n\tf32 e = 1e3;\n"
			"\tcolor = f32vec4(f32(a + c), f32(b), d, e);\n}\n",
			EntryKind::eFragment
		);
		const auto im = lower(mod, layoutOf(mod));
		ASSERT_FALSE(im.spirv.empty());

		spvtools::SpirvTools tools { SPV_ENV_VULKAN_1_3 };
		std::string text {};
		ASSERT_TRUE(tools.Validate(im.spirv));
		ASSERT_TRUE(tools.Disassemble(im.spirv, &text, SPV_BINARY_TO_TEXT_OPTION_NO_HEADER | SPV_BINARY_TO_TEXT_OPTION_FRIENDLY_NAMES));

		EXPECT_TRUE(text.contains("OpConstant %uint 7\n")) << text;
		EXPECT_TRUE(text.contains("OpConstant %int 30\n")) << text;
		EXPECT_TRUE(text.contains("OpConstant %uint 30\n")) << text;
		EXPECT_TRUE(text.contains("OpConstant %float 1.5\n")) << text;
		EXPECT_TRUE(text.contains("OpConstant %float 1000\n")) << text;
	}

	TEST(LowerTest, RefMutParamEmitsInout) {
		const auto mod = resolved(
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"fn F(x: &mut f32, y: &f32, z: f32) { x = y + z; }\n"
			"fn main() { let a = 1.0; let b = 2.0; F(a, b, b); color = a; }\n",
			EntryKind::eFragment,
			std::array { String("color") }
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));
		// x and y by reference, z by value
		EXPECT_EQ(count(text, "OpFunctionParameter %_ptr_Function_float\n"), 2uz) << text;
		EXPECT_EQ(count(text, "OpFunctionParameter %float\n"), 1uz) << text;
	}

	TEST(LowerTest, LocalRefEmitsNoVariableAndSubstitutesPlace) {
		const auto mod = resolved(
			"@input (mode = bind, id = \"lights\", class = storage, access = read_write) block lights { f32[] lights; };\n"
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"fn main() { let l = &mut lights.lights[2]; l = 1.0; color = l; }\n",
			EntryKind::eFragment,
			std::array { String("color") }
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));
		EXPECT_FALSE(text.contains("OpName %l ")) << text;
		EXPECT_FALSE(text.contains("%l = OpVariable")) << text;
		// the store and the load both go through lights.lights[2]
		EXPECT_TRUE(std::regex_search(text, std::regex { R"(OpAccessChain %\S+ %lights %\w+_0 %\w+_2\n)" })) << text;
	}

	TEST(LowerTest, LocalRefHoistsIndex) {
		const auto mod = resolved(
			"@input (mode = bind, id = \"lights\", class = storage, access = read_write) block lights { f32[] lights; };\n"
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"fn main() { u32 i = 0; let l = &mut lights.lights[i]; i = i + 1; l = 1.0; color = l; }\n",
			EntryKind::eFragment,
			std::array { String("color") }
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));
		// the index is read once into a hoisted local before `i = i + 1`, and only that local indexes the place
		std::smatch hoist {};
		ASSERT_TRUE(std::regex_search(text, hoist, std::regex { R"((%\S+) = OpLoad %uint %i\nOpStore (%\S+) \1\n)" })) << text;
		EXPECT_LT(static_cast<size_t>(hoist.position(0)), text.find("OpIAdd")) << text;
		const std::regex place { R"((%\S+) = OpLoad %uint (%\S+)\n%\S+ = OpAccessChain %\S+ %lights %\w+_0 \1\n)" };
		size_t places = 0uz;
		for (auto it = std::sregex_iterator(text.begin(), text.end(), place); it != std::sregex_iterator {}; ++it, ++places) {
			EXPECT_EQ((*it)[2].str(), hoist[2].str()) << text;
		}
		EXPECT_EQ(places, 2uz) << text;
	}

	TEST(LowerTest, EmitsCompoundAssignAndTernary) {
		const auto mod = resolved(
			"@input (mode = forward) f32 bias;\n"
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"fn main() {\n\tlet a = 0.0;\n\tfor (let i = 0u; i < 4u; ++i) { a += bias; if (a > 3.0) { break; } }\n"
			"\tcolor = bias > 0.0 ? a : -a;\n}\n",
			EntryKind::eFragment,
			std::array { String("color") }
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));
		EXPECT_EQ(count(text, "OpLoopMerge"), 1uz) << text;
		EXPECT_TRUE(text.contains("OpFAdd")) << text;
		EXPECT_TRUE(text.contains("OpULessThan")) << text;
		EXPECT_TRUE(text.contains("OpSelect") || text.contains("OpPhi")) << text;
	}

	TEST(LowerTest, ForWithOmittedInitAndStepHasValidHeader) {
		const auto mod = resolved(
			"@input (mode = forward) f32 bias;\n"
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"fn main() {\n\tlet a = 0.0;\n\tlet c = bias > 1.0;\n\tfor (; c; ) { a += bias; break; }\n"
			"\tlet i = 0u;\n\tfor (; i < 4u; ) { a += bias; i++; }\n"
			"\tfor (let j = 0u; j < 4u; ) { a += bias; j++; }\n\tcolor = a;\n}\n",
			EntryKind::eFragment,
			std::array { String("color") }
		);
		const auto text = textOf(lower(mod, layoutOf(mod)));
		EXPECT_EQ(count(text, "OpLoopMerge"), 3uz) << text;
	}

	TEST(LowerTest, SparseAddsExtensions) {
		const auto outputs = std::array { String("color") };
		const auto mod = resolved(
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
		const auto text = textOf(lower(mod, layoutOf(mod)));
		EXPECT_TRUE(text.contains("OpCapability SparseResidency\n")) << text;
		EXPECT_TRUE(text.contains("OpCapability MinLod\n")) << text;
		EXPECT_EQ(count(text, "OpImageSparseSampleImplicitLod"), 2uz) << text;
		EXPECT_EQ(count(text, "OpImageSparseTexelsResident"), 2uz) << text;
	}

	TEST(LowerTest, ViewIndexAddsMultiview) {
		const auto mod = resolved("fn main() {\n\tposition = f32vec4(f32(view_index));\n}\n", EntryKind::eVertex);
		const auto text = textOf(lower(mod, layoutOf(mod)));
		EXPECT_TRUE(text.contains("OpCapability MultiView\n")) << text;
		EXPECT_TRUE(text.contains("BuiltIn ViewIndex\n")) << text;
	}
}
