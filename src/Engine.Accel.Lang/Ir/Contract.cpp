#include "Contract.hpp"

#include <algorithm>
#include <utility>

using namespace hg::engine::accel::lang;
using namespace hg;

ir::ContractEntry ir::declared(String id_, const BindClass cls_, String typeName_, const std::source_location where_) {
	return ContractEntry {
		.id = std::move(id_),
		.cls = cls_,
		.typeName = std::move(typeName_),
		.at = diag::SourceSpan { .file = where_.file_name(), .begin = 0uL, .end = 0uL, .line = where_.line(), .column = where_.column() }
	};
}

ir::Diagnostics ir::checkContract(
	ref<const Vector<InterfaceEntry>> shader_,
	ref<const StageContract> stage_,
	ref<const Vector<String>> materialBindings_
) {
	Diagnostics diags {};
	for (const auto& entry : shader_) {
		if (entry.mode != BindMode::eBind || not entry.input) {
			continue;
		}

		const auto provider = std::ranges::find(stage_.provides, entry.id, &ContractEntry::id);
		if (entry.cls == BindClass::eNone) {
			diags.emplace_back(
				Diagnostic::error(codes::asl("missing-binding-class"), "binding '" + entry.id + "' has no class").at(entry.at)
			);

		} else if (provider == stage_.provides.end()) {
			if (not std::ranges::contains(materialBindings_, entry.id)) {
				diags.emplace_back(
					Diagnostic::error(
						codes::asl("missing-material-binding"),
						"binding '" + entry.id + "' has no material binding and is not provided by the stage"
					).at(entry.at)
				);
			}

		} else if (provider->cls != BindClass::eNone && provider->cls != entry.cls) {
			diags.emplace_back(
				Diagnostic::error(
					codes::asl("contract-class-mismatch"),
					"binding '" + entry.id + "' is '" + String(className(entry.cls)) + "', but the stage provides '" + String(className(provider->cls)) + "'"
				).at(entry.at).label(provider->at, "provided here")
			);
		}
	}

	for (const auto& req : stage_.requires_) {
		const auto output = std::ranges::find_if(shader_, [&](ref<const InterfaceEntry> e_) { return not e_.input && e_.id == req.id; });
		if (output == shader_.end()) {
			diags.emplace_back(
				Diagnostic::error(codes::asl("missing-stage-output"), "stage requires '" + req.id + "', but the shader does not output it")
					.at(req.at)
			);

		} else if (not req.typeName.empty() && req.typeName != output->typeName) {
			diags.emplace_back(
				Diagnostic::error(
					codes::asl("contract-type-mismatch"),
					"output '" + req.id + "' is '" + output->typeName + "', but the stage requires '" + req.typeName + "'"
				).at(output->at.is<diag::NoLocation>() ? req.at : output->at).label(req.at, "required here")
			);
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
					Diagnostic::error(
						codes::asl("unmatched-stage-input"),
						name(kind) + " input '" + in.id + "' has no matching output in the " + name(prevKind) + " stage"
					).at(in.at)
				);

			} else if (out->typeName != in.typeName) {
				diags.emplace_back(
					Diagnostic::error(
						codes::asl("stage-interface-mismatch"),
						name(kind) + " input '" + in.id + "' is '" + in.typeName + "', but the " + name(prevKind) +
						" stage outputs '" + out->typeName + "'"
					).at(in.at).label(out->at, "output declared here")
				);
			}
		}
	}
	return diags;
}
