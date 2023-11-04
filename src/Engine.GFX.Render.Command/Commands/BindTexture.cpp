#include "BindTexture.hpp"

#include "Engine.GFX.Render.Command/RenderCommandTranslationUnit.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

void BindTextureRenderCommand::operator()(ptr<RenderCommandTranslationUnit> rctu_) const noexcept {
    rctu_->translate(this);
}
