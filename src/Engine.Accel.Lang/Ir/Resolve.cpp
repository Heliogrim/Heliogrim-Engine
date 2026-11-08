#include "Resolve.hpp"

#include <algorithm>
#include <array>
#include <utility>

#include "Compact.hpp"
#include "Typecheck.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	constexpr u32 unset = ~0uL;

	[[nodiscard]] ir::Diagnostic error(String message_) {
		return ir::Diagnostic { .severity = ir::Severity::eError, .message = std::move(message_) };
	}

	[[nodiscard]] String quoted(StringView text_) {
		return "'" + String(text_) + "'";
	}

	/* Toggle fixing */

	using Labels = std::array<StringView, 3>;

	[[nodiscard]] const String* valueOf(ref<const ir::ToggleSet> set_, ref<const String> toggle_) {
		const auto it = std::ranges::find(set_, toggle_, &std::pair<String, String>::first);
		return it == set_.end() ? nullptr : &it->second;
	}

	void validate(ref<const ir::Module> mod_, ref<const ir::ToggleSet> set_, StringView label_, ref<ir::Diagnostics> diags_) {
		for (auto i = 0uz; i < set_.size(); ++i) {
			const auto& [name, value] = set_[i];
			const auto toggle = std::ranges::find(mod_.toggles, name, &ir::Toggle::name);
			if (toggle == mod_.toggles.end()) {
				diags_.emplace_back(error("unknown toggle " + quoted(name) + " in " + String(label_)));
			} else if (std::ranges::find(toggle->values, value) == toggle->values.end()) {
				diags_.emplace_back(error("toggle " + quoted(name) + " has no value " + quoted(value) + " (set by " + String(label_) + ")"));
			}
			if (std::ranges::find(set_.begin(), set_.begin() + static_cast<ptrdiff_t>(i), name, &std::pair<String, String>::first) !=
				set_.begin() + static_cast<ptrdiff_t>(i)) {
				diags_.emplace_back(error("toggle " + quoted(name) + " set twice by " + String(label_)));
			}
		}
	}

	[[nodiscard]] std::expected<ir::ResolveResult, ir::Diagnostics> fix(
		ref<const ir::Module> mod_,
		ref<const ir::ToggleSources> sources_,
		ref<const Labels> labels_
	) {
		ir::Diagnostics diags {};
		validate(mod_, sources_.pass, labels_[0], diags);
		validate(mod_, sources_.profile, labels_[1], diags);
		validate(mod_, sources_.material, labels_[2], diags);
		if (not diags.empty()) {
			return std::unexpected(std::move(diags));
		}

		ir::ResolveResult result {};
		const auto warn = [&](String message_) {
			result.warnings.emplace_back(ir::Diagnostic { .severity = ir::Severity::eWarning, .message = std::move(message_) });
		};
		for (const auto& toggle : mod_.toggles) {
			const auto* pass = valueOf(sources_.pass, toggle.name);
			const auto* profile = valueOf(sources_.profile, toggle.name);
			const auto* material = valueOf(sources_.material, toggle.name);
			const auto* fixed = pass != nullptr ? pass : profile;
			if (pass != nullptr && profile != nullptr && *pass != *profile) {
				warn(
					String(labels_[0]) + " sets toggle " + quoted(toggle.name) + " to " + quoted(*pass) + " over the " +
					String(labels_[1]) + " value " + quoted(*profile)
				);
			}
			if (material != nullptr && fixed != nullptr && *material != *fixed) {
				warn(
					String(labels_[2]) + " sets toggle " + quoted(toggle.name) + " to " + quoted(*material) + ", but " +
					String(labels_[pass != nullptr ? 0 : 1]) + " fixes it to " + quoted(*fixed)
				);
			}
			const auto* value = fixed != nullptr ? fixed : material;
			result.toggles.emplace_back(toggle.name, value != nullptr ? *value : toggle.defaultValue);
		}
		std::ranges::sort(result.toggles, {}, &std::pair<String, String>::first);
		return result;
	}

	/* Resolving */

	/* `A=x, B=y` over `order_` (declaration order), values from `set_` */
	[[nodiscard]] String combination(ref<const Vector<ir::Toggle>> order_, ref<const ir::ToggleSet> set_) {
		String text {};
		for (const auto& toggle : order_) {
			const auto* value = valueOf(set_, toggle.name);
			text += (text.empty() ? "" : ", ") + toggle.name + "=" + (value != nullptr ? *value : String {});
		}
		return text;
	}

	/* the declared toggles some arm names, declaration order */
	[[nodiscard]] Vector<ir::Toggle> named(ref<const Vector<ir::Toggle>> toggles_, ref<const Vector<ir::Condition>> arms_) {
		Vector<String> names {};
		Vector<const ir::Condition*> work {};
		for (const auto& arm : arms_) {
			work.emplace_back(&arm);
		}
		while (not work.empty()) {
			const auto* cond = work.back();
			work.pop_back();
			if (cond->kind == ir::ConditionKind::eEquals) {
				names.emplace_back(cond->toggle);
			}
			for (const auto& child : cond->children) {
				work.emplace_back(&child);
			}
		}
		Vector<ir::Toggle> result {};
		for (const auto& toggle : toggles_) {
			if (std::ranges::find(names, toggle.name) != names.end()) {
				result.emplace_back(toggle);
			}
		}
		return result;
	}

	/* function names in first-appearance order with their declarations */
	[[nodiscard]] Vector<std::pair<String, Vector<u32>>> byName(ref<const ir::Module> mod_) {
		Vector<std::pair<String, Vector<u32>>> names {};
		for (auto i = 0uz; i < mod_.functions.size(); ++i) {
			const auto signature = ir::signatureOf(mod_, mod_.functions[i]);
			auto it = std::ranges::find(names, signature, &std::pair<String, Vector<u32>>::first);
			if (it == names.end()) {
				it = names.insert(names.end(), { signature, {} });
			}
			it->second.emplace_back(static_cast<u32>(i));
		}
		return names;
	}

	/* shortcut: enumerates every combination of the toggles the arms name (exponential in that count, fine for a
	 * handful), upgrade to a per-toggle cover check when shaders name many toggles in one function's variants. */
	void exhaustiveness(ref<const ir::Module> mod_, ref<const Vector<std::pair<String, Vector<u32>>>> names_, ref<ir::Diagnostics> diags_) {
		for (const auto& [name, decls] : names_) {
			const auto conditional = std::ranges::any_of(
				decls,
				[&](u32 i_) { return mod_.functions[i_].when.kind != ir::ConditionKind::eTrue; }
			);
			Vector<ir::Condition> arms {};
			for (const auto i : decls) {
				if (mod_.functions[i].body.has_value()) {
					arms.emplace_back(mod_.functions[i].when);
				}
			}
			// no body at all is the merge's to report (`missing body`)
			if (decls.size() < 2uz || not conditional || arms.empty()) {
				continue;
			}
			const auto toggles = named(mod_.toggles, arms);
			if (const auto missing = ir::findUncovered(toggles, arms); missing.has_value()) {
				diags_.emplace_back(error("no body for " + quoted(name) + " at " + combination(toggles, *missing)));
			}
		}
	}
}

std::expected<ir::ResolveResult, ir::Diagnostics> ir::fixToggles(ref<const Module> mod_, ref<const ToggleSources> sources_) {
	return fix(mod_, sources_, Labels { "pass", "profile", "material" });
}

std::expected<ir::Module, ir::Diagnostics> ir::resolveToggles(ref<const Module> mod_, ref<const ToggleSet> toggles_) {
	auto fixed = fix(mod_, ToggleSources { .pass = toggles_, .profile = {}, .material = {} }, Labels { "toggle set", "", "" });
	if (not fixed.has_value()) {
		return std::unexpected(std::move(fixed).error());
	}
	const auto& set = fixed->toggles;
	const auto at = combination(mod_.toggles, set);

	Diagnostics diags {};
	const auto names = byName(mod_);
	exhaustiveness(mod_, names, diags);

	CompactPlan plan {};
	for (const auto& [name, decls] : names) {
		Vector<u32> active {};
		Vector<u32> bodies {};
		for (const auto i : decls) {
			if (evaluate(mod_.functions[i].when, set)) {
				active.emplace_back(i);
				if (mod_.functions[i].body.has_value()) {
					bodies.emplace_back(i);
				}
			}
		}
		if (bodies.size() > 1uz) {
			diags.emplace_back(error("overlapping bodies for " + quoted(name) + " at " + at));
		}
		if (not active.empty()) {
			plan.functions.emplace_back(bodies.empty() ? active.front() : bodies.front());
		}
	}
	if (not diags.empty()) {
		return std::unexpected(std::move(diags));
	}
	std::ranges::sort(plan.functions);

	const auto& bindings = mod_.bindings;
	for (auto i = 0uz; i < bindings.size(); ++i) {
		if (evaluate(bindings[i].when, set)) {
			plan.bindings.emplace_back(static_cast<u32>(i));
		}
	}
	/* the parser froze each use to the first binding of its local name; a dropped one hands over to the survivor */
	plan.bindingUses.resize(bindings.size(), unset);
	for (auto i = 0uz; i < bindings.size(); ++i) {
		if (std::ranges::contains(plan.bindings, static_cast<u32>(i))) {
			plan.bindingUses[i] = static_cast<u32>(i);
			continue;
		}
		/* shortcut: the survivor is matched by local name only, so a merged module whose modules name one binding
		 * differently hands over only under the first module's name; upgrade when names resolve per module scope. */
		const auto it = std::ranges::find(plan.bindings, bindings[i].localName, [&](u32 kept_) { return bindings[kept_].localName; });
		plan.bindingUses[i] = it == plan.bindings.end() ? unset : *it;
	}

	Vector<u32> constants {};
	for (auto i = 0uz; i < mod_.constants.size(); ++i) {
		if (evaluate(mod_.constants[i].when, set)) {
			constants.emplace_back(static_cast<u32>(i));
		}
	}
	plan.constants = std::move(constants);

	plan.stmtAlias.resize(mod_.stmts.size());
	for (auto i = 0uz; i < mod_.stmts.size(); ++i) {
		const auto& s = mod_.stmts[i];
		if (s.kind != StmtKind::eWhen) {
			plan.stmtAlias[i] = static_cast<u32>(i);
			continue;
		}
		const auto arm = evaluate(s.when, set) ? 0uz : 1uz;
		const auto next = arm < s.children.size() ? s.children[arm].index : unset;
		plan.stmtAlias[i] = next == i ? unset : next;// a hand-built self arm would keep the `when`
	}

	auto [result, unresolved] = compact(mod_, plan);
	for (const auto& name : unresolved) {
		diags.emplace_back(error("no body for " + quoted(name) + " at " + at));
	}
	if (not diags.empty()) {
		return std::unexpected(std::move(diags));
	}
	for (auto& binding : result.bindings) {
		binding.when = Condition::always();
	}
	for (auto& constant : result.constants) {
		constant.when = Condition::always();
	}
	for (auto& fn : result.functions) {
		fn.when = Condition::always();
	}
	return std::move(result);
}
