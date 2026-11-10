#pragma once
#include "../RenderCommand.hpp"

namespace hg::engine::render::cmd {
    class EndAccelerationPassRCmd :
        public RenderCommand {
    public:
        constexpr EndAccelerationPassRCmd() noexcept = default;

        constexpr ~EndAccelerationPassRCmd() noexcept override = default;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
