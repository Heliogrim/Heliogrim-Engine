#include "BindIndexBuffer.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

void BindIndexBufferRCmd::operator()(
    ptr<RenderCommandTranslator::State> state_,
    ptr<RenderCommandTranslator> translator_
) const noexcept {
    translator_->translate(state_, this);
}
