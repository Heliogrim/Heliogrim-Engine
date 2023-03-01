#include "FocusEvent.hpp"

using namespace hg::engine::reflow;
using namespace hg;

FocusEvent::FocusEvent(cref<sptr<Widget>> widget_) noexcept :
    _target(widget_) {}
