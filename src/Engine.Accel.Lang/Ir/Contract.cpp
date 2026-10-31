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
