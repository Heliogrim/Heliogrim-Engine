#include "pch.h"

#include <algorithm>
#include <array>

#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Slice.hpp>

using namespace hg;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] Module parse(StringView source_, EntryKind entry_ = EntryKind::eVertex) {
			auto res = parseModule("M", source_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		/* resolve order 1-3 (no toggles), then slice by the required outputs */
		[[nodiscard]] Module resolved(StringView source_, EntryKind entry_, std::span<const String> outputs_) {
			const auto module = parse(source_, entry_);
			auto res = resolveToggles(module, ToggleSet {});
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? slice(*res, outputs_) : Module {};
		}

		[[nodiscard]] Layout layoutOf(ref<const Module> module_, ref<const LayoutWeights> weights_ = {}, ref<const Vector<Layout>> existing_ = {}) {
			return assignLayout(computeInterface(module_), module_.types, weights_, existing_);
		}

		[[nodiscard]] Opt<SlotAssignment> slotOf(ref<const Layout> layout_, StringView id_) {
			const auto it = std::ranges::find(layout_.slots, id_, &SlotAssignment::id);
			if (it == layout_.slots.end()) {
				return None;
			}
			return *it;
		}

		[[nodiscard]] u32 pushSize(StringView fields_) {
			const auto module = parse(String("@input (mode = bind, id = \"p\", class = push) block p { ") + String(fields_) + " };\n");
			const auto layout = layoutOf(module);
			EXPECT_EQ(layout.push.size(), 1uz);
			return layout.push.empty() ? ~0u : layout.push.front().size;
		}

		[[nodiscard]] TypeRef pushType(ref<const Module> module_) {
			return module_.bindings.empty() ? TypeRef {} : module_.bindings.front().type;
		}

		[[nodiscard]] TypeDesc structOf(Vector<FieldDesc> fields_) {
			TypeDesc desc {};
			desc.kind = TypeKind::eStruct;
			desc.fields = std::move(fields_);
			return desc;
		}

		[[nodiscard]] TypeDesc typeOf(TypeKind kind_, u32 bits_ = 32u, u32 n_ = 1u, u32 count_ = 0u) {
			TypeDesc desc {};
			desc.kind = kind_;
			desc.bits = bits_;
			desc.n = n_;
			desc.count = count_;
			return desc;
		}

		[[nodiscard]] InterfaceEntry entry(StringView id_, TypeRef type_, BindMode mode_, BindClass cls_, bool input_ = true) {
			return InterfaceEntry { String(id_), type_, mode_, cls_, false, input_ };
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

	TEST(InterfaceTest, ShadowVertexInterfaceIsExact) {
		const auto vertex = resolved(shadowVertex, EntryKind::eVertex, {});
		const auto vs = computeInterface(vertex);
		ASSERT_EQ(vs.size(), 4uz);

		const auto expect = [](ref<const InterfaceEntry> e_, StringView id_, BindMode mode_, BindClass cls_, bool implicit_, bool input_) {
			EXPECT_EQ(e_.id, id_);
			EXPECT_EQ(e_.mode, mode_) << id_;
			EXPECT_EQ(e_.cls, cls_) << id_;
			EXPECT_EQ(e_.implicit, implicit_) << id_;
			EXPECT_EQ(e_.input, input_) << id_;
		};
		expect(vs[0], "block", BindMode::eBind, BindClass::ePushConstant, false, true);
		expect(vs[1], "model", BindMode::eBind, BindClass::eStorageBuffer, false, true);
		expect(vs[2], "vertex-position", BindMode::eForward, BindClass::eNone, false, true);
		expect(vs[3], "view", BindMode::eBind, BindClass::eStorageBuffer, false, true);

		const auto outputs = std::array { String("depth") };
		const auto fragment = resolved(shadowFragment, EntryKind::eFragment, outputs);
		const auto fs = computeInterface(fragment);
		ASSERT_EQ(fs.size(), 1uz);
		expect(fs[0], "depth", BindMode::eForward, BindClass::eNone, true, false);

		const auto layout = assignLayout(vs, vertex.types, {});
		EXPECT_EQ(layout.slots.size(), 2uz);
		EXPECT_EQ(layout.inputLocations, (Vector<u32> { 0u }));
		ASSERT_EQ(layout.push.size(), 1uz);
		EXPECT_EQ(layout.push[0].id, "block");
	}

	TEST(InterfaceTest, InputsPrecedeOutputsThenById) {
		const auto module = parse(
			"@output (mode = forward, id = \"a-out\") f32vec4 o;\n"
			"@input (mode = forward, id = \"z\") f32 z;\n"
			"@input (mode = forward, id = \"b\") f32 b;\n"
		);
		const auto entries = computeInterface(module);
		ASSERT_EQ(entries.size(), 3uz);
		EXPECT_EQ(entries[0].id, "b");
		EXPECT_EQ(entries[1].id, "z");
		EXPECT_EQ(entries[2].id, "a-out");
		EXPECT_FALSE(entries[2].input);
	}

	TEST(LayoutTest, IsDeterministicUnderShuffledDeclarationOrder) {
		const std::array<StringView, 5> decls {
			"@input (mode = bind, id = \"camera\", class = storage) block camera { f32mat4 vp; };\n",
			"@input (mode = bind, id = \"albedo\", class = texture) texture2d albedo;\n",
			"@input (mode = bind, id = \"lights\", class = uniform) block lights { f32vec4[4] color; u32 count; };\n",
			"@input (mode = bind, id = \"push\", class = push) block push { f32vec2 translate; f32vec2 scale; };\n",
			"@input (mode = forward, id = \"uv\") f32vec2 uv;\n@input (mode = forward, id = \"normal\") f32vec3 normal;\n"
		};
		const auto build = [&](std::array<u32, 5> order_) {
			String source {};
			for (const auto i : order_) {
				source += decls[i];
			}
			return layoutOf(parse(source));
		};

		const auto a = build({ 0u, 1u, 2u, 3u, 4u });
		const auto b = build({ 4u, 3u, 2u, 1u, 0u });
		const auto c = build({ 2u, 4u, 0u, 3u, 1u });
		EXPECT_EQ(a, b);
		EXPECT_EQ(a, c);
		ASSERT_EQ(a.slots.size(), 3uz);
		EXPECT_EQ(a.inputLocations, (Vector<u32> { 0u, 1u }));
	}

	TEST(LayoutTest, PacksWithoutGaps) {
		const auto layout = layoutOf(
			parse(
				"@input (mode = bind, id = \"a\", class = storage) block a { f32 x; };\n"
				"@input (mode = bind, id = \"b\", class = storage) block b { f32 y; };\n"
				"@input (mode = bind, id = \"c\", class = uniform) block c { f32 z; };\n"
			)
		);
		ASSERT_EQ(layout.slots.size(), 3uz);
		for (auto i = 0u; i < 3u; ++i) {
			EXPECT_EQ(layout.slots[i].set, 0u);
			EXPECT_EQ(layout.slots[i].binding, i);
		}
		// class grouping: uniform before storage, then by id
		EXPECT_EQ(layout.slots[0].id, "c");
		EXPECT_EQ(layout.slots[1].id, "a");
		EXPECT_EQ(layout.slots[2].id, "b");
	}

	TEST(LayoutTest, GroupsBindingsByClassInSetZero) {
		const auto layout = layoutOf(
			parse(
				"@input (mode = bind, id = \"s\", class = sampler) sampler s;\n"
				"@input (mode = bind, id = \"t\", class = texture) texture2d t;\n"
				"@input (mode = bind, id = \"b\", class = storage) block b { f32 x; };\n"
				"@input (mode = bind, id = \"u\", class = uniform) block u { f32 y; };\n"
			)
		);
		ASSERT_EQ(layout.slots.size(), 4uz);
		const std::array<StringView, 4> ids { "u", "b", "t", "s" };
		for (auto i = 0u; i < 4u; ++i) {
			EXPECT_EQ(layout.slots[i].id, ids[i]);
			EXPECT_EQ(layout.slots[i].set, 0u);
			EXPECT_EQ(layout.slots[i].binding, i);
		}
	}

	TEST(LayoutTest, ReusesIdenticalBindingAcrossShadersEvenWithSkip) {
		const auto first = layoutOf(
			parse(
				"@input (mode = bind, id = \"camera\", class = storage) block camera { f32mat4 vp; };\n"
				"@input (mode = bind, id = \"model\", class = storage) block model { f32mat4[] model; };\n"
			)
		);
		ASSERT_EQ(slotOf(first, "model")->binding, 1u);

		const auto second = parse("@input (mode = bind, id = \"model\", class = storage) block model { f32mat4[] model; };\n");
		const auto existing = Vector<Layout> { first };

		// skip is cheap: reuse binding 1, leaving 0 unused
		const auto reused = layoutOf(second, LayoutWeights { 1.0f, 4.0f, 8.0f }, existing);
		ASSERT_TRUE(slotOf(reused, "model").has_value());
		EXPECT_EQ(slotOf(reused, "model")->set, 0u);
		EXPECT_EQ(slotOf(reused, "model")->binding, 1u);

		// extra layout is cheap: a new packed binding
		const auto fresh = layoutOf(second, LayoutWeights { 8.0f, 4.0f, 8.0f }, existing);
		EXPECT_EQ(slotOf(fresh, "model")->binding, 0u);

		// a different type is never identical
		const auto other = parse("@input (mode = bind, id = \"model\", class = storage) block model { f32vec4[] model; };\n");
		EXPECT_EQ(slotOf(layoutOf(other, {}, existing), "model")->binding, 0u);

		// a different class is never identical
		const auto uniform = parse("@input (mode = bind, id = \"camera\", class = uniform) block camera { f32mat4 vp; };\n");
		EXPECT_EQ(slotOf(layoutOf(uniform, {}, existing), "camera")->binding, 0u);
	}

	TEST(LayoutTest, OpensNewSetWhenCheaperThanAccumulatedSkips) {
		const auto first = layoutOf(
			parse(
				"@input (mode = bind, id = \"a\", class = storage) block a { f32 a; };\n"
				"@input (mode = bind, id = \"b\", class = storage) block b { u32 b; };\n"
				"@input (mode = bind, id = \"c\", class = storage) block c { s32 c; };\n"
				"@input (mode = bind, id = \"d\", class = storage) block d { f32vec2 d; };\n"
			)
		);
		ASSERT_EQ(slotOf(first, "d")->binding, 3u);
		const auto second = parse(
			"@input (mode = bind, id = \"d\", class = storage) block d { f32vec2 d; };\n"
			"@input (mode = bind, id = \"e\", class = storage) block e { f32vec3 e; };\n"
		);
		const auto existing = Vector<Layout> { first };

		// skipping 3 slots (cost 3) is cheaper than a new layout (4), and a new set (2) is cheaper than the skips
		const auto split = layoutOf(second, LayoutWeights { 1.0f, 4.0f, 2.0f }, existing);
		EXPECT_EQ(slotOf(split, "d")->set, 0u);
		EXPECT_EQ(slotOf(split, "d")->binding, 3u);
		EXPECT_EQ(slotOf(split, "e")->set, 1u);
		EXPECT_EQ(slotOf(split, "e")->binding, 0u);

		// default: a new set costs more than the skips, `e` follows in set 0
		const auto packed = layoutOf(second, {}, existing);
		EXPECT_EQ(slotOf(packed, "e")->set, 0u);
		EXPECT_EQ(slotOf(packed, "e")->binding, 4u);
	}

	TEST(LayoutTest, EmptyInterface) {
		const auto layout = assignLayout({}, {}, {});
		EXPECT_TRUE(layout.slots.empty());
		EXPECT_TRUE(layout.push.empty());
		EXPECT_TRUE(layout.inputLocations.empty());
	}

	TEST(LayoutTest, DuplicateIdsCollapseDeterministically) {
		const auto module = parse(
			"@input (mode = bind, id = \"x\", class = storage) block x { f32 x; };\n"
			"@input (mode = bind, id = \"y\", class = uniform) block y { f32 y; };\n"
		);
		auto entries = computeInterface(module);
		ASSERT_EQ(entries.size(), 2uz);
		entries.emplace_back(entry("x", entries[1].type, BindMode::eBind, BindClass::eUniformBuffer));
		entries.emplace_back(entry("x", entries[0].type, BindMode::eBind, BindClass::eStorageBuffer));

		const auto forward = assignLayout(entries, module.types, {});
		std::ranges::reverse(entries);
		const auto backward = assignLayout(entries, module.types, {});
		EXPECT_EQ(forward, backward);
		ASSERT_EQ(forward.slots.size(), 2uz);
		// the first in (id, class) order wins: uniform `x` with the type of `y`
		EXPECT_EQ(slotOf(forward, "x")->identity, slotOf(forward, "y")->identity);
	}

	TEST(LayoutTest, SkipsBindEntriesWithoutClass) {
		const auto module = parse("@input (mode = bind, id = \"x\") block x { f32 x; };\n");
		const auto entries = computeInterface(module);
		ASSERT_EQ(entries.size(), 1uz);
		EXPECT_EQ(entries[0].cls, BindClass::eNone);
		const auto layout = assignLayout(entries, module.types, {});
		EXPECT_TRUE(layout.slots.empty());
		EXPECT_TRUE(layout.push.empty());
	}

	TEST(LayoutTest, PushRangeSizeFromType) {
		const auto shadow = layoutOf(resolved(shadowVertex, EntryKind::eVertex, {}));
		ASSERT_EQ(shadow.push.size(), 1uz);
		EXPECT_EQ(shadow.push[0].offset, 0u);
		EXPECT_EQ(shadow.push[0].size, 4u);

		const auto ui = parse("@input (mode = bind, id = \"meta\", class = push) block meta { f32vec2 translate; f32vec2 scale; };\n");
		const auto layout = layoutOf(ui);
		ASSERT_EQ(layout.push.size(), 1uz);
		EXPECT_EQ(layout.push[0].offset, 0u);
		EXPECT_EQ(layout.push[0].size, 16u);
		EXPECT_EQ(std430Offsets(ui.types, pushType(ui)), (Vector<u32> { 0u, 8u }));
	}

	TEST(LayoutTest, PushRangesPackInIdOrder) {
		const auto layout = layoutOf(
			parse(
				"@input (mode = bind, id = \"b\", class = push) block b { f32vec2 v; };\n"
				"@input (mode = bind, id = \"a\", class = push) block a { u32 x; };\n"
			)
		);
		ASSERT_EQ(layout.push.size(), 2uz);
		EXPECT_EQ(layout.push[0], (PushRange { "a", 0u, 4u }));
		EXPECT_EQ(layout.push[1], (PushRange { "b", 8u, 8u }));
	}

	TEST(LayoutTest, Std430SizesAndAlignment) {
		EXPECT_EQ(pushSize("f32vec3 a; f32 b;"), 16u);
		EXPECT_EQ(pushSize("f32 a; f32vec3 b;"), 32u);
		EXPECT_EQ(pushSize("f32mat4 m;"), 64u);
		EXPECT_EQ(pushSize("f32[3] a;"), 12u);
		EXPECT_EQ(pushSize("f32vec3[2] a;"), 32u);
		EXPECT_EQ(pushSize("bool a; u32 b;"), 8u);
		EXPECT_EQ(pushSize("f32vec4[4] a; u32 b;"), 80u);
		// u8 is 1 byte; the range rounds up to Vulkan's 4-byte granularity
		EXPECT_EQ(pushSize("u8 a;"), 4u);
		EXPECT_EQ(pushSize("u8 a; u8vec4 b;"), 8u);

		const auto nested = parse(
			"struct Inner { f32vec2 v; };\n"
			"@input (mode = bind, id = \"p\", class = push) block p { f32 a; Inner b; };\n"
		);
		EXPECT_EQ(std430Offsets(nested.types, pushType(nested)), (Vector<u32> { 0u, 8u }));
		ASSERT_EQ(layoutOf(nested).push.size(), 1uz);
		EXPECT_EQ(layoutOf(nested).push[0].size, 16u);
	}

	TEST(LayoutTest, UnsizedArrayOnlyAsLastStorageMember) {
		const auto storage = parse("@input (mode = bind, id = \"s\", class = storage) block s { u32 count; f32vec4[] items; };\n");
		EXPECT_EQ(std430Offsets(storage.types, pushType(storage)), (Vector<u32> { 0u, 16u }));
		EXPECT_EQ(layoutOf(storage).slots.size(), 1uz);

		const auto notLast = parse("@input (mode = bind, id = \"s\", class = storage) block s { f32vec4[] items; u32 count; };\n");
		EXPECT_TRUE(std430Offsets(notLast.types, pushType(notLast)).empty());

		// a push range has a fixed size: unsized is a safe 0
		EXPECT_EQ(pushSize("u32 count; f32[] items;"), 0u);
	}

	TEST(LayoutTest, ForwardInputLocations) {
		const auto layout = layoutOf(
			parse(
				"@input (mode = forward, id = \"a\") f32vec3 a;\n"
				"@input (mode = forward, id = \"b\") f32mat4 b;\n"
				"@input (mode = forward, id = \"c\") f32[3] c;\n"
				"@input (mode = forward, id = \"d\") f32 d;\n"
				"@output (mode = forward, id = \"e\") f32vec4 e;\n"
			)
		);
		EXPECT_EQ(layout.inputLocations, (Vector<u32> { 0u, 1u, 5u, 8u }));
		EXPECT_TRUE(layout.slots.empty());
	}

	TEST(LayoutTest, HostileTypeTableIsSafe) {
		Vector<TypeDesc> types {};
		// 0: struct referencing itself
		types.emplace_back(structOf({ FieldDesc { "self", TypeRef { 0u } } }));
		// 1: struct with an out-of-range field
		types.emplace_back(structOf({ FieldDesc { "bad", TypeRef { 99u } } }));
		// 2: array of an unset element
		types.emplace_back(typeOf(TypeKind::eArray, 32u, 1u, 2u));
		// 3: vector with a bogus width
		types.emplace_back(typeOf(TypeKind::eVector, 7u, 9u));
		// 4: mutual cycle 4 -> 5 -> 4
		types.emplace_back(structOf({ FieldDesc { "x", TypeRef { 5u } } }));
		types.emplace_back(structOf({ FieldDesc { "y", TypeRef { 4u } } }));

		Vector<InterfaceEntry> entries {};
		for (auto i = 0u; i < 6u; ++i) {
			entries.emplace_back(entry(String("p") + std::to_string(i), TypeRef { i }, BindMode::eBind, BindClass::ePushConstant));
			entries.emplace_back(entry(String("s") + std::to_string(i), TypeRef { i }, BindMode::eBind, BindClass::eStorageBuffer));
			EXPECT_TRUE(std430Offsets(types, TypeRef { i }).empty()) << i;
		}
		entries.emplace_back(entry("unset", TypeRef {}, BindMode::eBind, BindClass::ePushConstant));
		entries.emplace_back(entry("far", TypeRef { 1000u }, BindMode::eForward, BindClass::eNone));

		const auto layout = assignLayout(entries, types, {});
		ASSERT_EQ(layout.push.size(), 7uz);
		for (const auto& range : layout.push) {
			EXPECT_EQ(range.size, 0u) << range.id;
		}
		EXPECT_EQ(layout.slots.size(), 6uz);
		EXPECT_EQ(layout.inputLocations, (Vector<u32> { 0u }));

		// bad slots never match each other across shaders, even when skipping is free
		ASSERT_EQ(slotOf(layout, "s5")->binding, 5u);
		const auto again = assignLayout(
			Vector<InterfaceEntry> { entry("s5", TypeRef { 5u }, BindMode::eBind, BindClass::eStorageBuffer) },
			types,
			LayoutWeights { 0.0f, 4.0f, 8.0f },
			Vector<Layout> { layout }
		);
		EXPECT_EQ(slotOf(again, "s5")->binding, 0u);
	}

	TEST(LayoutTest, DeepAndExplosiveTypesTerminate) {
		// 1000 nested structs exceed the depth limit
		Vector<TypeDesc> deep {};
		deep.emplace_back(typeOf(TypeKind::eScalar));
		for (auto i = 1u; i < 1000u; ++i) {
			deep.emplace_back(structOf({ FieldDesc { "x", TypeRef { i - 1u } } }));
		}
		EXPECT_TRUE(std430Offsets(deep, TypeRef { 999u }).empty());
		EXPECT_EQ(std430Offsets(deep, TypeRef { 100u }), (Vector<u32> { 0u }));

		// 64 levels of two fields each: memoized, and its 2^66-byte size overflows to a safe 0
		Vector<TypeDesc> wide {};
		wide.emplace_back(typeOf(TypeKind::eScalar));
		for (auto i = 1u; i <= 64u; ++i) {
			wide.emplace_back(structOf({ FieldDesc { "l", TypeRef { i - 1u } }, FieldDesc { "r", TypeRef { i - 1u } } }));
		}
		const auto layout = assignLayout(
			Vector<InterfaceEntry> { entry("p", TypeRef { 64u }, BindMode::eBind, BindClass::ePushConstant) },
			wide,
			{}
		);
		ASSERT_EQ(layout.push.size(), 1uz);
		EXPECT_EQ(layout.push[0].size, 0u);
	}
}
