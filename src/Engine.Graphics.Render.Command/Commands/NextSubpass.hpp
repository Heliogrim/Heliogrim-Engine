#pragma once
#include "../RenderCommand.hpp"

namespace hg::engine::render::cmd {
    class NextSubpassRCmd :
        public RenderCommand {
    public:
        constexpr NextSubpassRCmd() noexcept = default;

        constexpr ~NextSubpassRCmd() noexcept override = default;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
