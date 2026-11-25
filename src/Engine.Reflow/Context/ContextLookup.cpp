#include "ContextLookup.hpp"

using namespace hg::engine::reflow;
using namespace hg;

nmpt<Widget> hg::engine::reflow::findContextProvider(ref<const Widget> from_, const type_id contextType_) noexcept {
	for (auto next = from_.parent(); next != nullptr; next = next->parent()) {
		if (next->providedContext(contextType_) != nullptr) {
			return next.get();
		}
	}
	return nullptr;
}
