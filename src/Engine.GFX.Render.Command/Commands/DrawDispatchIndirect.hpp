#pragma once
#include "../RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class DrawDispatchIndirectRCmd :
        public RenderCommand {
    public:
        using this_type = DrawDispatchIndirectRCmd;

    public:
        constexpr DrawDispatchIndirectRCmd(
            const size_t count_,
            const void* const payload_,
            const bool indexedPrimitive_
        ) noexcept :
            count(count_),
            payload(payload_),
            indexedPrimitive(indexedPrimitive_) {}

        constexpr ~DrawDispatchIndirectRCmd() noexcept override = default;

    private:
    public:
        const size_t count;
        const void* const payload;
        const bool indexedPrimitive;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
