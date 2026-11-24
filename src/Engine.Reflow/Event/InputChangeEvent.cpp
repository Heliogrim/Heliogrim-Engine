#include "InputChangeEvent.hpp"

using namespace hg::engine::reflow;
using namespace hg;

InputChangeEvent::InputChangeEvent(ref<const SharedPtr<Widget>> widget_) noexcept :
    target(widget_) {}
