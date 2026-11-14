#include "Module.hpp"

#include <cassert>

using namespace hg::engine::accel::lang;
using namespace hg;

ir::SourceSpan ir::spanIn(ref<const Module> mod_, SourceLoc loc_) {
	if (mod_.sources.empty()) {
		assert(loc_.source == 0u);
		return ir::pointAt(mod_.name, loc_);
	}
	assert(loc_.source < mod_.sources.size());
	return ir::pointAt(mod_.sources[loc_.source], loc_);
}
