#include "StyleCascade.hpp"

import Engine.Common;

using namespace hg::engine::reflow::theming;
using namespace hg::engine::reflow;
using namespace hg;

Opt<ref<const ComponentStyle>> hg::engine::reflow::theming::findComponentStyle(
	ref<const ThemeContext> context_,
	ref<const ReflowClassList> classList_
) {
	return FirstOf<Opt<ref<const ComponentStyle>>>(
		classList_,
		[&](const auto& className_) -> Opt<ref<const ComponentStyle>> {
			if (not className_.template is<StringView>()) {
				return None;
			}

			return context_.getComponentStyle(className_.template as<StringView>());
		}
	);
}
