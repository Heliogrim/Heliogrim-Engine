#include "Begin.hpp"

using namespace hg::engine::render::cmd;
using namespace hg;

void BeginRCmd::operator()(
    const ptr<RenderCommandTranslator::State> state_,
    const ptr<RenderCommandTranslator> translator_
) const noexcept {
    translator_->translate(state_, this);
}
