#pragma once
#include "../RenderCommand.hpp"
#include "Structs.hpp"

namespace hg::engine::render::cmd {
    class BeginAccelerationPassRCmd :
        public RenderCommand {
    public:
        constexpr BeginAccelerationPassRCmd(BeginAccelerationPassStruct data_) noexcept :
            data(std::move(data_)) {}

        constexpr ~BeginAccelerationPassRCmd() noexcept override = default;

    private:
    public:
        BeginAccelerationPassStruct data;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
