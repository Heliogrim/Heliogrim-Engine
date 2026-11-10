#include "BeginAccelerationPass.hpp"

using namespace hg::engine::render::cmd;
using namespace hg;

void BeginAccelerationPassRCmd::operator()(
    ptr<RenderCommandTranslator::State> state_,
    ptr<RenderCommandTranslator> translator_
) const noexcept {
    translator_->translate(state_, this);
}
