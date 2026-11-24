#include "AttributeBatch.hpp"

#include "Widget.hpp"
#include "../ReflowPhase.hpp"

import Engine.Asserts;

using namespace hg::engine::reflow;
using namespace hg;

AttributeBatch::AttributeBatch(ref<Widget> widget_) noexcept :
	_widget(widget_) {
	::hg::assertd(_widget._batchDepth < std::numeric_limits<decltype(_widget._batchDepth)>::max());
	++_widget._batchDepth;
}

AttributeBatch::~AttributeBatch() {

	if (--_widget._batchDepth > 0u || _widget._batchPhases.unwrap == 0u) {
		return;
	}

	markPhases(_widget, std::exchange(_widget._batchPhases, Flag<ReflowPhase, u8> {}));
}
