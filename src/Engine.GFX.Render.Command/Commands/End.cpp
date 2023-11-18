#include "End.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

void EndRCmd::operator()(
    ptr<RenderCommandTranslator::State> state_,
    ptr<RenderCommandTranslator> translator_
) const noexcept {
    translator_->translate(state_, this);
}
