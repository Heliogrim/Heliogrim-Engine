#pragma once

#include <Engine.Accel.Lang/Rep/Symbol.hpp>
#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::gfx {
    class SampledTextureView;
    class TextureLikeObject;
    class TextureSampler;
}

namespace hg::engine::render::cmd {
    class AttachResourceRCmd :
        public RenderCommand {
    public:
        AttachResourceRCmd(
            nmpt<const gfx::TextureLikeObject> texture_,
            nmpt<const gfx::TextureSampler> sampler_
        ) noexcept;

        ~AttachResourceRCmd() noexcept override = default;

    private:
    public:
        smr<ClassMetaBase> _attached;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;

    public:
        [[nodiscard]] smr<ClassMetaBase> attached() const noexcept;

        [[nodiscard]] nmpt<const gfx::SampledTextureView> asTextureView() const noexcept;
    };
}
