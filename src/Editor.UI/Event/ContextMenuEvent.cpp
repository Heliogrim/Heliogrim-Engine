#include "ContextMenuEvent.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

ContextMenuEvent::ContextMenuEvent(ref<MenuBuilder> builder_) noexcept :
	StatelessEvent(),
	builder(builder_) {}
