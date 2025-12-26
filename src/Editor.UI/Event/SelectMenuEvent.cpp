#include "SelectMenuEvent.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

SelectMenuEvent::SelectMenuEvent(ref<MenuBuilder> builder_) noexcept :
	StatelessEvent(),
	builder(builder_) {}