#include "BindSkeletalMesh.hpp"

#include "Engine.GFX.Render.Command/RenderCommandTranslationUnit.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

void BindSkeletalMeshRenderCommand::operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept {
    rctu_->translate(this);
}
