#include "pch.h"

/**/

#include <Engine.Accel.Lang/Diagnostic/Codes.hpp>
#include <Engine.Accel.Lang/Ir/Contract.hpp>
#include <Engine.Accel.Lang/Ir/Interface.hpp>
#include <Engine.Accel.Lang/Ir/Layout.hpp>
#include <Engine.Accel.Lang/Ir/Lexer.hpp>
#include <Engine.Accel.Lang/Ir/Lower.hpp>
#include <Engine.Accel.Lang/Ir/Merge.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/References.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Tier.hpp>
#include <Engine.Accel.Lang/Ir/Spirv/SpirvEmitter.hpp>

#include "IrSpan.hpp"
#include "IrSpirvText.hpp"
#include "IrWording.hpp"

import Engine.Diagnostics;
import std;

using namespace hg::engine::accel::lang::ir;
using namespace hg::engine::accel::lang;
using namespace hg;
namespace diag = hg::engine::diag;

namespace AccelLang {
	namespace {
		using Diagnostics = diag::Diagnostics;

		[[nodiscard]] Opt<String> noIncludes(StringView) { return None; }

		[[nodiscard]] Opt<String> twoIncludes(StringView name_) {
			if (name_ == "a") { return String { "#include <b>\nfn A() -> f32;" }; }
			if (name_ == "b") { return String { "#include <a>\nfn B() -> f32;" }; }
			return None;
		}

		/* the errors of a failed parse, else the warnings of the parsed module */
		[[nodiscard]] Diagnostics diagsOf(std::expected<Module, Diagnostics>&& res_) {
			return res_.has_value() ? res_->warnings : std::move(res_).error();
		}

		template <EntryKind K>
		[[nodiscard]] Diagnostics parseD(StringView src_) {
			return diagsOf(parseModule("M", src_, K, ModuleResolver { noIncludes }));
		}

		[[nodiscard]] Diagnostics lexD(StringView src_) {
			auto res = lex(src_);
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}

		[[nodiscard]] Diagnostics includeD(StringView src_) {
			return diagsOf(parseModule("M", src_, EntryKind::eFragment, ModuleResolver { twoIncludes }));
		}

		[[nodiscard]] Module mk(StringView src_, EntryKind entry_ = EntryKind::eFragment) {
			auto parsed = parseModule("M", src_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(parsed.has_value());
			if (not parsed.has_value()) { return {}; }

			auto res = resolveToggles(*parsed, ToggleSet {});
			EXPECT_TRUE(res.has_value());
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] Layout layoutOf(ref<const Module> mod_) {
			return assignLayout(computeInterface(mod_), mod_.types, {});
		}

		constexpr StringView shading = "toggle Shading : { Forward, Deferred } = Forward;\n";
		constexpr StringView shadows = "toggle Shadows : { Off, On } = Off;\n";

		[[nodiscard]] Diagnostics resolveD(StringView src_, ToggleSet set_, EntryKind entry_ = EntryKind::eFragment) {
			auto parsed = parseModule("M", src_, entry_, ModuleResolver { noIncludes });
			if (not parsed.has_value()) { return std::move(parsed).error(); }

			auto res = resolveToggles(*parsed, set_);
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}

		[[nodiscard]] Diagnostics fixD(StringView src_, ToggleSources sources_) {
			auto parsed = parseModule("M", src_, EntryKind::eFragment, ModuleResolver { noIncludes });
			if (not parsed.has_value()) { return std::move(parsed).error(); }

			auto res = fixToggles(*parsed, sources_);
			return res.has_value() ? res->warnings : std::move(res).error();
		}

		[[nodiscard]] Diagnostics mergeD(StringView a_, StringView b_) {
			auto a = parseModule("A", a_, EntryKind::eFragment, ModuleResolver { noIncludes });
			auto b = parseModule("B", b_, EntryKind::eFragment, ModuleResolver { noIncludes });
			if (not a.has_value() || not b.has_value()) { return {}; }

			const Vector<const Module*> mods { &*a, &*b };
			auto res = merge(mods);
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}

		[[nodiscard]] Diagnostics regionsD(StringView src_) {
			auto parsed = parseModule("M", src_, EntryKind::eCompute, ModuleResolver { noIncludes });
			if (not parsed.has_value()) { return std::move(parsed).error(); }

			auto res = lowerReferences(*parsed);
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}

		[[nodiscard]] Diagnostics emitD(ref<const Module> mod_, ref<const Layout> layout_) {
			return spirv::emit(mod_, layout_, Tier::eDev).diagnostics;
		}

		[[nodiscard]] InterfaceEntry ie(StringView id_, TypeRef type_, BindMode mode_, BindClass cls_, bool input_ = true) {
			return InterfaceEntry { String(id_), type_, mode_, cls_, false, input_ };
		}

		/* a pass-side declaration outside the module sources: rendered without a snippet */
		[[nodiscard]] diag::Location passAt() {
			return SourceSpan { .file = "pass", .begin = 0uL, .end = 0uL, .line = 3uL, .column = 5uL };
		}

		[[nodiscard]] InterfaceEntry fwd(StringView id_, StringView type_, bool input_) {
			return InterfaceEntry { String(id_), TypeRef { 0 }, BindMode::eForward, BindClass::eNone, false, input_, false, String(type_) };
		}

		[[nodiscard]] Vector<TypeDesc> cameraTypes(bool padded_) {
			Vector<TypeDesc> types {};
			if (padded_) { types.emplace_back(TypeDesc { .kind = TypeKind::eVector, .n = 3uL }); }
			types.emplace_back(TypeDesc {});
			types.emplace_back(
				TypeDesc { .kind = TypeKind::eStruct, .name = "Camera", .fields = { FieldDesc { "x", TypeRef { padded_ ? 1u : 0u } } } }
			);
			return types;
		}

		constexpr StringView storageBlock = "@input (mode = bind, id = \"b\", class = storage) block b { f32 x; };\n";
		constexpr StringView regionPrelude =
			"@input (mode = bind, id = \"buf\", class = storage, access = read_write) block buf { u32[] data; };\n"
			"workgroup scratch: u32[256];\n"
			"fn w(v: &mut u32[], i: u32) { v[i] = 1u; }\n";
		constexpr StringView uniformityHead =
			"@input (mode = bind, id = \"p\", class = push) block p { f32 x; };\n"
			"@input (mode = forward, id = \"in_x\") f32 in_x;\n"
			"fn a() {}\nfn b() {}\nfn c() {}\n";

		const std::string deepSource = "@when (" + std::string(257uz, '(') + "A == B" + std::string(257uz, ')') + ") fn F();";
		const std::string regionNotRooted = std::string(regionPrelude) + "fn main() {\nlet tmp: u32[4]; w(tmp[0u .. 4u], 0u);\n}\n";
		const std::string regionShadowed = std::string(regionPrelude) +
			"fn h(v: &mut u32[]) { let scratch: u32 = 2u; v[0u] = scratch; }\nfn main() { h(scratch[0u .. 8u]); }\n";
		const std::string emptyRange = std::string(regionPrelude) + "fn main() {\nw(scratch[4u .. 4u], 0u);\n}\n";
		const std::string atomicLocal = std::string(regionPrelude) + "fn main() {\n\tlet x: u32 = 0u;\n\tlet o = atomic_add(x, 1u);\n}\n";
		const std::string missingVariant = std::string(shading) + std::string(shadows) +
			"fn GetWorldPosition() -> f32vec4;\n"
			"@when (Shading == Forward) fn GetWorldPosition() -> f32vec4 { return f32vec4(1.0); }\n"
			"@when (Shading == Deferred && Shadows == On) fn GetWorldPosition() -> f32vec4 { return f32vec4(0.5); }\n"
			"fn main() {\n\tposition = GetWorldPosition();\n}\n";
		const std::string overlapping = std::string(shading) + std::string(shadows) +
			"@when (Shading == Forward) fn F() -> f32 { return 1.0; }\n"
			"@when (Shadows == On) fn F() -> f32 { return 0.5; }\n"
			"@when (Shading == Deferred && Shadows == Off) fn F() -> f32 { return 0.25; }\n"
			"fn main() {\n\tfrag_depth = F();\n}\n";
		const std::string divergent = std::string(uniformityHead) +
			"fn main() {\n\twhile (p.x > 0.0) {\n\t\tif (in_x > 0.0) { break; }\n\t\ta();\n\t}\n\tb();\n}\n";
		const std::string shadowVertex =
			"@input (mode = bind, id = \"model\", class = storage) block model { f32mat4[] model; };\n"
			"@input (mode = forward, id = \"vertex-position\") f32vec3 local_position;\n"
			"fn main() {\n\tlet pos = model.model[instance_index] * f32vec4(local_position, 1.0);\n\tposition = pos;\n}\n";

		struct Golden {
			StringView key;
			StringView src;

			Diagnostics (*produce)(StringView);

			StringView rendered;
		};

		const Golden table[] = {
			{
				"unexpected-character", "a\n b $ c ` d", parseD<EntryKind::eFragment>, R"GOLD(error[ASL0001]: unexpected character '$'
  --> M:2:4
  |
2 |  b $ c ` d
  |    ^
)GOLD"
			},
			{
				"unterminated-token", "a\n  /* never closed", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0002]: unterminated block comment
  --> M:2:3
  |
2 |   /* never closed
  |   ^
)GOLD"
			},
			{
				"malformed-number", "1.2.3 12ab 1e", parseD<EntryKind::eFragment>, R"GOLD(error[ASL0003]: malformed number '1.2.3'
  --> M:1:1
  |
1 | 1.2.3 12ab 1e
  | ^
)GOLD"
			},
			{
				"literal-out-of-range", "fn main() {\n\tlet a = 99999999999999999999;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0068]: literal '99999999999999999999' is malformed or does not fit 64 bits
  --> M:2:10
  |
2 | 	let a = 99999999999999999999;
  |          ^^^^^^^^^^^^^^^^^^^^
)GOLD"
			},
			{
				"unknown-directive", "#define x", parseD<EntryKind::eFragment>, R"GOLD(error[ASL0004]: unknown directive '#define'
  --> M:1:1
  |
1 | #define x
  | ^
)GOLD"
			},
			{
				"malformed-include", "#include foo", parseD<EntryKind::eFragment>, R"GOLD(error[ASL0005]: expected '<' after '#include'
  --> M:1:10
  |
1 | #include foo
  |          ^
)GOLD"
			},
			{
				"misplaced-include", "fn F() -> f32;\n#include <a>\n", includeD,
				R"GOLD(error[ASL0006]: '#include' must precede all declarations of a module
  --> M:2:1
  |
2 | #include <a>
  | ^^^^^^^^
)GOLD"
			},
			{
				"include-cycle", "#include <a>\n", includeD, R"GOLD(error[ASL0007]: include cycle through 'M', 'a', 'b', 'a'
  --> b:1:11
  |
1 | #include <a>
  |           ^
)GOLD"
			},
			{
				"unknown-include", "\n#include <missing>\nfn F() -> f32;", includeD, R"GOLD(error[ASL0008]: unknown include 'missing'
  --> M:2:11
  |
2 | #include <missing>
  |           ^^^^^^^
)GOLD"
			},
			{
				"unexpected-token", "toggle A : { A, B } = A;\n@when (some(A == B)) fn F();", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0009]: unknown condition 'some', expected 'any', 'all' or 'none'
  --> M:2:8
  |
2 | @when (some(A == B)) fn F();
  |        ^^^^
)GOLD"
			},
			{
				"nesting-too-deep", deepSource, parseD<EntryKind::eFragment>, R"GOLD(error[ASL0010]: nesting too deep (limit 256)
  --> M:1:264
  |
1 | @when ((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((A == B)))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))) fn F();
  |                                                                                                                                                                                                                                                                        ^
)GOLD"
			},
			{
				"reserved-name", "fn main() {\n\tlet bitcast = 1.0;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0011]: 'bitcast' is an ASL builtin
  --> M:2:6
  |
2 | 	let bitcast = 1.0;
  |      ^^^^^^^
)GOLD"
			},
			{
				"unknown-annotation", "@input (mode = forward, colour = \"x\") f32 a;", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0012]: unknown annotation key 'colour' for @input
  --> M:1:25
  |
1 | @input (mode = forward, colour = "x") f32 a;
  |                         ^^^^^^
)GOLD"
			},
			{
				"misplaced-annotation", "@compute (local_size = [8, 4, 2]) fn F() {}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0013]: @compute contradicts the module's stage
  --> M:1:2
  |
1 | @compute (local_size = [8, 4, 2]) fn F() {}
  |  ^^^^^^^
)GOLD"
			},
			{
				"duplicate-annotation", "@input @input (mode = forward, id = \"a\") f32 a;\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0014]: duplicate '@input' annotation
  --> M:1:9
  |
1 | @input @input (mode = forward, id = "a") f32 a;
  |         ^^^^^
)GOLD"
			},
			{
				"invalid-annotation-value",
				"@input (mode = bind, id = \"buf\", class = storage, access = rw) block buf { u32[] data; };\nfn main() {}\n",
				parseD<EntryKind::eFragment>, R"GOLD(error[ASL0015]: invalid value 'rw' for annotation key 'access'
  --> M:1:60
  |
1 | @input (mode = bind, id = "buf", class = storage, access = rw) block buf { u32[] data; };
  |                                                            ^^
)GOLD"
			},
			{
				"missing-annotation-key", "@geometry fn main() {}\n", parseD<EntryKind::eGeometry>,
				R"GOLD(error[ASL0016]: @geometry requires 'input', 'output' and 'max_vertices'
  --> M:1:2
  |
1 | @geometry fn main() {}
  |  ^^^^^^^^
)GOLD"
			},
			{
				"invalid-toggle-declaration", "toggle Empty : { };\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0017]: toggle 'Empty' declares no values
  --> M:1:8
  |
1 | toggle Empty : { };
  |        ^^^^^
)GOLD"
			},
			{
				"unknown-toggle", "toggle Shading : { Forward } = Forward;\n@when (Shading == Forward && Nope == X)\nfn F() -> f32;",
				parseD<EntryKind::eFragment>, R"GOLD(error[ASL0018]: unknown toggle 'Nope'
  --> M:2:30
  |
2 | @when (Shading == Forward && Nope == X)
  |                              ^^^^
)GOLD"
			},
			{
				"duplicate-toggle-setting", shading,
				[](StringView s_) {
					return fixD(
						s_,
						ToggleSources { .pass = { { "Shading", "Forward" }, { "Shading", "Forward" } }, .profile = {}, .material = {} }
					);
				},
				R"GOLD(error[ASL0019]: toggle 'Shading' is set twice by the pass
)GOLD"
			},
			{
				"toggle-override", shading,
				[](StringView s_) {
					return fixD(
						s_,
						ToggleSources { .pass = { { "Shading", "Deferred" } }, .profile = {}, .material = { { "Shading", "Forward" } } }
					);
				},
				R"GOLD(warning[ASL0020]: the material sets toggle 'Shading' to 'Forward', but the pass fixes it to 'Deferred'
)GOLD"
			},
			{
				"missing-variant-body", missingVariant,
				[](StringView s_) { return resolveD(s_, { { "Shading", "Forward" }, { "Shadows", "Off" } }, EntryKind::eVertex); },
				R"GOLD(error[ASL0021]: no body for 'GetWorldPosition()' at Shading=Deferred, Shadows=Off
)GOLD"
			},
			{
				"overlapping-variant-bodies", overlapping,
				[](StringView s_) { return resolveD(s_, { { "Shading", "Forward" }, { "Shadows", "On" } }); },
				R"GOLD(error[ASL0022]: overlapping bodies for 'F()' at Shading=Forward, Shadows=On
)GOLD"
			},
			{
				"duplicate-declaration", "toggle A : { X, Y } = X;\ntoggle A : { X, Y } = X;\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0023]: duplicate toggle 'A'
  --> M:2:8
  |
2 | toggle A : { X, Y } = X;
  |        ^
)GOLD"
			},
			{
				"incomplete-local-declaration", "fn main() {\n\tconst k: f32;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0024]: a 'const' local needs an initializer
  --> M:2:14
  |
2 | 	const k: f32;
  |              ^
)GOLD"
			},
			{
				"typed-reference-declaration", "fn main() {\n\tlet a = 1.0;\n\tlet l: f32 = &mut a;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0025]: a reference declaration cannot have a type annotation
  --> M:3:6
  |
3 | 	let l: f32 = &mut a;
  |      ^
)GOLD"
			},
			{
				"misplaced-qualifier", "@input (mode = bind, id = \"smp\", class = sampler, packing = std140) sampler smp;\nfn main() {}\n",
				parseD<EntryKind::eFragment>, R"GOLD(error[ASL0026]: 'packing' is not allowed on opaque binding 'smp'
  --> M:1:77
  |
1 | @input (mode = bind, id = "smp", class = sampler, packing = std140) sampler smp;
  |                                                                             ^^^
)GOLD"
			},
			{
				"recursive-struct",
				"struct S { f32 a; }\n@input (mode = bind, id = \"b\", class = storage) block b { S s; };\nfn main() {}\n", [
				](StringView s_) {
					auto mod = mk(s_);
					const auto layout = layoutOf(mod);
					for (auto i = 0uz; i < mod.types.size(); ++i) {
						if (mod.types[i].kind == TypeKind::eStruct) {
							mod.types[i].fields.emplace_back(FieldDesc { "self", TypeRef { static_cast<u32>(i) } });
						}
					}
					return emitD(mod, layout);
				},
				R"GOLD(error[ASL0027]: struct 'S' contains itself
  --> M:2:55
  |
2 | @input (mode = bind, id = "b", class = storage) block b { S s; };
  |                                                       ^
)GOLD"
			},
			{
				"invalid-entry-signature", "fn main() {}\n", [](StringView s_) {
					auto mod = mk(s_);
					const auto layout = layoutOf(mod);
					mod.types.emplace_back(TypeDesc {});
					for (auto& fn : mod.functions) {
						if (fn.name == "main") { fn.ret = TypeRef { static_cast<u32>(mod.types.size() - 1uz) }; }
					}
					return emitD(mod, layout);
				},
				R"GOLD(error[ASL0028]: 'main' takes no parameters and returns nothing
  --> M:1:4
  |
1 | fn main() {}
  |    ^^^^
)GOLD"
			},
			{
				"unknown-type", "fn main() {\n\tlet x: Foo = 1.0;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0029]: unknown type 'Foo'
  --> M:2:9
  |
2 | 	let x: Foo = 1.0;
  |         ^^^
)GOLD"
			},
			{
				"unknown-name", "fn F() -> f32 {\n\treturn missing;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0030]: unknown name 'missing'
  --> M:2:9
  |
2 | 	return missing;
  |         ^^^^^^^
)GOLD"
			},
			{
				"unknown-function", "fn main() {\n\tlet x = nope(1.0);\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0031]: unknown function 'nope'
  --> M:2:10
  |
2 | 	let x = nope(1.0);
  |          ^^^^
)GOLD"
			},
			{
				"invalid-array-size", "fn main() {\n\tlet a = f32[0](1.0);\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0032]: invalid array size '0'
  --> M:2:13
  |
2 | 	let a = f32[0](1.0);
  |             ^
)GOLD"
			},
			{
				"type-mismatch", "fn F(a: bool, b: f32) -> f32 {\n\treturn a * b;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0033]: operands of '*' have types 'bool' and 'f32'
  --> M:2:11
  |
2 | 	return a * b;
  |           ^
)GOLD"
			},
			{
				"implicit-conversion", "fn F(a: s32, b: f32) -> f32 {\n\treturn a * b;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0069]: operands of '*' have types 's32' and 'f32'
  --> M:2:11
  |
2 | 	return a * b;
  |           ^
  = help: write `f32(a)`
)GOLD"
			},
			{
				"bitcast-size", "fn F(a: f32) -> u64 {\n\treturn bitcast<u64>(a);\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0070]: cannot bitcast 'f32' (32 bits) to 'u64' (64 bits)
  --> M:2:9
  |
2 | 	return bitcast<u64>(a);
  |         ^^^^^^^
)GOLD"
			},
			{
				"wrong-argument-count", "fn main() {\n\tlet n = len();\n}\n", parseD<EntryKind::eCompute>,
				R"GOLD(error[ASL0034]: 'len' takes one region argument
  --> M:2:10
  |
2 | 	let n = len();
  |          ^^^
)GOLD"
			},
			{
				"no-matching-overload",
				"fn saturate(x: f32) -> f32 { return x; }\nfn saturate(x: f32vec3) -> f32vec3 { return x; }\nfn main() {\n\tlet i: s32 = 1;\n\tlet r = saturate(i);\n}\n",
				parseD<EntryKind::eFragment>, R"GOLD(error[ASL0035]: no overload of 'saturate' takes ('s32')
  --> M:5:10
  |
5 | 	let r = saturate(i);
  |          ^^^^^^^^
  = note: candidates are 'saturate(f32)', 'saturate(f32vec3)'
)GOLD"
			},
			{
				"ambiguous-call", "fn F(a: f32, b: s32) {}\nfn F(a: s32, b: f32) {}\nfn main() {\n\tF(1, 1);\n}\n",
				parseD<EntryKind::eFragment>, R"GOLD(error[ASL0036]: call to 'F' is ambiguous for ('s32', 's32')
  --> M:4:2
  |
4 | 	F(1, 1);
  |  ^
)GOLD"
			},
			{
				"empty-range", emptyRange, parseD<EntryKind::eCompute>, R"GOLD(error[ASL0037]: empty or reversed range
  --> M:5:3
  |
5 | w(scratch[4u .. 4u], 0u);
  |   ^^^^^^^
)GOLD"
			},
			{
				"not-assignable", "fn main() {\n\tconst k = 1.0;\n\tk = 2.0;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0038]: cannot assign to constant 'k'
  --> M:3:2
  |
3 | 	k = 2.0;
  |  ^
)GOLD"
			},
			{
				"not-borrowable", "fn main() {\n\tlet l = &mut 1.0;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0039]: '&' needs a variable, a field or an element
  --> M:2:15
  |
2 | 	let l = &mut 1.0;
  |               ^^^
)GOLD"
			},
			{
				"not-storage-resource", atomicLocal, parseD<EntryKind::eCompute>,
				R"GOLD(error[ASL0040]: 'atomic_add' needs a storage buffer or workgroup variable, got local 'x'
  --> M:6:10
  |
6 | 	let o = atomic_add(x, 1u);
  |          ^^^^^^^^^^
)GOLD"
			},
			{
				"borrow-conflict", "fn F(a: &mut f32, b: &mut f32) {}\nfn main() {\n\tlet x = 1.0;\n\tF(x, x);\n}\n",
				parseD<EntryKind::eFragment>, R"GOLD(error[ASL0041]: '&mut' argument 'x' overlaps argument 'x' of call to 'F'
  --> M:4:4
  |
4 | 	F(x, x);
  |    ^
  --> M:4:7: overlaps the '&mut' argument
  |
4 | 	F(x, x);
  |       ^
)GOLD"
			},
			{
				"not-constant", "@input (mode = forward, id = \"x\") f32 x;\nconst C: f32 = x;\nfn main() {}\n",
				parseD<EntryKind::eFragment>, R"GOLD(error[ASL0042]: initializer of constant 'C' is not a constant expression
  --> M:2:7
  |
2 | const C: f32 = x;
  |       ^
)GOLD"
			},
			{
				"loop-control-outside-loop", "fn main() {\n\tbreak;\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0043]: 'break' outside a loop
  --> M:2:7
  |
2 | 	break;
  |       ^
)GOLD"
			},
			{
				"constant-loop-condition", "fn main() {\n\twhile (true) {\n\t}\n}\n", parseD<EntryKind::eFragment>,
				R"GOLD(error[ASL0044]: loop condition is always true
  --> M:2:9
  |
2 | 	while (true) {
  |         ^^^^
  = note: invocations that never reach 'break' keep the others waiting in the loop
  = note: use a condition that bounds the loop
)GOLD"
			},
			{
				"divergent-control", "fn main() {\n\tif (local_invocation_id.x > 3u) {\n\t\tbarrier();\n\t}\n}\n",
				parseD<EntryKind::eCompute>,
				R"GOLD(error[ASL0045]: 'barrier' needs every invocation of a group together but runs under a condition that can differ between invocations
  --> M:3:3
  |
3 | 		barrier();
  |   ^^^^^^^
  --> M:2:6: this condition can differ between invocations
  |
2 | 	if (local_invocation_id.x > 3u) {
  |      ^^^^^^^^^^^^^^^^^^^^^^^^^^
  = note: invocations that skip it leave the others in the group waiting forever
)GOLD"
			},
			{
				"stage-unavailable", "fn main() {\n\tdiscard;\n}\n", parseD<EntryKind::eVertex>,
				R"GOLD(error[ASL0046]: 'discard' is only valid in fragment stages
  --> M:2:9
  |
2 | 	discard;
  |         ^
)GOLD"
			},
			{
				"region-not-rooted", regionNotRooted, regionsD,
				R"GOLD(error[ASL0047]: region argument to 'w' must be rooted in a buffer or workgroup array
  --> M:5:18
  |
5 | let tmp: u32[4]; w(tmp[0u .. 4u], 0u);
  |                  ^
)GOLD"
			},
			{
				"region-root-shadowed", regionShadowed, regionsD, R"GOLD(error[ASL0048]: 'scratch' shadows the root of live region 'v'
  --> M:4:42
  |
4 | fn h(v: &mut u32[]) { let scratch: u32 = 2u; v[0u] = scratch; }
  |                                          ^^
)GOLD"
			},
			{
				"merge-conflict", "const K: f32 = 1.0;\n", [](StringView s_) { return mergeD(s_, "const K: f32 = 2.0;\n"); },
				R"GOLD(error[ASL0049]: constant 'K' declared with a different value in 'A' and in 'B'
)GOLD"
			},
			{
				"duplicate-binding", "@input (mode = bind, id = \"b\", class = storage) block b { f32 x; };\nfn main() {}\n", [
				](StringView s_) {
					auto mod = mk(s_);
					if (mod.bindings.empty()) {
						ADD_FAILURE() << "mk failed";
						return Diagnostics {};
					}
					const auto layout = layoutOf(mod);
					mod.bindings.emplace_back(mod.bindings.front());
					return emitD(mod, layout);
				},
				R"GOLD(error[ASL0050]: binding 'b' is declared twice
  --> M:1:55
  |
1 | @input (mode = bind, id = "b", class = storage) block b { f32 x; };
  |                                                       ^
)GOLD"
			},
			{
				"binding-slot-conflict", "", [](StringView) {
					const std::array types { cameraTypes(false), cameraTypes(true) };
					const std::array stages {
						Vector<InterfaceEntry> { ie("camera", TypeRef { 1u }, BindMode::eBind, BindClass::eUniformBuffer) },
						Vector<InterfaceEntry> { ie("camera", TypeRef { 0u }, BindMode::eBind, BindClass::eUniformBuffer) }
					};
					auto res = assignPipelineLayout(stages, types, {});
					return res.has_value() ? Diagnostics {} : std::move(res).error();
				},
				R"GOLD(error[ASL0051]: binding 'camera' has another class or type in stage 0 and stage 1
)GOLD"
			},
			{
				"spec-constant-conflict", "", [](StringView) {
					ConstantDecl x {};
					x.name = "z";
					x.type = TypeRef { 0u };
					x.specId = String { "c" };
					ConstantDecl y = x;
					y.type = TypeRef { 1u };
					const std::array types { cameraTypes(false), cameraTypes(false) };
					const std::array constants { Vector<ConstantDecl> { x }, Vector<ConstantDecl> { y } };
					auto res = assignPipelineSpecConstants(constants, types);
					return res.has_value() ? Diagnostics {} : std::move(res).error();
				},
				R"GOLD(error[ASL0052]: spec constant 'c' has another type in stage 0 and stage 1
)GOLD"
			},
			{
				"too-many-push-constant-blocks", "@input (mode = bind, id = \"p\", class = push) block p { f32 y; };\nfn main() {}\n", [
				](StringView s_) {
					auto mod = mk(s_);
					if (mod.bindings.empty()) {
						ADD_FAILURE() << "mk failed";
						return Diagnostics {};
					}
					const auto layout = layoutOf(mod);
					auto second = mod.bindings.front();
					second.id = "q";
					mod.bindings.emplace_back(second);
					return emitD(mod, layout);
				},
				R"GOLD(error[ASL0053]: a stage has at most one push constant block, but the module declares 2
  --> M:1:52
  |
1 | @input (mode = bind, id = "p", class = push) block p { f32 y; };
  |                                                    ^
)GOLD"
			},
			{
				"invalid-push-constant-type", "@input (mode = bind, id = \"p\", class = push) block p { f32 y; };\nfn main() {}\n", [
				](StringView s_) {
					const auto mod = mk(s_);
					auto layout = layoutOf(mod);
					for (auto& range : layout.push) { range.size = 0uL; }
					return emitD(mod, layout);
				},
				R"GOLD(error[ASL0054]: push constant 'p' cannot be laid out
  --> M:1:52
  |
1 | @input (mode = bind, id = "p", class = push) block p { f32 y; };
  |                                                    ^
)GOLD"
			},
			{
				"conflicting-struct-definitions",
				"struct S { f32 a; }\n@input (mode = bind, id = \"b\", class = storage) block b { S s; };\nfn main() {}\n", [
				](StringView s_) {
					auto mod = mk(s_);
					if (mod.bindings.empty()) {
						ADD_FAILURE() << "mk failed";
						return Diagnostics {};
					}
					const auto layout = layoutOf(mod);
					const auto sIdx = mod.types[mod.bindings.front().type.index].fields.front().type.index;
					auto other = mod.types[sIdx];
					other.fields.emplace_back(FieldDesc { "extra", other.fields.front().type });
					mod.types.emplace_back(std::move(other));
					mod.types[mod.bindings.front().type.index].fields.emplace_back(
						FieldDesc { "t", TypeRef { static_cast<u32>(mod.types.size() - 1uz) } }
					);
					return emitD(mod, layout);
				},
				R"GOLD(error[ASL0055]: conflicting definitions of struct 'S'
  --> M:2:55
  |
2 | @input (mode = bind, id = "b", class = storage) block b { S s; };
  |                                                       ^
)GOLD"
			},
			{
				"missing-binding-class", "", [](StringView) {
					return checkContract({ ie("albedo", TypeRef { 0 }, BindMode::eBind, BindClass::eNone) }, {}, { "albedo" });
				},
				R"GOLD(error[ASL0056]: binding 'albedo' has no class
)GOLD"
			},
			{
				"binding-class-mismatch", "@input (mode = bind, id = \"b\", class = storage) block b { f32 x; };\nfn main() {}\n", [
				](StringView s_) {
					auto mod = mk(s_);
					if (mod.bindings.empty()) {
						ADD_FAILURE() << "mk failed";
						return Diagnostics {};
					}
					const auto layout = layoutOf(mod);
					mod.bindings.front().bindClass = BindClass::eTexture;
					return emitD(mod, layout);
				},
				R"GOLD(error[ASL0057]: type of binding 'b' does not fit its class
  --> M:1:55
  |
1 | @input (mode = bind, id = "b", class = storage) block b { f32 x; };
  |                                                       ^
)GOLD"
			},
			{
				"missing-material-binding", "", [](StringView) {
					const Vector<InterfaceEntry> iface {
						ie("block", TypeRef { 0 }, BindMode::eBind, BindClass::ePushConstant),
						ie("view", TypeRef { 0 }, BindMode::eBind, BindClass::eStorageBuffer)
					};
					return checkContract(iface, StageContract { .provides = {}, .requires_ = {} }, {});
				},
				R"GOLD(error[ASL0058]: binding 'block' has no material binding and is not provided by the stage
)GOLD"
			},
			{
				"missing-stage-output", "", [](StringView) {
					const Vector<InterfaceEntry> iface { ie("model", TypeRef { 0 }, BindMode::eBind, BindClass::eStorageBuffer) };
					return checkContract(iface, StageContract { .provides = {}, .requires_ = { ContractEntry { .id = "depth" }, ContractEntry { .id = "normal" } } }, { "model" });
				},
				R"GOLD(error[ASL0059]: stage requires 'depth', but the shader does not output it
)GOLD"
			},
			{
				"unmatched-stage-input", "", [](StringView) {
					const std::array<std::pair<EntryKind, Vector<InterfaceEntry>>, 2> chain {
						std::pair { EntryKind::eGeometry, Vector<InterfaceEntry> { fwd("a", "f32", false) } },
						std::pair { EntryKind::eFragment, Vector<InterfaceEntry> { fwd("world-pos", "f32vec4", true) } }
					};
					return checkStageChain(chain);
				},
				R"GOLD(error[ASL0060]: fragment input 'world-pos' has no matching output in the geometry stage
)GOLD"
			},
			{
				"stage-interface-mismatch", "out f32vec3 uv;\nin f32vec2 uv;\n", [](StringView) {
					auto out = fwd("uv", "f32vec3", false);
					out.at = SourceSpan { .file = "M", .begin = 12uL, .end = 14uL, .line = 1uL, .column = 13uL };
					auto in = fwd("uv", "f32vec2", true);
					in.at = SourceSpan { .file = "M", .begin = 27uL, .end = 29uL, .line = 2uL, .column = 12uL };
					const std::array<std::pair<EntryKind, Vector<InterfaceEntry>>, 2> chain {
						std::pair { EntryKind::eVertex, Vector<InterfaceEntry> { out } },
						std::pair { EntryKind::eFragment, Vector<InterfaceEntry> { in } }
					};
					return checkStageChain(chain);
				},
				R"GOLD(error[ASL0061]: fragment input 'uv' is 'f32vec2', but the vertex stage outputs 'f32vec3'
  --> M:2:12
  |
2 | in f32vec2 uv;
  |            ^^
  --> M:1:13: output declared here
  |
1 | out f32vec3 uv;
  |             ^^
)GOLD"
			},
			{
				"contract-class-mismatch",
				"@input (mode = bind, id = \"lights\", class = storage) block lights { f32 x; };\nfn main() {}\n", [](StringView s_) {
					const StageContract stage {
						.provides = { ContractEntry { .id = "lights", .cls = BindClass::eUniformBuffer, .at = passAt() } }, .requires_ = {}
					};
					return checkContract(computeInterface(mk(s_)), stage, {});
				},
				R"GOLD(error[ASL0075]: binding 'lights' is 'storage', but the stage provides 'uniform'
  --> M:1:60
  |
1 | @input (mode = bind, id = "lights", class = storage) block lights { f32 x; };
  |                                                            ^^^^^^
  --> pass:3:5: provided here
)GOLD"
			},
			{
				"contract-type-mismatch",
				"@output (mode = forward, id = \"color\") f32vec4 color;\nfn main() {\n\tcolor = f32vec4(1.0);\n}\n", [](StringView s_) {
					const StageContract stage {
						.provides = {}, .requires_ = { ContractEntry { .id = "color", .typeName = "f32vec3", .at = passAt() } }
					};
					return checkContract(computeInterface(mk(s_)), stage, {});
				},
				R"GOLD(error[ASL0076]: output 'color' is 'f32vec4', but the stage requires 'f32vec3'
  --> M:1:48
  |
1 | @output (mode = forward, id = "color") f32vec4 color;
  |                                                ^^^^^
  --> pass:3:5: required here
)GOLD"
			},
			{
				"missing-vertex-offset", shadowVertex, [](StringView s_) {
					const auto mod = mk(s_, EntryKind::eVertex);
					auto res = lowerToIntermediate(mod, layoutOf(mod), Tier::eDev);
					return res.has_value() ? Diagnostics {} : std::move(res).error();
				},
				R"GOLD(error[ASL0062]: vertex input 'vertex-position' has no vertex offset
)GOLD"
			},
			{
				"unsupported-feature", "@input (mode = forward, id = \"v\") u8vec4 v;\nfn main() {\n\tlet a = v;\n}\n", [](StringView s_) {
					const auto mod = mk(s_);
					auto lowered = lowerReferences(mod);
					if (not lowered.has_value()) { return std::move(lowered).error(); }
					return spirv::emit(*lowered, layoutOf(*lowered), Tier::eDev).diagnostics;
				},
				R"GOLD(error[ASL0073]: module 'M': binding 'v' passes an 8-bit type through the stage interface
  --> M:1:42
  |
1 | @input (mode = forward, id = "v") u8vec4 v;
  |                                          ^
)GOLD"
			},
		};
	}

	TEST(IrDiagnosticGolden, EveryCode) {
		for (const auto& row : table) {
			SCOPED_TRACE(std::string(row.key));
			const auto diags = row.produce(row.src);
			const auto hit = std::ranges::find_if(
				diags,
				[&](const auto& d_) { return d_.code() != nullptr && StringView { d_.code()->key } == row.key; }
			);
			if (hit == diags.end()) {
				std::string keys {};
				for (const auto& d : diags) { keys += (d.code() != nullptr ? std::string(d.code()->key) : std::string("?")) + " "; }
				ADD_FAILURE() << "NOPRODUCE " << row.key << " got: " << keys;
				continue;
			}
			expectWording(*hit);
			const ModuleSources sources {
				{ "M", row.src }, { "a", "#include <b>\nfn A() -> f32;" }, { "b", "#include <a>\nfn B() -> f32;" }
			};
			EXPECT_EQ(diag::render(*hit, &sources), row.rendered) << "ACTUAL[" << row.key << "]";
		}
	}

	TEST(IrDiagnosticGolden, CoversEveryCode) {
		for (const auto& info : codes::table) {
			const auto count = std::ranges::count_if(table, [&](const auto& r_) { return r_.key == StringView { info.key }; });
			/* retired codes keep their number but have no case */
			const auto exempt = StringView { info.key }.starts_with("internal-") || StringView { info.key }.starts_with("retired-");
			EXPECT_EQ(count, exempt ? 0 : 1) << info.key;
		}
		EXPECT_EQ(std::size(table), 68uz);
	}
}
