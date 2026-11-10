#include "BindSkeletalMesh.hpp"

using namespace hg::engine::render::cmd;
using namespace hg;

void BindSkeletalMeshRCmd::operator()(
    ptr<RenderCommandTranslator::State> state_,
    ptr<RenderCommandTranslator> translator_
) const noexcept {
    translator_->translate(state_, this);
}
