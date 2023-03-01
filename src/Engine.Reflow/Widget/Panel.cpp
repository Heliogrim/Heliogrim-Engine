#include "Panel.hpp"

using namespace hg::engine::reflow;
using namespace hg;

Panel::Panel(mref<sptr<BoundStyleSheet>> style_) :
    VBox(_STD move(style_)) {}

Panel::~Panel() = default;
