#include "AttachResource.hpp"

#include <Engine.GFX/Texture/SampledTextureView.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>

#include <Engine.Reflect/Cast.hpp>

using namespace hg::engine::render::cmd;
using namespace hg;

AttachResourceRCmd::AttachResourceRCmd(
    nmpt<const gfx::TextureLikeObject> texture_,
    nmpt<const gfx::TextureSampler> sampler_
) noexcept :
    RenderCommand(),
    _attached(
        make_smr<gfx::SampledTextureView>(
            _STD move(texture_),
            _STD move(sampler_)
        )
    ) {}

void AttachResourceRCmd::operator()(
    ptr<RenderCommandTranslator::State> state_,
    ptr<RenderCommandTranslator> translator_
) const noexcept {
    translator_->translate(state_, this);
}

smr<ClassMetaBase> AttachResourceRCmd::attached() const noexcept {
    return _attached;
}

nmpt<const engine::gfx::SampledTextureView> AttachResourceRCmd::asTextureView() const noexcept {
    return Cast<gfx::SampledTextureView>(_attached.get());
}
