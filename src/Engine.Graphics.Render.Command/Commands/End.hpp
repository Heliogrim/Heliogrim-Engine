#pragma once
#include "../RenderCommand.hpp"

namespace hg::engine::render::cmd {
    class EndRCmd :
        public RenderCommand {
    public:
        constexpr EndRCmd() noexcept = default;

        constexpr ~EndRCmd() noexcept override = default;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
