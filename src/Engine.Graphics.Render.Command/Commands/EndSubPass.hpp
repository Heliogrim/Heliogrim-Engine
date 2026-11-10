#pragma once
#include "../RenderCommand.hpp"

namespace hg::engine::render::cmd {
    class EndSubPassRCmd :
        public RenderCommand {
    public:
        constexpr EndSubPassRCmd() noexcept = default;

        constexpr ~EndSubPassRCmd() noexcept override = default;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
