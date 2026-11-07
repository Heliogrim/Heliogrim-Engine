#include "Interface.hpp"

#include <algorithm>
#include <tuple>

#include "Typecheck.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

Vector<ir::InterfaceEntry> ir::computeInterface(ref<const Module> module_) {
	Vector<InterfaceEntry> entries {};
	entries.reserve(module_.bindings.size());
	for (const auto& binding : module_.bindings) {
		if (binding.bindClass == BindClass::eWorkgroup) {
			continue;
		}
		entries.emplace_back(
			InterfaceEntry { binding.id, binding.type, binding.mode, binding.bindClass, binding.implicit, binding.input, binding.arrayed, describeType(module_.types, binding.type) }
		);
	}

	std::ranges::sort(
		entries,
		[](ref<const InterfaceEntry> a_, ref<const InterfaceEntry> b_) {
			return std::forward_as_tuple(not a_.input, a_.id, a_.mode, a_.cls, a_.implicit, a_.type.index) <
				std::forward_as_tuple(not b_.input, b_.id, b_.mode, b_.cls, b_.implicit, b_.type.index);
		}
	);
	return entries;
}
