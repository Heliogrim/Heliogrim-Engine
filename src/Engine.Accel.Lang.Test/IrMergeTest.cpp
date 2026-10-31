#include "pch.h"

#include <algorithm>
#include <array>

#include <Engine.Accel.Lang/Ir/Analysis.hpp>
#include <Engine.Accel.Lang/Ir/Merge.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>

using namespace hg;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] Module parse(StringView name_, StringView source_, EntryKind entry_ = EntryKind::eFragment) {
			auto res = parseModule(name_, source_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] Module mergeOk(std::initializer_list<const Module*> modules_) {
			const Vector<const Module*> list { modules_ };
			auto res = merge(list);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] Diagnostics mergeFail(std::initializer_list<const Module*> modules_) {
			const Vector<const Module*> list { modules_ };
			auto res = merge(list);
			EXPECT_FALSE(res.has_value());
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}

		[[nodiscard]] bool mentions(ref<const Diagnostic> diag_, StringView text_) {
			return diag_.message.find(text_) != String::npos;
		}

		[[nodiscard]] u32 functionIndex(ref<const Module> module_, StringView name_) {
			for (auto i = 0uz; i < module_.functions.size(); ++i) {
				if (module_.functions[i].name == name_) {
					return static_cast<u32>(i);
				}
			}
			ADD_FAILURE() << "no function " << name_;
			return ~0u;
		}

		[[nodiscard]] u32 bindingIndex(ref<const Module> module_, StringView id_) {
			for (auto i = 0uz; i < module_.bindings.size(); ++i) {
				if (module_.bindings[i].id == id_) {
					return static_cast<u32>(i);
				}
			}
			ADD_FAILURE() << "no binding " << id_;
			return ~0u;
		}

		/* Structural equality of whole modules (Module has no operator==) */

		[[nodiscard]] bool same(cref<Condition> a_, cref<Condition> b_) {
			if (a_.kind != b_.kind || a_.toggle != b_.toggle || a_.value != b_.value || a_.children.size() != b_.children.size()) {
				return false;
			}
			for (auto i = 0uz; i < a_.children.size(); ++i) {
				if (not same(a_.children[i], b_.children[i])) {
					return false;
				}
			}
			return true;
		}

		[[nodiscard]] bool same(cref<FieldDesc> a_, cref<FieldDesc> b_) {
			return a_.name == b_.name && a_.type == b_.type;
		}

		[[nodiscard]] bool same(cref<TypeDesc> a_, cref<TypeDesc> b_) {
			return a_.kind == b_.kind && a_.scalar == b_.scalar && a_.bits == b_.bits && a_.n == b_.n && a_.m == b_.m &&
				a_.element == b_.element && a_.count == b_.count && a_.name == b_.name && a_.dim == b_.dim &&
				std::ranges::equal(a_.fields, b_.fields, [](const auto& x_, const auto& y_) { return same(x_, y_); });
		}

		[[nodiscard]] bool same(cref<Toggle> a_, cref<Toggle> b_) {
			return a_.name == b_.name && a_.values == b_.values && a_.defaultValue == b_.defaultValue;
		}

		[[nodiscard]] bool same(cref<BindingDecl> a_, cref<BindingDecl> b_) {
			return a_.id == b_.id && a_.type == b_.type && a_.mode == b_.mode && a_.bindClass == b_.bindClass &&
				a_.implicit == b_.implicit && a_.input == b_.input && same(a_.when, b_.when) && a_.localName == b_.localName;
		}

		[[nodiscard]] bool same(cref<FunctionDecl> a_, cref<FunctionDecl> b_) {
			const auto params = std::ranges::equal(
				a_.params,
				b_.params,
				[](const auto& x_, const auto& y_) { return x_.name == y_.name && x_.type == y_.type; }
			);
			const auto body = a_.body.has_value() == b_.body.has_value() &&
				(not a_.body.has_value() || a_.body->root.index == b_.body->root.index);
			return a_.name == b_.name && params && a_.ret == b_.ret && body && a_.redefineAllowed == b_.redefineAllowed &&
				same(a_.when, b_.when);
		}

		[[nodiscard]] bool same(cref<Expr> a_, cref<Expr> b_) {
			return a_.kind == b_.kind && a_.name == b_.name && a_.binding.index == b_.binding.index &&
				a_.function.index == b_.function.index && a_.local == b_.local && a_.type == b_.type &&
				std::ranges::equal(a_.operands, b_.operands, [](auto x_, auto y_) { return x_.index == y_.index; });
		}

		[[nodiscard]] bool same(cref<Stmt> a_, cref<Stmt> b_) {
			return a_.kind == b_.kind && a_.name == b_.name && a_.type == b_.type && a_.lhs.index == b_.lhs.index &&
				a_.rhs.index == b_.rhs.index &&
				std::ranges::equal(a_.children, b_.children, [](auto x_, auto y_) { return x_.index == y_.index; }) &&
				same(a_.when, b_.when);
		}

		[[nodiscard]] bool same(cref<Module> a_, cref<Module> b_) {
			const auto each = [](const auto& x_, const auto& y_) {
				return std::ranges::equal(x_, y_, [](const auto& l_, const auto& r_) { return same(l_, r_); });
			};
			return a_.name == b_.name && a_.entry == b_.entry && each(a_.toggles, b_.toggles) && each(a_.types, b_.types) &&
				each(a_.bindings, b_.bindings) && each(a_.functions, b_.functions) && each(a_.exprs, b_.exprs) &&
				each(a_.stmts, b_.stmts);
		}

		constexpr StringView cameraMat4 = "@input (mode = bind, id = \"camera\", class = storage) block camera { f32mat4 vp; };\n";
		constexpr StringView cameraVec4 = "@input (mode = bind, id = \"camera\", class = storage) block camera { f32vec4 vp; };\n";
	}

	/* The brief's contract */

	TEST(MergeTest, MissingBodyNamesFunction) {
		const auto profile = parse("profile", "@input (mode = forward) f32vec4 offset;\n");
		const auto shader = parse("shader", "fn GetWorldPosition() -> f32vec4;\nfn main() {\n\tposition = GetWorldPosition();\n}\n");
		const auto diags = mergeFail({ &profile, &shader });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "missing body for function 'GetWorldPosition'")) << diags[0].message;
	}

	TEST(MergeTest, DuplicateBodyNamesBothModules) {
		const auto a = parse("A", "fn X() -> f32 { return 1.0; }\n");
		const auto b = parse("B", "fn X() -> f32 { return 0.5; }\n");
		const auto diags = mergeFail({ &a, &b });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "duplicate body for function 'X' in modules 'A' and 'B'")) << diags[0].message;
	}

	TEST(MergeTest, RedefineAllowedLastWins) {
		const auto a = parse("A", "@redefine (allow = true) fn Color() -> f32 { return 1.0; }\nfn main() {\n\tfrag_depth = Color();\n}\n");
		const auto b = parse("B", "fn Color() -> f32 { return 0.5; }\n");
		const auto merged = mergeOk({ &a, &b });

		ASSERT_EQ(std::ranges::count(merged.functions, String("Color"), &FunctionDecl::name), 1);
		const auto color = functionIndex(merged, "Color");
		const auto& fn = merged.functions[color];
		ASSERT_TRUE(fn.body.has_value());
		const auto& root = merged.stmts[fn.body->root.index];
		ASSERT_EQ(root.children.size(), 1uz);
		const auto& ret = merged.stmts[root.children[0].index];
		EXPECT_EQ(ret.kind, StmtKind::eReturn);
		EXPECT_EQ(merged.exprs[ret.lhs.index].name, "0.5");// B's body

		// A's call re-points to B's body
		EXPECT_EQ(usageOf(merged, functionIndex(merged, "main")).calledFunctions, (Vector<u32> { color }));
	}

	TEST(MergeTest, RedefineDoesNotRelaxOtherFunctions) {
		const auto a = parse("A", "@redefine (allow = true) fn Color() -> f32 { return 1.0; }\nfn Other() -> f32 { return 1.0; }\n");
		const auto b = parse("B", "fn Color() -> f32 { return 0.5; }\nfn Other() -> f32 { return 0.5; }\n");
		const auto diags = mergeFail({ &a, &b });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "duplicate body for function 'Other' in modules 'A' and 'B'")) << diags[0].message;
	}

	TEST(MergeTest, RedefineWithoutAllowOnTheReplacedBodyIsDiagnostic) {
		// A allows B to redefine; B does not allow C
		const auto a = parse("A", "@redefine (allow = true) fn Color() -> f32 { return 1.0; }\n");
		const auto b = parse("B", "fn Color() -> f32 { return 0.5; }\n");
		const auto c = parse("C", "fn Color() -> f32 { return 0.25; }\n");
		const auto diags = mergeFail({ &a, &b, &c });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "duplicate body for function 'Color' in modules 'B' and 'C'")) << diags[0].message;
	}

	TEST(MergeTest, BindingTypeConflictNamesBothModules) {
		const auto a = parse("A", cameraMat4);
		const auto b = parse("B", cameraVec4);
		const auto diags = mergeFail({ &a, &b });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(
			mentions(diags[0], "binding 'camera' declared as block { f32mat4 vp; } in 'A' and block { f32vec4 vp; } in 'B'")
		) << diags[0].message;
	}

	TEST(MergeTest, SameBindingSameTypeCollapses) {
		const auto a = parse("A", String(cameraMat4) + "fn GetViewProj() -> f32mat4 { return camera.vp; }\n");
		// camera is binding 1 in B, so B's reads must be re-pointed to the surviving binding 0
		const auto b = parse(
			"B",
			"@input (mode = forward) f32 extra;\n" + String(cameraMat4) +
			"fn GetViewProj() -> f32mat4;\nfn main() {\n\tlet m = camera.vp;\n\tlet n = GetViewProj();\n}\n"
		);
		const auto merged = mergeOk({ &a, &b });

		ASSERT_EQ(merged.bindings.size(), 2uz);
		const auto camera = bindingIndex(merged, "camera");
		EXPECT_EQ(camera, 0u);
		EXPECT_EQ(bindingIndex(merged, "extra"), 1u);
		EXPECT_EQ(usageOf(merged, functionIndex(merged, "main")).readBindings, (Vector<u32> { camera }));
		EXPECT_EQ(usageOf(merged, functionIndex(merged, "GetViewProj")).readBindings, (Vector<u32> { camera }));
	}

	TEST(MergeTest, SameBindingIdAcrossStagesStaysSeparate) {
		const auto profile = parse("profile", "@input (mode = forward) f32 shared;\n");
		const auto vertex = parse("vertex", String(cameraMat4) + "fn main() {\n\tlet m = camera.vp;\n}\n", EntryKind::eVertex);
		const auto fragment = parse("fragment", String(cameraVec4) + "fn main() {\n\tlet v = camera.vp;\n}\n");

		const auto vs = mergeOk({ &profile, &vertex });
		const auto fs = mergeOk({ &profile, &fragment });
		const auto vsAgain = mergeOk({ &profile, &vertex });

		EXPECT_EQ(vs.entry, EntryKind::eVertex);
		EXPECT_EQ(fs.entry, EntryKind::eFragment);
		const auto& vsCamera = vs.bindings[bindingIndex(vs, "camera")];
		const auto& fsCamera = fs.bindings[bindingIndex(fs, "camera")];
		const auto& vsField = vs.types[vsCamera.type.index].fields.front();
		const auto& fsField = fs.types[fsCamera.type.index].fields.front();
		EXPECT_EQ(vs.types[vsField.type.index].kind, TypeKind::eMatrix);
		EXPECT_EQ(fs.types[fsField.type.index].kind, TypeKind::eVector);
		// no state survives a merge call
		EXPECT_TRUE(same(vs, vsAgain));
	}

	TEST(MergeTest, ToggleDomainConflict) {
		const auto a = parse("A", "toggle Shading : { Forward, Deferred } = Forward;\n");
		const auto b = parse("B", "toggle Shading : { Forward, Clustered } = Forward;\n");
		const auto diags = mergeFail({ &a, &b });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "toggle 'Shading' declared with different values in 'A' and 'B'")) << diags[0].message;
	}

	/* Rulings */

	TEST(MergeTest, ToggleSameDomainCollapsesInAnyValueOrder) {
		const auto a = parse("A", "toggle Shading : { Forward, Deferred } = Forward;\n");
		const auto b = parse("B", "toggle Shading : { Deferred, Forward } = Forward;\ntoggle Shadows : { Off, On } = Off;\n");
		const auto merged = mergeOk({ &a, &b });
		ASSERT_EQ(merged.toggles.size(), 2uz);
		EXPECT_EQ(merged.toggles[0].name, "Shading");
		EXPECT_EQ(merged.toggles[0].values, (Vector<String> { "Forward", "Deferred" }));// first declaration's order
		EXPECT_EQ(merged.toggles[1].name, "Shadows");
	}

	TEST(MergeTest, ToggleDefaultConflict) {
		const auto a = parse("A", "toggle Shading : { Forward, Deferred } = Forward;\n");
		const auto b = parse("B", "toggle Shading : { Forward, Deferred } = Deferred;\n");
		const auto diags = mergeFail({ &a, &b });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "toggle 'Shading' defaults to 'Forward' in 'A' and 'Deferred' in 'B'")) << diags[0].message;
	}

	TEST(MergeTest, ForwardCallIntoOtherModuleIsRemapped) {
		// P's arena comes first, so every ref of S must be shifted to be found again
		const auto profile = parse(
			"P",
			"@input (mode = forward) f32vec3 local_position;\n"
			"fn Unused() -> f32 { return 1.0; }\n"
			"fn GetWorldPosition() -> f32vec4 {\n\treturn f32vec4(local_position, 1.0);\n}\n"
		);
		const auto shader = parse(
			"S",
			"@input (mode = forward) f32vec4 offset;\n"
			"fn GetWorldPosition() -> f32vec4;\n"
			"fn main() {\n\tposition = GetWorldPosition() + offset;\n}\n",
			EntryKind::eVertex
		);
		const auto merged = mergeOk({ &profile, &shader });

		EXPECT_EQ(merged.name, "S");
		EXPECT_EQ(merged.entry, EntryKind::eVertex);
		EXPECT_EQ(merged.functions.size(), 3uz);// the forward declaration is satisfied and dropped
		const auto main = functionIndex(merged, "main");
		const auto world = functionIndex(merged, "GetWorldPosition");
		ASSERT_TRUE(merged.functions[world].body.has_value());

		const auto usage = usageOf(merged, main);
		EXPECT_EQ(usage.calledFunctions, (Vector<u32> { world }));
		EXPECT_EQ(usage.readBindings, (Vector<u32> { bindingIndex(merged, "offset") }));
		EXPECT_EQ(usageOf(merged, world).readBindings, (Vector<u32> { bindingIndex(merged, "local_position") }));

		const std::array roots { main };
		auto expected = Vector<u32> { main, world };
		std::ranges::sort(expected);
		EXPECT_EQ(reachableFunctions(merged, roots), expected);
	}

	TEST(MergeTest, TypeEqualityIncludesBits) {
		const auto a = parse("A", "@input (mode = forward, id = \"tag\") u8 tag;\n");
		const auto b = parse("B", "@input (mode = forward, id = \"tag\") u32 tag;\n");
		const auto diags = mergeFail({ &a, &b });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "binding 'tag' declared as u8 in 'A' and u32 in 'B'")) << diags[0].message;
	}

	TEST(MergeTest, StructurallyEqualTypesAreDeduplicated) {
		const auto a = parse("A", String(cameraMat4) + "@input (mode = forward) f32vec4 x;\n");
		const auto b = parse("B", String(cameraMat4) + "@input (mode = forward) f32vec4 y;\n");
		const auto merged = mergeOk({ &a, &b });
		// f32mat4, the anonymous camera block, f32vec4: once each
		EXPECT_EQ(merged.types.size(), 3uz);
		EXPECT_EQ(merged.bindings[bindingIndex(merged, "x")].type, merged.bindings[bindingIndex(merged, "y")].type);
	}

	TEST(MergeTest, BindingFlagMismatchNamesBothModules) {
		struct Case {
			StringView a;
			StringView b;
			StringView message;
		};
		const std::array cases {
			Case {
				"@input (mode = bind, id = \"x\") f32 x;\n",
				"@input (mode = forward, id = \"x\") f32 x;\n",
				"binding 'x' declared with mode bind in 'A' and forward in 'B'"
			},
			Case {
				"@input (mode = bind, id = \"x\", class = storage) block x { f32 v; };\n",
				"@input (mode = bind, id = \"x\", class = uniform) block x { f32 v; };\n",
				"binding 'x' declared with class storage in 'A' and uniform in 'B'"
			},
			Case {
				"@input (mode = forward, id = \"x\") f32 x;\n",
				"@output (mode = forward, id = \"x\") f32 x;\n",
				"binding 'x' declared as input in 'A' and output in 'B'"
			},
			Case {
				"@output (mode = forward, id = \"x\") implicit f32 x;\n",
				"@output (mode = forward, id = \"x\") f32 x;\n",
				"binding 'x' declared implicit in 'A' and not in 'B'"
			}
		};
		for (const auto& c : cases) {
			const auto a = parse("A", c.a);
			const auto b = parse("B", c.b);
			const auto diags = mergeFail({ &a, &b });
			ASSERT_EQ(diags.size(), 1uz) << c.message;
			EXPECT_TRUE(mentions(diags[0], c.message)) << diags[0].message;
		}
	}

	TEST(MergeTest, SignatureMismatchNamesBothModules) {
		const auto a = parse("A", "fn F() -> f32;\nfn main() {\n\tfrag_depth = F();\n}\n");
		const auto b = parse("B", "fn F() -> f32vec4 { return f32vec4(1.0); }\n");
		const auto diags = mergeFail({ &a, &b });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "signature of function 'F' differs between 'A' and 'B'")) << diags[0].message;
	}

	TEST(MergeTest, WhenVariantBodiesAreNotDuplicates) {
		const auto a = parse(
			"A",
			"toggle Shading : { Forward, Deferred } = Forward;\n"
			"@when (Shading == Forward) fn F() -> f32 { return 1.0; }\n"
			"@when (Shading == Deferred) fn F() -> f32 { return 0.5; }\n"
		);
		const auto b = parse("B", "fn F() -> f32;\nfn main() {\n\tfrag_depth = F();\n}\n");
		const auto merged = mergeOk({ &a, &b });
		EXPECT_EQ(std::ranges::count(merged.functions, String("F"), &FunctionDecl::name), 2);
	}

	TEST(MergeTest, WhenVariantBindingsWithSameIdCollapseUnderEitherCondition) {
		const auto a = parse(
			"A",
			"toggle Shading : { Forward, Deferred } = Forward;\n"
			"@when (Shading == Forward) @input (mode = forward, id = \"light\") f32vec3 light;\n"
			"@when (Shading == Deferred) @input (mode = forward, id = \"light\") f32vec3 light;\n"
		);
		const auto merged = mergeOk({ &a });
		ASSERT_EQ(merged.bindings.size(), 1uz);
		EXPECT_TRUE(evaluate(merged.bindings[0].when, { { "Shading", "Forward" } }));
		EXPECT_TRUE(evaluate(merged.bindings[0].when, { { "Shading", "Deferred" } }));
	}

	TEST(MergeTest, IsDeterministic) {
		constexpr StringView profileSource =
			"toggle Shading : { Forward, Deferred } = Forward;\n"
			"@input (mode = forward) f32vec3 local_position;\n"
			"fn GetWorldPosition() -> f32vec4 {\n\treturn f32vec4(local_position, 1.0);\n}\n";
		constexpr StringView shaderSource =
			"@input (mode = forward) f32vec4 offset;\n"
			"fn GetWorldPosition() -> f32vec4;\n"
			"fn main() {\n\tposition = GetWorldPosition() + offset;\n}\n";
		const auto p1 = parse("P", profileSource);
		const auto s1 = parse("S", shaderSource);
		const auto p2 = parse("P", profileSource);
		const auto s2 = parse("S", shaderSource);

		const auto first = mergeOk({ &p1, &s1 });
		EXPECT_TRUE(same(first, mergeOk({ &p1, &s1 })));
		EXPECT_TRUE(same(first, mergeOk({ &p2, &s2 })));
	}

	/* Degenerate and hostile input */

	TEST(MergeTest, EmptyInputIsDiagnostic) {
		const auto diags = mergeFail({});
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "no modules")) << diags[0].message;
	}

	TEST(MergeTest, NullModuleIsDiagnostic) {
		const auto a = parse("A", "@input (mode = forward) f32 x;\n");
		const auto diags = mergeFail({ &a, nullptr });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "module #1 is null")) << diags[0].message;
	}

	TEST(MergeTest, SingleModuleMergesToItself) {
		const auto a = parse("A", String(cameraMat4) + "fn main() {\n\tlet m = camera.vp;\n}\n");
		EXPECT_TRUE(same(mergeOk({ &a }), a));
	}

	TEST(MergeTest, ModuleWithoutBodiesMerges) {
		const auto a = parse("A", "toggle Shading : { Forward } = Forward;\n@input (mode = forward) f32 x;\n");
		const auto merged = mergeOk({ &a });
		EXPECT_EQ(merged.bindings.size(), 1uz);
		EXPECT_TRUE(merged.functions.empty());
	}

	TEST(MergeTest, SameModuleTwiceIsDuplicateBody) {
		const auto a = parse("A", String(cameraMat4) + "fn main() {\n\tlet m = camera.vp;\n}\n");
		const auto diags = mergeFail({ &a, &a });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "duplicate body for function 'main' in modules 'A' and 'A'")) << diags[0].message;
	}

	TEST(MergeTest, OutOfRangeRefsAreDiagnostics) {
		Module valid {};
		valid.name = "hand";
		valid.types.emplace_back();
		auto& binding = valid.bindings.emplace_back();
		binding.id = "x";
		binding.type = TypeRef { 0u };
		binding.localName = "x";
		auto& expr = valid.exprs.emplace_back();
		expr.kind = ExprKind::eNameRef;
		expr.name = "x";
		expr.binding = BindingRef { 0u };
		auto& stmt = valid.stmts.emplace_back();
		stmt.kind = StmtKind::eExpr;
		stmt.lhs = ExprRef { 0u };
		auto& fn = valid.functions.emplace_back();
		fn.name = "main";
		fn.body = BodyRef { StmtRef { 0u } };
		static_cast<void>(mergeOk({ &valid }));

		std::array<Module, 7> invalid {};
		std::ranges::fill(invalid, valid);
		invalid[0].exprs[0].binding.index = 5u;
		invalid[1].exprs[0].operands.emplace_back(ExprRef { 9u });
		invalid[2].exprs[0].function.index = 3u;
		invalid[3].stmts[0].lhs.index = 7u;
		invalid[4].stmts[0].children.emplace_back(StmtRef { 4u });
		invalid[5].functions[0].body = BodyRef { StmtRef { 2u } };
		// a type may only refer to an earlier type (no cycles)
		invalid[6].types[0].kind = TypeKind::eArray;
		invalid[6].types[0].element = TypeRef { 0u };

		for (const auto& module : invalid) {
			const auto diags = mergeFail({ &valid, &module });
			ASSERT_EQ(diags.size(), 1uz);
			EXPECT_TRUE(mentions(diags[0], "module 'hand'")) << diags[0].message;
			EXPECT_TRUE(mentions(diags[0], "out of range")) << diags[0].message;
		}
	}
}
