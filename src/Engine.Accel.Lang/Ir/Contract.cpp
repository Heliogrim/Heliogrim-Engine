#include "Contract.hpp"

#include <algorithm>

using namespace hg::engine::accel::lang;
using namespace hg;

ir::Diagnostics ir::checkContract(
	ref<const Vector<InterfaceEntry>> shader_,
	ref<const StageContract> stage_,
	ref<const Vector<String>> materialBindings_
) {
	const auto contains = [](ref<const Vector<String>> ids_, ref<const String> id_) {
		return std::ranges::find(ids_, id_) != ids_.end();
	};

	Diagnostics diags {};
	for (const auto& entry : shader_) {
		if (entry.mode != BindMode::eBind || not entry.input) {
			continue;
		}
		if (entry.cls == BindClass::eNone) {
			diags.emplace_back(Diagnostic { .message = "Binding '" + entry.id + "' has no class." });
		} else if (not contains(materialBindings_, entry.id) && not contains(stage_.provides, entry.id)) {
			diags.emplace_back(
				Diagnostic { .message = "Binding '" + entry.id + "' has no material binding and is not provided by the stage." }
			);
		}
	}

	for (const auto& id : stage_.requires_) {
		const auto provided = std::ranges::any_of(shader_, [&](ref<const InterfaceEntry> e_) { return not e_.input && e_.id == id; });
		if (not provided) {
			diags.emplace_back(Diagnostic { .message = "Stage requires '" + id + "', which the shader does not output." });
		}
	}
	return diags;
}

ir::Diagnostics ir::checkStageChain(std::span<const std::pair<EntryKind, Vector<InterfaceEntry>>> stages_) {
	const auto name = [](const EntryKind kind_) -> String {
		switch (kind_) {
			case EntryKind::eVertex: return "vertex";
			case EntryKind::eTessCtrl: return "tess_ctrl";
			case EntryKind::eTessEval: return "tess_eval";
			case EntryKind::eGeometry: return "geometry";
			case EntryKind::eFragment: return "fragment";
			default: return "compute";
		}
	};

	Diagnostics diags {};
	for (size_t idx = 1uz; idx < stages_.size(); ++idx) {
		const auto& [prevKind, prev] = stages_[idx - 1uz];
		const auto& [kind, entries] = stages_[idx];

		for (const auto& in : entries) {
			if (not in.input || in.implicit || in.mode != BindMode::eForward) {
				continue;
			}

			const auto out = std::ranges::find_if(
				prev,
				[&](ref<const InterfaceEntry> e_) {
					return not e_.input && not e_.implicit && e_.mode == BindMode::eForward && (not e_.arrayed || in.arrayed) &&
						e_.patch == in.patch && e_.id == in.id;
				}
			);
			if (out == prev.end()) {
				diags.emplace_back(
					Diagnostic {
						.message = name(kind) + " input '" + in.id + "' has no matching output in " + name(prevKind) + " stage"
					}
				);
			} else if (out->typeName != in.typeName) {
				diags.emplace_back(
					Diagnostic {
						.message = name(kind) + " input '" + in.id + "' is " + in.typeName + ", but the " + name(prevKind) +
						" stage outputs " + out->typeName
					}
				);
			}
		}
	}
	return diags;
}
