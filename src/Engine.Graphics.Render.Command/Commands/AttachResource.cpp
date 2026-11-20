#include "AttachResource.hpp"

import Engine.Reflect;

using namespace hg::engine::render::cmd;
using namespace hg;

AttachResourceRCmd::AttachResourceRCmd() noexcept :
    RenderCommand(),
    _attached() {}

void AttachResourceRCmd::operator()(
    ptr<RenderCommandTranslator::State> state_,
    ptr<RenderCommandTranslator> translator_
) const noexcept {
    translator_->translate(state_, this);
}

smr<ClassMetaBase> AttachResourceRCmd::attached() const noexcept {
    return _attached;
}
