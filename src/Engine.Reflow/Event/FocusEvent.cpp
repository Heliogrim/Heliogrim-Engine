#include "FocusEvent.hpp"

using namespace ember::engine::reflow;
using namespace ember;

FocusEvent::FocusEvent(cref<sptr<Widget>> widget_) noexcept :
    _target(widget_) {}
