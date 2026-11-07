#include "pch.h"

#include <algorithm>
#include <array>
#include <functional>
#include <utility>

#include <Engine.Accel.Lang/Ir/Analysis.hpp>
#include <Engine.Accel.Lang/Ir/Merge.hpp>
#include <Engine.Accel.Lang/Ir/Parser.hpp>
#include <Engine.Accel.Lang/Ir/Resolve.hpp>
#include <Engine.Accel.Lang/Ir/Slice.hpp>

#include "IrSame.hpp"

using namespace hg;
using namespace hg::engine::accel::lang::ir;

namespace AccelLang {
	namespace {
		constexpr u32 unset = ~0uL;

		[[nodiscard]] Opt<String> noIncludes(StringView) {
			return None;
		}

		[[nodiscard]] Module parse(StringView source_, StringView name_ = "M", EntryKind entry_ = EntryKind::eFragment) {
			auto res = parseModule(name_, source_, entry_, ModuleResolver { noIncludes });
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : Module {};
		}

		[[nodiscard]] bool mentions(ref<const Diagnostic> diag_, StringView text_) {
			return diag_.message.find(text_) != String::npos;
		}

		[[nodiscard]] u32 functionIndex(ref<const Module> module_, StringView name_) {
			const auto it = std::ranges::find(module_.functions, name_, &FunctionDecl::name);
			EXPECT_NE(it, module_.functions.end()) << "no function " << name_;
			return it == module_.functions.end() ? unset : static_cast<u32>(it - module_.functions.begin());
		}

		[[nodiscard]] u32 bindingIndex(ref<const Module> module_, StringView id_) {
			const auto it = std::ranges::find(module_.bindings, id_, &BindingDecl::id);
			EXPECT_NE(it, module_.bindings.end()) << "no binding " << id_;
			return it == module_.bindings.end() ? unset : static_cast<u32>(it - module_.bindings.begin());
		}

		[[nodiscard]] FunctionDecl function(StringView name_, u32 root_) {
			FunctionDecl fn {};
			fn.name = name_;
			fn.body = BodyRef { StmtRef { root_ } };
			return fn;
		}

		[[nodiscard]] Vector<String> functionNames(ref<const Module> module_) {
			Vector<String> names {};
			for (const auto& fn : module_.functions) {
				names.emplace_back(fn.name);
			}
			return names;
		}

		[[nodiscard]] Vector<String> bindingIds(ref<const Module> module_, bool inputsOnly_ = false) {
			Vector<String> ids {};
			for (const auto& binding : module_.bindings) {
				if (not inputsOnly_ || binding.input) {
					ids.emplace_back(binding.id);
				}
			}
			return ids;
		}

		[[nodiscard]] ref<const Stmt> bodyRoot(ref<const Module> module_, StringView function_) {
			const auto& fn = module_.functions.at(functionIndex(module_, function_));
			return module_.stmts.at(fn.body.value().root.index);
		}

		/* every ref of the module is in range or unset */
		void expectConsistent(ref<const Module> module_) {
			const auto check = [](u32 index_, size_t limit_, StringView what_) {
				EXPECT_TRUE(index_ == unset || index_ < limit_) << what_ << " ref " << index_ << " out of range " << limit_;
			};
			const auto types = module_.types.size();
			for (const auto& type : module_.types) {
				check(type.element.index, types, "type");
				for (const auto& field : type.fields) {
					check(field.type.index, types, "field type");
				}
			}
			for (const auto& binding : module_.bindings) {
				check(binding.type.index, types, "binding type");
			}
			for (const auto& fn : module_.functions) {
				check(fn.ret.index, types, "return type");
				for (const auto& param : fn.params) {
					check(param.type.index, types, "param type");
				}
				if (fn.body.has_value()) {
					check(fn.body->root.index, module_.stmts.size(), "body root");
				}
			}
			for (const auto& e : module_.exprs) {
				check(e.binding.index, module_.bindings.size(), "expr binding");
				check(e.function.index, module_.functions.size(), "expr function");
				check(e.type.index, types, "expr type");
				for (const auto& op : e.operands) {
					check(op.index, module_.exprs.size(), "operand");
				}
			}
			for (const auto& s : module_.stmts) {
				check(s.type.index, types, "stmt type");
				check(s.lhs.index, module_.exprs.size(), "stmt lhs");
				check(s.rhs.index, module_.exprs.size(), "stmt rhs");
				for (const auto& child : s.children) {
					check(child.index, module_.stmts.size(), "stmt child");
				}
			}
		}

		/* consistent, and every arena node is reachable from a function body (no dead nodes left to hash) */
		void expectCompact(ref<const Module> module_) {
			expectConsistent(module_);
			Vector<bool> stmts(module_.stmts.size(), false);
			Vector<bool> exprs(module_.exprs.size(), false);
			Vector<u32> work {};
			for (const auto& fn : module_.functions) {
				if (fn.body.has_value() && fn.body->root.index < stmts.size()) {
					work.emplace_back(fn.body->root.index);
				}
			}
			Vector<u32> exprWork {};
			while (not work.empty()) {
				const auto at = work.back();
				work.pop_back();
				if (stmts[at]) {
					continue;
				}
				stmts[at] = true;
				const auto& s = module_.stmts[at];
				for (const auto ref : { s.lhs.index, s.rhs.index }) {
					if (ref < exprs.size()) {
						exprWork.emplace_back(ref);
					}
				}
				for (const auto& child : s.children) {
					if (child.index < stmts.size()) {
						work.emplace_back(child.index);
					}
				}
			}
			while (not exprWork.empty()) {
				const auto at = exprWork.back();
				exprWork.pop_back();
				if (exprs[at]) {
					continue;
				}
				exprs[at] = true;
				for (const auto& op : module_.exprs[at].operands) {
					if (op.index < exprs.size()) {
						exprWork.emplace_back(op.index);
					}
				}
			}
			EXPECT_TRUE(std::ranges::all_of(stmts, std::identity {})) << "unreachable statement";
			EXPECT_TRUE(std::ranges::all_of(exprs, std::identity {})) << "unreachable expression";
		}

		[[nodiscard]] bool hasLiteral(ref<const Module> module_, StringView text_) {
			return std::ranges::any_of(
				module_.exprs,
				[&](ref<const Expr> e_) { return e_.kind == ExprKind::eLiteral && e_.name == text_; }
			);
		}

		[[nodiscard]] Module resolveOk(ref<const Module> module_, ref<const ToggleSet> toggles_) {
			auto res = resolveToggles(module_, toggles_);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			if (not res.has_value()) {
				return Module {};
			}
			expectCompact(*res);
			return std::move(res).value();
		}

		[[nodiscard]] Diagnostics resolveFail(ref<const Module> module_, ref<const ToggleSet> toggles_) {
			auto res = resolveToggles(module_, toggles_);
			EXPECT_FALSE(res.has_value());
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}

		[[nodiscard]] ResolveResult fixOk(ref<const Module> module_, ref<const ToggleSources> sources_) {
			auto res = fixToggles(module_, sources_);
			EXPECT_TRUE(res.has_value()) << (res.has_value() || res.error().empty() ? "" : res.error().front().message);
			return res.has_value() ? std::move(res).value() : ResolveResult {};
		}

		[[nodiscard]] Diagnostics fixFail(ref<const Module> module_, ref<const ToggleSources> sources_) {
			auto res = fixToggles(module_, sources_);
			EXPECT_FALSE(res.has_value());
			return res.has_value() ? Diagnostics {} : std::move(res).error();
		}

		[[nodiscard]] Module sliceChecked(ref<const Module> module_, std::initializer_list<String> required_) {
			const Vector<String> required { required_ };
			const auto copy = module_;
			auto result = slice(module_, required);
			EXPECT_TRUE(same(copy, module_)) << "slice mutated its input";
			expectConsistent(result);
			return result;
		}

		constexpr StringView shading = "toggle Shading : { Forward, Deferred } = Forward;\n";
		constexpr StringView shadows = "toggle Shadows : { Off, On } = Off;\n";

		/* depth + lighting + shadow-map functions */
		constexpr StringView litSource =
			"@input (mode = forward) f32vec3 normal;\n"
			"@input (mode = forward) f32vec2 uv;\n"
			"@input (mode = bind, id = \"light\", class = uniform) block light { f32vec3 dir; };\n"
			"@input (mode = bind, id = \"shadow_map\", class = texture) texture2d shadow_map;\n"
			"@input (mode = bind, id = \"shadow_sampler\", class = sampler) sampler shadow_sampler;\n"
			"@input (mode = forward) f32 depth_bias;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
			"fn ShadowFactor() -> f32 { return sample(shadow_map, shadow_sampler, uv).x; }\n"
			"fn Lighting() -> f32vec3 { return light.dir * normal * ShadowFactor(); }\n"
			"fn Depth() -> f32 { return 0.5 + depth_bias; }\n"
			"fn main() {\n"
			"\tlet lit = Lighting();\n"
			"\tcolor = f32vec4(lit, 1.0);\n"
			"\tfrag_depth = Depth();\n"
			"}\n";
	}

	/* The brief's contract */

	TEST(ResolveTest, PassValueWinsOverMaterialWithWarning) {
		const auto module = parse(shading);
		const auto fixed = fixOk(
			module,
			ToggleSources { .pass = { { "Shading", "Deferred" } }, .profile = {}, .material = { { "Shading", "Forward" } } }
		);
		EXPECT_EQ(fixed.toggles, (ToggleSet { { "Shading", "Deferred" } }));
		ASSERT_EQ(fixed.warnings.size(), 1uz);
		EXPECT_EQ(fixed.warnings[0].severity, Severity::eWarning);
		EXPECT_TRUE(mentions(fixed.warnings[0], "'Shading'")) << fixed.warnings[0].message;
		EXPECT_TRUE(mentions(fixed.warnings[0], "material")) << fixed.warnings[0].message;
	}

	TEST(ResolveTest, DefaultsApplyWhenUnset) {
		const auto module = parse(String(shadows) + String(shading));
		const auto fixed = fixOk(module, ToggleSources { .pass = {}, .profile = { { "Shadows", "On" } }, .material = {} });
		// sorted by name, every declared toggle present
		EXPECT_EQ(fixed.toggles, (ToggleSet { { "Shading", "Forward" }, { "Shadows", "On" } }));
		EXPECT_TRUE(fixed.warnings.empty());

		const auto none = fixOk(module, {});
		EXPECT_EQ(none.toggles, (ToggleSet { { "Shading", "Forward" }, { "Shadows", "Off" } }));
	}

	TEST(ResolveTest, ExhaustivenessErrorNamesCombination) {
		const auto module = parse(
			String(shading) + String(shadows) +
			"fn GetWorldPosition() -> f32vec4;\n"
			"@when (Shading == Forward) fn GetWorldPosition() -> f32vec4 { return f32vec4(1.0); }\n"
			"@when (Shading == Deferred && Shadows == On) fn GetWorldPosition() -> f32vec4 { return f32vec4(0.5); }\n"
			"fn main() {\n\tposition = GetWorldPosition();\n}\n"
		, "M", EntryKind::eVertex);
		// checked ahead of time: the requested combination itself is covered
		const auto diags = resolveFail(module, { { "Shading", "Forward" }, { "Shadows", "Off" } });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].message, "no body for 'GetWorldPosition()' at Shading=Deferred, Shadows=Off");
	}

	TEST(ResolveTest, WhenBlockInputsAreInterfaceOfThatVariant) {
		const auto module = parse(
			String(shading) +
			"@input (mode = forward) f32vec3 normal;\n"
			"@when (Shading == Forward) {\n"
			"\t@input (mode = forward) f32vec3 light;\n"
			"\tfn Lit() -> f32vec3 { return light * normal; }\n"
			"}\n"
			"@when (Shading == Deferred) {\n"
			"\tfn Lit() -> f32vec3 { return normal; }\n"
			"}\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tcolor = f32vec4(Lit(), 1.0);\n}\n"
		);

		const auto forward = resolveOk(module, { { "Shading", "Forward" } });
		EXPECT_EQ(bindingIds(forward, true), (Vector<String> { "normal", "light" }));
		EXPECT_EQ(functionNames(forward), (Vector<String> { "Lit", "main" }));
		EXPECT_EQ(
			usageOf(forward, functionIndex(forward, "Lit")).readBindings,
			(Vector<u32> { bindingIndex(forward, "normal"), bindingIndex(forward, "light") })
		);

		const auto deferred = resolveOk(module, { { "Shading", "Deferred" } });
		EXPECT_EQ(bindingIds(deferred, true), (Vector<String> { "normal" }));
		EXPECT_EQ(usageOf(deferred, functionIndex(deferred, "main")).calledFunctions, (Vector<u32> { functionIndex(deferred, "Lit") }));
		EXPECT_EQ(usageOf(deferred, functionIndex(deferred, "Lit")).readBindings, (Vector<u32> { bindingIndex(deferred, "normal") }));
		// a resolved module carries no conditions
		for (const auto& binding : deferred.bindings) {
			EXPECT_EQ(binding.when.kind, ConditionKind::eTrue);
		}
		for (const auto& fn : deferred.functions) {
			EXPECT_EQ(fn.when.kind, ConditionKind::eTrue);
		}
	}

	TEST(SliceTest, DepthOnlyDropsLightingAndItsFunctionsAndBindings) {
		const auto module = parse(litSource);
		const auto sliced = sliceChecked(module, { "depth" });
		expectCompact(sliced);

		EXPECT_EQ(functionNames(sliced), (Vector<String> { "Depth", "main" }));
		EXPECT_EQ(bindingIds(sliced), (Vector<String> { "depth_bias", "depth" }));
		const auto main = functionIndex(sliced, "main");
		EXPECT_EQ(bodyRoot(sliced, "main").children.size(), 1uz);// frag_depth = Depth();
		const std::array roots { main };
		EXPECT_EQ(reachableFunctions(sliced, roots), (Vector<u32> { functionIndex(sliced, "Depth"), main }));
		EXPECT_EQ(usageOf(sliced, functionIndex(sliced, "Depth")).readBindings, (Vector<u32> { bindingIndex(sliced, "depth_bias") }));

		// color keeps lighting and the shadow map, and the depth path (frag_depth is a builtin, always kept)
		const auto color = sliceChecked(module, { "color" });
		expectCompact(color);
		EXPECT_EQ(functionNames(color), (Vector<String> { "ShadowFactor", "Lighting", "Depth", "main" }));
		EXPECT_EQ(
			bindingIds(color),
			(Vector<String> { "normal", "uv", "light", "shadow_map", "shadow_sampler", "depth_bias", "color" })
		);
	}

	TEST(SliceTest, DropsUnusedConstant) {
		const auto module = parse(
			"const USED: f32 = 1.0;\nconst DEAD: f32 = 2.0;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tcolor = f32vec4(USED);\n}\n"
		);
		ASSERT_EQ(module.constants.size(), 2uz);
		const auto sliced = sliceChecked(module, { "color" });
		ASSERT_EQ(sliced.constants.size(), 1uz);
		EXPECT_EQ(sliced.constants[0].name, "USED");
	}

	TEST(SliceTest, EmptyRequiredKeepsEverything) {
		const auto module = parse(litSource);
		EXPECT_TRUE(same(sliceChecked(module, {}), module));
	}

	TEST(SliceTest, ImplicitOutputOnlyShaderSlicesToEmptyBody) {
		const auto module = parse(
			"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"@input (mode = forward) f32vec4 tint;\n"
			"fn main() {\n\tcolor = tint;\n}\n"
		);
		const auto sliced = sliceChecked(module, { "depth" });
		expectCompact(sliced);
		EXPECT_EQ(functionNames(sliced), (Vector<String> { "main" }));
		EXPECT_EQ(bindingIds(sliced), (Vector<String> { "depth" }));
		EXPECT_TRUE(sliced.bindings[0].implicit);
		const auto& root = bodyRoot(sliced, "main");
		EXPECT_EQ(root.kind, StmtKind::eBlock);
		EXPECT_TRUE(root.children.empty());

		// the pre-depth shape: empty main plus only the implicit output stays as it is
		const auto empty = parse("@output (mode = forward, id = \"depth\") implicit f32 depth;\nfn main() {}\n");
		const auto again = sliceChecked(empty, { "depth" });
		EXPECT_TRUE(same(again, empty));
	}

	/* Rulings: toggle fixing */

	TEST(ResolveTest, UnknownToggleOrValueNamesSource) {
		const auto module = parse(shading);
		struct Case {
			ToggleSources sources;
			Vector<StringView> words;
		};
		const std::array cases {
			Case { { .pass = { { "Nope", "X" } }, .profile = {}, .material = {} }, { "'Nope'", "pass" } },
			Case { { .pass = {}, .profile = { { "Shading", "Clustered" } }, .material = {} }, { "'Shading'", "'Clustered'", "profile" } },
			Case { { .pass = {}, .profile = {}, .material = { { "Shading", "" } } }, { "'Shading'", "material" } },
			Case { { .pass = { { "Shading", "Forward" }, { "Shading", "Forward" } }, .profile = {}, .material = {} }, { "'Shading'", "twice", "pass" } }
		};
		for (const auto& c : cases) {
			const auto diags = fixFail(module, c.sources);
			ASSERT_EQ(diags.size(), 1uz);
			EXPECT_EQ(diags[0].severity, Severity::eError);
			for (const auto word : c.words) {
				EXPECT_TRUE(mentions(diags[0], word)) << diags[0].message;
			}
		}

		// resolveToggles validates the fixed set the same way
		const auto diags = resolveFail(module, { { "Shading", "Clustered" } });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_TRUE(mentions(diags[0], "'Clustered'")) << diags[0].message;
	}

	TEST(ResolveTest, PassValueWinsOverProfileWithWarning) {
		const auto module = parse(shading);
		const auto fixed = fixOk(
			module,
			ToggleSources { .pass = { { "Shading", "Deferred" } }, .profile = { { "Shading", "Forward" } }, .material = { { "Shading", "Deferred" } } }
		);
		EXPECT_EQ(fixed.toggles, (ToggleSet { { "Shading", "Deferred" } }));
		ASSERT_EQ(fixed.warnings.size(), 1uz);// the material agrees with the winning pass value
		EXPECT_TRUE(mentions(fixed.warnings[0], "profile")) << fixed.warnings[0].message;

		const auto materialOnly = fixOk(module, ToggleSources { .pass = {}, .profile = {}, .material = { { "Shading", "Deferred" } } });
		EXPECT_EQ(materialOnly.toggles, (ToggleSet { { "Shading", "Deferred" } }));
		EXPECT_TRUE(materialOnly.warnings.empty());
	}

	/* Rulings: resolving */

	TEST(ResolveTest, OverlappingVariantsAreError) {
		const auto module = parse(
			String(shading) + String(shadows) +
			"@when (Shading == Forward) fn F() -> f32 { return 1.0; }\n"
			"@when (Shadows == On) fn F() -> f32 { return 0.5; }\n"
			"@when (Shading == Deferred && Shadows == Off) fn F() -> f32 { return 0.25; }\n"
			"fn main() {\n\tfrag_depth = F();\n}\n"
		);
		const auto diags = resolveFail(module, { { "Shading", "Forward" }, { "Shadows", "On" } });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].message, "overlapping bodies for 'F()' at Shading=Forward, Shadows=On");

		const auto resolved = resolveOk(module, { { "Shading", "Forward" }, { "Shadows", "Off" } });
		EXPECT_EQ(functionNames(resolved), (Vector<String> { "F", "main" }));
		EXPECT_TRUE(hasLiteral(resolved, "1.0"));
		EXPECT_FALSE(hasLiteral(resolved, "0.5"));
		EXPECT_FALSE(hasLiteral(resolved, "0.25"));
	}

	TEST(ResolveTest, CallInSurvivingCodeWithoutBodyIsError) {
		const auto module = parse(String(shading) + "@when (Shading == Forward) fn F() -> f32 { return 1.0; }\nfn main() {\n\tfrag_depth = F();\n}\n");
		const auto diags = resolveFail(module, { { "Shading", "Deferred" } });
		ASSERT_EQ(diags.size(), 1uz);
		EXPECT_EQ(diags[0].message, "no body for 'F()' at Shading=Deferred");
		static_cast<void>(resolveOk(module, { { "Shading", "Forward" } }));

		// a conditional function nobody calls in that variant is simply dropped
		const auto guarded = parse(
			String(shading) + "@when (Shading == Forward) fn F() -> f32 { return 1.0; }\n"
			"fn main() {\n\t@when (Shading == Forward) frag_depth = F();\n}\n"
		);
		EXPECT_EQ(functionNames(resolveOk(guarded, { { "Shading", "Deferred" } })), (Vector<String> { "main" }));
	}

	TEST(MergeConstantTest, DifferentTypeOrValueIsDiagnosticNamingBothModules) {
		const auto base = parse("const K: f32 = 1.0;\nfn main() {}\n", "P");
		for (const auto* other : { "const K: u32 = 1u;\nfn main() {}\n", "const K: f32 = 2.0;\nfn main() {}\n" }) {
			const auto second = parse(other, "S");
			const Vector<const Module*> list { &base, &second };
			const auto merged = merge(list);
			ASSERT_FALSE(merged.has_value());
			EXPECT_TRUE(mentions(merged.error().front(), "'P'") && mentions(merged.error().front(), "'S'") && mentions(merged.error().front(), "'K'"))
				<< merged.error().front().message;
		}
	}

	TEST(MergeConstantTest, IdenticalConstantsCollapse) {
		const auto a = parse("const K: f32 = 1.0;\nfn helper() {}\n", "P");
		const auto b = parse("const K: f32 = 1.0;\nfn main() {}\n", "S");
		const Vector<const Module*> list { &a, &b };
		const auto merged = merge(list);
		ASSERT_TRUE(merged.has_value());
		EXPECT_EQ(merged->constants.size(), 1uz);
	}

	TEST(SliceTest, KeepsConstantUsedByKeptConstantInitializer) {
		const auto module = parse(
			"const BASE: f32 = 1.0;\nconst DERIVED: f32 = BASE * 2.0;\nconst UNRELATED: f32 = 3.0;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn main() {\n\tcolor = f32vec4(DERIVED);\n}\n"
		);
		const auto sliced = sliceChecked(module, { "color" });
		ASSERT_EQ(sliced.constants.size(), 2uz);
		EXPECT_EQ(sliced.constants[0].name, "BASE");
		EXPECT_EQ(sliced.constants[1].name, "DERIVED");
	}

	TEST(ResolveTest, MergedCallsResolveToSurvivingVariant) {
		const auto profile = parse(
			String(shading) +
			"@input (mode = forward) f32vec3 local_position;\n"
			"@input (mode = forward) f32vec4 gbuffer_position;\n"
			"@when (Shading == Forward) fn GetWorldPosition() -> f32vec4 { return f32vec4(local_position, 1.0); }\n"
			"@when (Shading == Deferred) fn GetWorldPosition() -> f32vec4 { return gbuffer_position; }\n",
			"P"
		);
		const auto shader = parse("fn GetWorldPosition() -> f32vec4;\nfn main() {\n\tposition = GetWorldPosition();\n}\n", "S", EntryKind::eVertex);
		const Vector<const Module*> list { &profile, &shader };
		const auto merged = merge(list);
		ASSERT_TRUE(merged.has_value());

		// the merge froze the call to the last variant (Deferred); Forward must re-resolve
		for (const auto& [value, input] : { std::pair { "Forward", "local_position" }, std::pair { "Deferred", "gbuffer_position" } }) {
			const auto resolved = resolveOk(*merged, { { "Shading", value } });
			EXPECT_EQ(functionNames(resolved), (Vector<String> { "GetWorldPosition", "main" }));
			const auto main = functionIndex(resolved, "main");
			const auto world = functionIndex(resolved, "GetWorldPosition");
			EXPECT_EQ(usageOf(resolved, main).calledFunctions, (Vector<u32> { world }));
			EXPECT_EQ(usageOf(resolved, world).readBindings, (Vector<u32> { bindingIndex(resolved, input) }));
			const std::array roots { main };
			EXPECT_EQ(reachableFunctions(resolved, roots), (Vector<u32> { world, main }));
		}
	}

	TEST(ResolveTest, BindingUsesReResolveToSurvivingVariant) {
		const auto module = parse(
			String(shading) +
			"@when (Shading == Forward) @input (mode = forward, id = \"light_forward\") f32vec3 light;\n"
			"@when (Shading == Deferred) @input (mode = forward, id = \"light_deferred\") f32vec3 light;\n"
			"@output (mode = forward, id = \"color\") f32vec3 color;\n"
			"fn main() {\n\tcolor = light;\n}\n"
		);
		const auto deferred = resolveOk(module, { { "Shading", "Deferred" } });
		EXPECT_EQ(bindingIds(deferred), (Vector<String> { "light_deferred", "color" }));
		const auto usage = usageOf(deferred, functionIndex(deferred, "main"));
		EXPECT_EQ(usage.readBindings, (Vector<u32> { bindingIndex(deferred, "light_deferred") }));
		EXPECT_EQ(usage.writtenBindings, (Vector<u32> { bindingIndex(deferred, "color") }));
	}

	TEST(ResolveTest, ResultIsCompactAndCanonical) {
		constexpr StringView live = "@when (Shading == Forward) fn F() -> f32 { return 1.0; }\n";
		const auto a = parse(String(shading) + "@when (Shading == Deferred) fn F() -> f32 { return 0.5; }\n" + String(live) + "fn main() {\n\tfrag_depth = F();\n}\n");
		// differs only in the dead variant, which also sits elsewhere in the arena
		const auto b = parse(
			String(shading) + String(live) + "@when (Shading == Deferred) fn F() -> f32 { return 0.25 * (2.0 + 0.5); }\n" +
			"fn main() {\n\tfrag_depth = F();\n}\n"
		);
		const ToggleSet forward { { "Shading", "Forward" } };
		const auto first = resolveOk(a, forward);
		EXPECT_TRUE(same(first, resolveOk(a, forward)));
		EXPECT_TRUE(same(first, resolveOk(b, forward)));
		// resolving a resolved module changes nothing
		EXPECT_TRUE(same(first, resolveOk(first, forward)));
	}

	TEST(ResolveTest, NestedWhenStatementsInline) {
		const auto module = parse(
			"toggle A : { Off, On } = Off;\ntoggle B : { Off, On } = Off;\n"
			"fn main() {\n"
			"\t@when (A == On) {\n"
			"\t\t@when (B == On) frag_depth = 1.0;\n"
			"\t\t@when (B == Off) frag_depth = 0.5;\n"
			"\t}\n"
			"\t@when (A == Off) frag_depth = 0.25;\n"
			"}\n"
		);
		const auto resolved = resolveOk(module, { { "A", "On" }, { "B", "Off" } });
		EXPECT_FALSE(std::ranges::any_of(resolved.stmts, [](ref<const Stmt> s_) { return s_.kind == StmtKind::eWhen; }));
		const auto& root = bodyRoot(resolved, "main");
		ASSERT_EQ(root.children.size(), 1uz);
		const auto& inner = resolved.stmts.at(root.children[0].index);
		ASSERT_EQ(inner.kind, StmtKind::eBlock);// the inlined `{ ... }` keeps its scope
		ASSERT_EQ(inner.children.size(), 1uz);
		EXPECT_EQ(resolved.stmts.at(inner.children[0].index).kind, StmtKind::eAssign);
		EXPECT_TRUE(hasLiteral(resolved, "0.5"));
		EXPECT_FALSE(hasLiteral(resolved, "1.0"));
		EXPECT_FALSE(hasLiteral(resolved, "0.25"));
	}

	TEST(ResolveTest, DegenerateInputIsSafe) {
		EXPECT_TRUE(same(resolveOk(Module {}, {}), Module {}));
		const auto declsOnly = parse(String(shading) + "@input (mode = forward) f32 x;\n");
		EXPECT_TRUE(same(resolveOk(declsOnly, {}), declsOnly));

		// hand-built: out-of-range refs, a statement cycle and a `when` with an else arm
		Module hand {};
		hand.toggles.emplace_back(Toggle { "A", { "Off", "On" }, "Off" });
		auto& lit = hand.exprs.emplace_back();
		lit.name = "1.0";
		lit.operands.emplace_back(ExprRef { 99u });
		auto& call = hand.exprs.emplace_back();
		call.kind = ExprKind::eCall;
		call.name = "main";
		call.function = FunctionRef { 42u };
		hand.stmts.resize(5uz);
		hand.stmts[0].kind = StmtKind::eBlock;
		hand.stmts[0].children = { StmtRef { 1u }, StmtRef { 0u }, StmtRef { 77u }, StmtRef { 3u } };
		hand.stmts[1].kind = StmtKind::eWhen;
		hand.stmts[1].when = Condition::equals("A", "On");
		hand.stmts[1].children = { StmtRef { 2u }, StmtRef { 4u } };
		hand.stmts[2].lhs = ExprRef { 0u };
		hand.stmts[3].lhs = ExprRef { 1u };
		hand.stmts[3].rhs = ExprRef { 1234u };
		hand.stmts[4].kind = StmtKind::eReturn;
		hand.functions.emplace_back(function("main", 0u));
		hand.functions.back().ret = TypeRef { 5u };
		hand.functions.emplace_back(function("lost", 500u));

		const auto off = resolveOk(hand, { { "A", "Off" } });
		// the else arm (return) replaced the `when`
		const auto& root = bodyRoot(off, "main");
		ASSERT_FALSE(root.children.empty());
		EXPECT_EQ(off.stmts.at(root.children[0].index).kind, StmtKind::eReturn);
		static_cast<void>(resolveOk(hand, { { "A", "On" } }));
	}

	/* Rulings: slicing */

	TEST(SliceTest, RemovesStatementsWritingDroppedOutputs) {
		const auto module = parse(
			"@input (mode = forward) f32vec3 normal;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"@output (mode = forward, id = \"normal_out\") f32vec3 normal_out;\n"
			"fn Write() {\n\tcolor = f32vec4(1.0);\n\tnormal_out = normal;\n}\n"
			"fn Tint() -> f32vec4 { return f32vec4(0.5); }\n"
			"fn main() {\n\tlet unused = Tint();\n\tWrite();\n\tTint();\n\tcolor = unused;\n\tcolor.x = 1.0;\n}\n"
		);
		const auto sliced = sliceChecked(module, { "normal_out" });
		expectCompact(sliced);
		EXPECT_EQ(functionNames(sliced), (Vector<String> { "Write", "main" }));
		EXPECT_EQ(bindingIds(sliced), (Vector<String> { "normal", "normal_out" }));
		EXPECT_EQ(bodyRoot(sliced, "Write").children.size(), 1uz);
		EXPECT_EQ(bodyRoot(sliced, "main").children.size(), 1uz);// Write(): it writes a kept output
		EXPECT_EQ(usageOf(sliced, functionIndex(sliced, "Write")).writtenBindings, (Vector<u32> { bindingIndex(sliced, "normal_out") }));
	}

	TEST(SliceTest, StatementsCallingAKeptOutputWriterStay) {
		constexpr StringView decls =
			"@input (mode = forward) f32vec3 normal;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"@output (mode = forward, id = \"normal_out\") f32vec3 normal_out;\n"
			"fn Shade() -> f32vec4 { normal_out = normal; return f32vec4(1.0); }\n"
			"fn WriteN() -> bool { normal_out = normal; return true; }\n"
			"fn Get() -> f32vec4 { return Shade(); }\n";
		struct Case {
			StringView main;
			StringView writer;
		};
		const std::array cases {
			Case { "fn main() {\n\tcolor = Shade();\n}\n", "Shade" },// a write to a dropped output
			Case { "fn main() {\n\tif (WriteN()) {\n\t\tcolor = f32vec4(1.0);\n\t}\n}\n", "WriteN" },// a condition
			Case { "fn main() {\n\tcolor = Get();\n}\n", "Shade" },// through a return
			Case { "fn main() {\n\tShade();\n}\n", "Shade" }// a call statement
		};
		for (const auto& c : cases) {
			const auto module = parse(String(decls) + String(c.main));
			const auto sliced = sliceChecked(module, { "normal_out" });
			expectCompact(sliced);
			EXPECT_EQ(bodyRoot(sliced, "main").children.size(), 1uz) << c.main;
			const auto writer = functionIndex(sliced, c.writer);
			ASSERT_NE(writer, unset) << c.main;
			EXPECT_TRUE(std::ranges::contains(usageOf(sliced, writer).writtenBindings, bindingIndex(sliced, "normal_out"))) << c.main;
		}
	}

	TEST(SliceTest, LocalsFeedingKeptCodeStay) {
		const auto module = parse(
			"@input (mode = forward) f32 bias;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
			"fn main() {\n"
			"\tlet d = bias;\n"
			"\tlet c = d * 2.0;\n"
			"\tif (d > 0.0) {\n\t\tcolor = f32vec4(c);\n\t}\n"
			"\td = d + 0.5;\n"
			"\tfrag_depth = d;\n"
			"}\n"
		);
		const auto sliced = sliceChecked(module, { "depth" });
		expectCompact(sliced);
		// `let c` and the `if` that only wrote color are gone; d's definitions stay
		const auto& root = bodyRoot(sliced, "main");
		ASSERT_EQ(root.children.size(), 3uz);
		EXPECT_EQ(sliced.stmts.at(root.children[0].index).kind, StmtKind::eDeclareLocal);
		EXPECT_EQ(sliced.stmts.at(root.children[0].index).name, "d");
		EXPECT_EQ(sliced.stmts.at(root.children[1].index).kind, StmtKind::eAssign);
		EXPECT_EQ(sliced.stmts.at(root.children[2].index).kind, StmtKind::eAssign);
		EXPECT_EQ(bindingIds(sliced), (Vector<String> { "bias", "depth" }));
	}

	TEST(SliceTest, NestedWhenInsideKeptFunction) {
		const auto module = parse(
			String(shadows) + String(litSource.substr(0, litSource.find("fn main"))) +
			"fn main() {\n"
			"\t@when (Shadows == On) {\n\t\tlet s = ShadowFactor();\n\t\tcolor = f32vec4(s);\n\t}\n"
			"\tfrag_depth = 0.5;\n"
			"}\n"
		);
		const auto on = resolveOk(module, { { "Shadows", "On" } });
		const auto depth = sliceChecked(on, { "depth" });
		expectCompact(depth);
		EXPECT_EQ(functionNames(depth), (Vector<String> { "main" }));
		EXPECT_EQ(bindingIds(depth), (Vector<String> { "depth" }));
		EXPECT_EQ(bodyRoot(depth, "main").children.size(), 1uz);

		// sliced before resolving, the `when` stays and keeps what it uses
		const auto unresolved = sliceChecked(module, { "color" });
		EXPECT_EQ(functionNames(unresolved), (Vector<String> { "ShadowFactor", "main" }));
		EXPECT_EQ(bindingIds(unresolved), (Vector<String> { "uv", "shadow_map", "shadow_sampler", "color" }));
		const auto& root = bodyRoot(unresolved, "main");
		ASSERT_EQ(root.children.size(), 2uz);
		EXPECT_EQ(unresolved.stmts.at(root.children[0].index).kind, StmtKind::eWhen);
	}

	TEST(SliceTest, DegenerateInputIsSafe) {
		EXPECT_TRUE(same(sliceChecked(Module {}, { "depth" }), Module {}));

		// unknown ids are no error (Task 11 validates them against the interface): every output is dropped
		const auto module = parse(litSource);
		const auto unknown = sliceChecked(module, { "nope" });
		EXPECT_EQ(functionNames(unknown), (Vector<String> { "Depth", "main" }));
		EXPECT_EQ(bindingIds(unknown), (Vector<String> { "depth_bias" }));

		// no entry: only the required outputs remain
		const auto noMain = parse(
			"@input (mode = forward) f32 x;\n@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"fn Helper() -> f32 { return x; }\n"
		);
		const auto sliced = sliceChecked(noMain, { "color" });
		EXPECT_TRUE(sliced.functions.empty());
		EXPECT_TRUE(sliced.exprs.empty());
		EXPECT_EQ(bindingIds(sliced), (Vector<String> { "color" }));

		// hand-built: out-of-range refs and a statement cycle
		Module hand {};
		auto& out = hand.bindings.emplace_back();
		out.id = "out";
		out.type = TypeRef { 9u };
		out.input = false;
		out.localName = "out";
		auto& target = hand.exprs.emplace_back();
		target.kind = ExprKind::eNameRef;
		target.name = "out";
		target.binding = BindingRef { 0u };
		auto& bad = hand.exprs.emplace_back();
		bad.kind = ExprKind::eField;
		bad.operands = { ExprRef { 1u }, ExprRef { 50u } };// cycle onto itself and out of range
		hand.stmts.resize(3uz);
		hand.stmts[0].kind = StmtKind::eBlock;
		hand.stmts[0].children = { StmtRef { 1u }, StmtRef { 0u }, StmtRef { 2u }, StmtRef { 8u } };
		hand.stmts[1].kind = StmtKind::eAssign;
		hand.stmts[1].lhs = ExprRef { 0u };
		hand.stmts[1].rhs = ExprRef { 1u };
		hand.stmts[2].kind = StmtKind::eAssign;
		hand.stmts[2].lhs = ExprRef { 1u };
		hand.functions.emplace_back(function("main", 0u));
		static_cast<void>(sliceChecked(hand, { "out" }));
		static_cast<void>(sliceChecked(hand, { "other" }));
	}

	TEST(SliceTest, KeepsLoopFeedingOutput) {
		const auto module = parse(
			"@input (mode = forward) f32 bias;\n"
			"@output (mode = forward, id = \"color\") f32vec4 color;\n"
			"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
			"fn main() {\n\tlet a = 0.0;\n\tfor (let i = 0u; i < 4u; ++i) { a += bias; }\n\tfrag_depth = a;\n}\n"
		);
		const auto sliced = sliceChecked(module, { "depth" });
		expectCompact(sliced);
		EXPECT_TRUE(std::ranges::any_of(sliced.stmts, [](ref<const Stmt> s_) { return s_.kind == StmtKind::eFor; }));
	}

	TEST(SliceTest, DropsDeadLoop) {
		const auto module = parse(
			"@input (mode = forward) f32 bias;\n"
			"@output (mode = forward, id = \"color\") f32 color;\n"
			"@output (mode = forward, id = \"depth\") implicit f32 depth;\n"
			"fn main() {\n\tfor (let i = 0u; i < 4u; ++i) { color += bias; }\n\tfrag_depth = 1.0;\n}\n"
		);
		const auto sliced = sliceChecked(module, { "depth" });
		expectCompact(sliced);
		EXPECT_FALSE(std::ranges::any_of(sliced.stmts, [](ref<const Stmt> s_) { return s_.kind == StmtKind::eFor; }));
	}
}
