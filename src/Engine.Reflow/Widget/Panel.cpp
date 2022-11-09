#include "Panel.hpp"

using namespace ember::engine::reflow;
using namespace ember;

Panel::Panel(mref<sptr<BoundStyleSheet>> style_) :
    VBox(_STD move(style_)) {}

Panel::~Panel() = default;
