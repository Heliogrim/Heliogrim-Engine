#pragma once
#include "../RenderCommand.hpp"
#include "Structs.hpp"

namespace hg::engine::render::cmd {
    class BeginSubPassRCmd :
        public RenderCommand {
    public:
        constexpr BeginSubPassRCmd(BeginSubPassStruct data_) noexcept :
            data(_STD move(data_)) {}

        constexpr ~BeginSubPassRCmd() noexcept override = default;

    private:
    public:
        BeginSubPassStruct data;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
