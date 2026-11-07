#include "Merge.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <utility>

#include "Typecheck.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

/*
 * Every remap and check below destructures the IR structs with structured bindings, so adding a member to Expr,
 * Stmt, TypeDesc, BindingDecl, FunctionDecl or Module stops this unit from compiling until the merge handles it.
 * Refs are remapped independent of `kind`, so a new kind that reuses the existing members needs no change here.
 */

namespace {
	constexpr u32 unset = ~0uL;

	[[nodiscard]] ir::Diagnostic error(String message_) {
		return ir::Diagnostic { .severity = ir::Severity::eError, .message = std::move(message_) };
	}

	[[nodiscard]] String quoted(StringView text_) {
		return "'" + String(text_) + "'";
	}

	/* Equality */

	[[nodiscard]] bool sameCondition(ref<const ir::Condition> a_, ref<const ir::Condition> b_) {
		// iterative: hand-built conditions may nest deeper than the parser allows
		Vector<std::pair<const ir::Condition*, const ir::Condition*>> work { { &a_, &b_ } };
		while (not work.empty()) {
			const auto [a, b] = work.back();
			work.pop_back();
			if (a->kind != b->kind || a->toggle != b->toggle || a->value != b->value || a->children.size() != b->children.size()) {
				return false;
			}
			for (auto i = 0uz; i < a->children.size(); ++i) {
				work.emplace_back(&a->children[i], &b->children[i]);
			}
		}
		return true;
	}

	/* all members, so the parser's defaults for irrelevant ones keep equal types equal */
	[[nodiscard]] bool sameType(ref<const ir::TypeDesc> a_, ref<const ir::TypeDesc> b_) {
		const auto& [kind, scalar, bits, n, m, element, count, name, fields, dim] = a_;
		return kind == b_.kind && scalar == b_.scalar && bits == b_.bits && n == b_.n && m == b_.m && element == b_.element &&
			count == b_.count && name == b_.name && dim == b_.dim &&
			std::ranges::equal(
				fields,
				b_.fields,
				[](ref<const ir::FieldDesc> x_, ref<const ir::FieldDesc> y_) {
					const auto& [fieldName, fieldType] = x_;
					return fieldName == y_.name && fieldType == y_.type;
				}
			);
	}

	/* Type names for diagnostics, in `.asl` spelling */

	[[nodiscard]] String describe(ref<const Vector<ir::TypeDesc>> types_, ir::TypeRef ref_) {
		return ir::describeType(types_, ref_);
	}

	[[nodiscard]] StringView className(ir::BindClass class_) {
		constexpr std::array<StringView, 6> names { "none", "uniform", "storage", "texture", "sampler", "push" };
		const auto index = static_cast<u32>(class_);
		return index < names.size() ? names[index] : "unknown";
	}

	/* Input validation: every ref in range (sentinel allowed), types refer only to earlier types (no cycles) */

	[[nodiscard]] Opt<String> invalidRef(ref<const ir::Module> module_) {
		const auto& [name, entry, toggles, types, bindings, functions, exprs, stmts] = module_;
		Opt<String> found = None;
		const auto check = [&](StringView owner_, size_t at_, StringView kind_, u32 index_, size_t limit_) {
			if (not found.has_value() && index_ != unset && index_ >= limit_) {
				found = String(owner_) + " " + std::to_string(at_) + " refers to out of range " + String(kind_) + " " +
					std::to_string(index_);
			}
		};

		for (auto i = 0uz; i < types.size(); ++i) {
			check("type", i, "type (earlier types only)", types[i].element.index, i);
			for (const auto& field : types[i].fields) {
				check("type", i, "type (earlier types only)", field.type.index, i);
			}
		}
		for (auto i = 0uz; i < bindings.size(); ++i) {
			check("binding", i, "type", bindings[i].type.index, types.size());
		}
		for (auto i = 0uz; i < functions.size(); ++i) {
			const auto& fn = functions[i];
			for (const auto& param : fn.params) {
				check("function", i, "type", param.type.index, types.size());
			}
			check("function", i, "type", fn.ret.index, types.size());
			if (fn.body.has_value()) {
				check("function", i, "stmt", fn.body->root.index, stmts.size());
			}
		}
		for (auto i = 0uz; i < exprs.size(); ++i) {
			const auto& e = exprs[i];
			check("expr", i, "binding", e.binding.index, bindings.size());
			check("expr", i, "function", e.function.index, functions.size());
			check("expr", i, "type", e.type.index, types.size());
			for (const auto& op : e.operands) {
				check("expr", i, "expr", op.index, exprs.size());
			}
		}
		for (auto i = 0uz; i < stmts.size(); ++i) {
			const auto& s = stmts[i];
			check("stmt", i, "type", s.type.index, types.size());
			check("stmt", i, "expr", s.lhs.index, exprs.size());
			check("stmt", i, "expr", s.rhs.index, exprs.size());
			for (const auto& child : s.children) {
				check("stmt", i, "stmt", child.index, stmts.size());
			}
		}
		static_cast<void>(name);
		static_cast<void>(entry);
		static_cast<void>(toggles);
		return found;
	}

	/* Remapping one module's refs into the result */

	struct Remap {
		Vector<u32> types;
		Vector<u32> bindings;
		u32 exprOffset = 0uL;
		u32 stmtOffset = 0uL;

		[[nodiscard]] ir::TypeRef type(ir::TypeRef ref_) const {
			return ref_.index == unset ? ref_ : ir::TypeRef { types[ref_.index] };
		}

		[[nodiscard]] ir::BindingRef binding(ir::BindingRef ref_) const {
			return ref_.index == unset ? ref_ : ir::BindingRef { bindings[ref_.index] };
		}

		[[nodiscard]] ir::ExprRef expr(ir::ExprRef ref_) const {
			return ref_.index == unset ? ref_ : ir::ExprRef { ref_.index + exprOffset };
		}

		[[nodiscard]] ir::StmtRef stmt(ir::StmtRef ref_) const {
			return ref_.index == unset ? ref_ : ir::StmtRef { ref_.index + stmtOffset };
		}
	};

	[[nodiscard]] ir::TypeDesc remapType(ir::TypeDesc type_, ref<const Remap> remap_) {
		auto& [kind, scalar, bits, n, m, element, count, name, fields, dim] = type_;
		element = remap_.type(element);
		for (auto& [fieldName, fieldType] : fields) {
			fieldType = remap_.type(fieldType);
		}
		return type_;
	}

	[[nodiscard]] ir::FunctionDecl remapFunction(ir::FunctionDecl fn_, ref<const Remap> remap_) {
		auto& [name, params, ret, body, redefineAllowed, when] = fn_;
		for (auto& [paramName, paramType, paramKind] : params) {
			paramType = remap_.type(paramType);
		}
		ret = remap_.type(ret);
		if (body.has_value()) {
			body->root = remap_.stmt(body->root);
		}
		return fn_;
	}

	/* shortcut: a name with several `@when` variants resolves to the last one (Task 5's rule, in merged order), so a call
	 * cannot pick its variant; upgrade when Task 7 slices by toggle values and resolves calls per variant. */
	[[nodiscard]] ir::FunctionRef resolveCall(
		ref<const Vector<ir::FunctionDecl>> functions_,
		StringView name_,
		ref<const Vector<ir::ParamDecl>> source_,
		ref<const Remap> remap_
	) {
		const auto matches = [&](ref<const ir::FunctionDecl> fn_) {
			return fn_.name == name_ && std::ranges::equal(
				fn_.params,
				source_,
				[&](ref<const ir::ParamDecl> x_, ref<const ir::ParamDecl> y_) { return x_.type == remap_.type(y_.type); }
			);
		};
		for (auto i = functions_.size(); i > 0uz; --i) {
			if (matches(functions_[i - 1uz])) {
				return ir::FunctionRef { static_cast<u32>(i - 1uz) };
			}
		}
		return {};
	}

	[[nodiscard]] ir::Expr remapExpr(
		ir::Expr expr_,
		ref<const Remap> remap_,
		ref<const Vector<ir::FunctionDecl>> functions_,
		ref<const Vector<ir::FunctionDecl>> source_
	) {
		auto& [kind, name, binding, function, local, type, operands, loc] = expr_;
		/* shortcut: a binding use keeps its parse-time binding (first localName match) mapped through the id collapse,
		 * so same-named bindings with different ids under different @when stay resolved to the first; upgrade when Task 7
		 * slices by toggle values and can re-resolve names per variant. */
		binding = remap_.binding(binding);
		// calls re-resolve by name: the frozen index may name a dropped forward declaration or a replaced body
		if (kind == ir::ExprKind::eCall && function.index < source_.size()) {
			function = resolveCall(functions_, name, source_[function.index].params, remap_);
		} else {
			function = ir::FunctionRef {};
		}
		type = remap_.type(type);
		for (auto& op : operands) {
			op = remap_.expr(op);
		}
		static_cast<void>(local);
		static_cast<void>(loc);
		return expr_;
	}

	[[nodiscard]] ir::Stmt remapStmt(ir::Stmt stmt_, ref<const Remap> remap_) {
		auto& [kind, name, type, lhs, rhs, mutable_, children, when, loc] = stmt_;
		type = remap_.type(type);
		lhs = remap_.expr(lhs);
		rhs = remap_.expr(rhs);
		for (auto& child : children) {
			child = remap_.stmt(child);
		}
		static_cast<void>(kind);
		static_cast<void>(mutable_);
		return stmt_;
	}

	/* Declaration merging */

	struct Merger {
		std::span<const ir::Module* const> modules;
		Vector<Remap> remaps;
		ir::Module result {};
		ir::Diagnostics diags {};

		[[nodiscard]] ref<const String> moduleName(u32 module_) const {
			return modules[module_]->name;
		}

		void types(u32 module_) {
			auto& remap = remaps[module_];
			for (const auto& type : modules[module_]->types) {
				// refs point to earlier types (validated), so they are mapped already
				auto mapped = remapType(type, remap);
				const auto it = std::ranges::find_if(result.types, [&](ref<const ir::TypeDesc> t_) { return sameType(t_, mapped); });
				remap.types.emplace_back(static_cast<u32>(it - result.types.begin()));
				if (it == result.types.end()) {
					result.types.emplace_back(std::move(mapped));
				}
			}
		}

		/* same value set and default collapse into the first declaration */
		void toggles(u32 module_, ref<Vector<u32>> origins_) {
			for (const auto& toggle : modules[module_]->toggles) {
				const auto& [name, values, defaultValue] = toggle;
				const auto it = std::ranges::find(result.toggles, name, &ir::Toggle::name);
				if (it == result.toggles.end()) {
					result.toggles.emplace_back(toggle);
					origins_.emplace_back(module_);
					continue;
				}

				const auto& first = moduleName(origins_[static_cast<size_t>(it - result.toggles.begin())]);
				auto lhs = it->values;
				auto rhs = values;
				std::ranges::sort(lhs);
				std::ranges::sort(rhs);
				if (lhs != rhs) {
					diags.emplace_back(
						error(
							"toggle " + quoted(name) + " declared with different values in " + quoted(first) + " and " +
							quoted(moduleName(module_))
						)
					);
				} else if (it->defaultValue != defaultValue) {
					diags.emplace_back(
						error(
							"toggle " + quoted(name) + " defaults to " + quoted(it->defaultValue) + " in " + quoted(first) + " and " +
							quoted(defaultValue) + " in " + quoted(moduleName(module_))
						)
					);
				}
			}
		}

		/* first difference of two declarations of one id, or empty; `a_` is in the result already */
		[[nodiscard]] String bindingConflict(ref<const ir::BindingDecl> a_, ref<const ir::BindingDecl> b_, ref<const String> first_, ref<const String> second_) const {
			const auto in = [](ref<const String> module_) {
				return " in " + quoted(module_);
			};
			const auto prefix = "binding " + quoted(a_.id) + " declared ";
			if (a_.type != b_.type) {
				return prefix + "as " + describe(result.types, a_.type) + in(first_) + " and " + describe(result.types, b_.type) + in(second_);
			}
			if (a_.mode != b_.mode) {
				const auto mode = [](ir::BindMode mode_) {
					return String(mode_ == ir::BindMode::eBind ? "bind" : "forward");
				};
				return prefix + "with mode " + mode(a_.mode) + in(first_) + " and " + mode(b_.mode) + in(second_);
			}
			if (a_.bindClass != b_.bindClass) {
				return prefix + "with class " + String(className(a_.bindClass)) + in(first_) + " and " +
					String(className(b_.bindClass)) + in(second_);
			}
			if (a_.input != b_.input) {
				const auto direction = [](bool input_) {
					return String(input_ ? "input" : "output");
				};
				return prefix + "as " + direction(a_.input) + in(first_) + " and " + direction(b_.input) + in(second_);
			}
			if (a_.access != b_.access) {
				const auto access = [](ir::Access access_) {
					return String(access_ == ir::Access::eRead ? "read" : (access_ == ir::Access::eWrite ? "write" : "read_write"));
				};
				return prefix + "with access " + access(a_.access) + in(first_) + " and " + access(b_.access) + in(second_);
			}
			if (a_.implicit != b_.implicit) {
				// not OR-ed: an implicit entry emits no variable, while the other module's body may use one
				return prefix + "implicit" + in(a_.implicit ? first_ : second_) + " and not" + in(a_.implicit ? second_ : first_);
			}
			return {};
		}

		/* identity is the string id (a `mode = bind` input without `id` defaulted to its local name at parse, so two such
		 * inputs of the same local name collapse too); collapsed conditions are OR-ed */
		void bindings(u32 module_, ref<Vector<u32>> origins_) {
			auto& remap = remaps[module_];
			for (const auto& binding : modules[module_]->bindings) {
				auto mapped = binding;
				auto& [id, type, mode, bindClass, implicit, input, when, localName, access] = mapped;
				type = remap.type(type);

				const auto it = std::ranges::find(result.bindings, id, &ir::BindingDecl::id);
				const auto index = static_cast<u32>(it - result.bindings.begin());
				remap.bindings.emplace_back(index);
				if (it == result.bindings.end()) {
					result.bindings.emplace_back(std::move(mapped));
					origins_.emplace_back(module_);
					continue;
				}

				if (auto conflict = bindingConflict(*it, mapped, moduleName(origins_[index]), moduleName(module_)); not conflict.empty()) {
					diags.emplace_back(error(std::move(conflict)));
				} else if (not sameCondition(it->when, when) && it->when.kind != ir::ConditionKind::eTrue) {
					it->when = when.kind == ir::ConditionKind::eTrue ? ir::Condition::always() : ir::Condition::any({ it->when, when });
				}
				static_cast<void>(mode);
				static_cast<void>(bindClass);
				static_cast<void>(implicit);
				static_cast<void>(input);
				static_cast<void>(localName);
				static_cast<void>(access);
			}
		}

		struct Variant {
			u32 module;
			u32 index;
		};

		struct Named {
			String name;
			Variant first;
			/* one surviving body per distinct `@when` condition, in first-appearance order */
			Vector<Variant> bodies;
		};

		[[nodiscard]] ref<const ir::FunctionDecl> decl(Variant at_) const {
			return modules[at_.module]->functions[at_.index];
		}

		[[nodiscard]] bool sameSignature(Variant a_, Variant b_) const {
			const auto& a = decl(a_);
			const auto& b = decl(b_);
			const auto& ra = remaps[a_.module];
			const auto& rb = remaps[b_.module];
			return ra.type(a.ret) == rb.type(b.ret) &&
				std::ranges::equal(
					a.params,
					b.params,
					[&](ref<const ir::ParamDecl> x_, ref<const ir::ParamDecl> y_) { return ra.type(x_.type) == rb.type(y_.type); }
				);
		}

		void functions() {
			Vector<Named> names {};
			for (auto m = 0uz; m < modules.size(); ++m) {
				for (auto i = 0uz; i < modules[m]->functions.size(); ++i) {
					const Variant at { static_cast<u32>(m), static_cast<u32>(i) };
					const auto& fn = decl(at);
					const auto signature = ir::signatureOf(*modules[m], fn);
					auto named = std::ranges::find(names, signature, &Named::name);
					if (named == names.end()) {
						names.emplace_back(Named { signature, at, {} });
						named = names.end() - 1;
					} else if (not sameSignature(named->first, at)) {
						diags.emplace_back(
							error(
								"signature of function " + quoted(fn.name) + " differs between " + quoted(moduleName(named->first.module)) +
								" and " + quoted(moduleName(at.module))
							)
						);
					}

					if (not fn.body.has_value()) {
						continue;
					}
					/* shortcut: variants match by structurally equal conditions only; overlapping but unequal ones are not
					 * caught here, upgrade when Task 7 checks exhaustiveness and overlap per toggle combination. */
					const auto variant = std::ranges::find_if(
						named->bodies,
						[&](Variant v_) { return sameCondition(decl(v_).when, fn.when); }
					);
					if (variant == named->bodies.end()) {
						named->bodies.emplace_back(at);
					} else if (decl(*variant).redefineAllowed) {
						*variant = at;// the body being replaced allowed it; the last wins
					} else {
						diags.emplace_back(
							error(
								"duplicate body for function " + quoted(signature) + " in modules " + quoted(moduleName(variant->module)) +
								" and " + quoted(moduleName(at.module))
							)
						);
					}
				}
			}

			for (const auto& named : names) {
				if (named.bodies.empty()) {
					diags.emplace_back(
						error("missing body for function " + quoted(named.name) + " declared in " + quoted(moduleName(named.first.module)))
					);
				}
				for (const auto at : named.bodies) {
					result.functions.emplace_back(remapFunction(decl(at), remaps[at.module]));
				}
			}
		}

		void arenas() {
			for (auto m = 0uz; m < modules.size(); ++m) {
				const auto& remap = remaps[m];
				for (const auto& e : modules[m]->exprs) {
					result.exprs.emplace_back(remapExpr(e, remap, result.functions, modules[m]->functions));
				}
				for (const auto& s : modules[m]->stmts) {
					result.stmts.emplace_back(remapStmt(s, remap));
				}
			}
		}

		void run() {
			auto& [name, entry, toggleList, typeList, bindingList, functionList, exprList, stmtList] = result;
			name = modules.back()->name;
			entry = modules.back()->entry;

			Vector<u32> toggleOrigins {};
			Vector<u32> bindingOrigins {};
			u32 exprs = 0uL;
			u32 stmts = 0uL;
			remaps.resize(modules.size());
			for (auto m = 0uz; m < modules.size(); ++m) {
				const auto module = static_cast<u32>(m);
				types(module);
				toggles(module, toggleOrigins);
				bindings(module, bindingOrigins);
				remaps[m].exprOffset = exprs;
				remaps[m].stmtOffset = stmts;
				exprs += static_cast<u32>(modules[m]->exprs.size());
				stmts += static_cast<u32>(modules[m]->stmts.size());
			}
			functions();
			if (diags.empty()) {
				arenas();
			}
			static_cast<void>(toggleList);
			static_cast<void>(typeList);
			static_cast<void>(bindingList);
			static_cast<void>(functionList);
			static_cast<void>(exprList);
			static_cast<void>(stmtList);
		}
	};
}

std::expected<ir::Module, ir::Diagnostics> ir::merge(std::span<const Module* const> ordered_) {
	if (ordered_.empty()) {
		return std::unexpected(Diagnostics { error("merge: no modules to merge") });
	}

	Diagnostics diags {};
	for (auto i = 0uz; i < ordered_.size(); ++i) {
		if (ordered_[i] == nullptr) {
			diags.emplace_back(error("merge: module #" + std::to_string(i) + " is null"));
		} else if (auto invalid = invalidRef(*ordered_[i]); invalid.has_value()) {
			diags.emplace_back(error("module " + quoted(ordered_[i]->name) + ": " + std::move(*invalid)));
		}
	}
	if (not diags.empty()) {
		return std::unexpected(std::move(diags));
	}

	Merger merger { ordered_, {} };
	merger.run();
	if (not merger.diags.empty()) {
		return std::unexpected(std::move(merger.diags));
	}
	return std::move(merger.result);
}
