#pragma once
#include <Engine.Common/Types.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::render::cmd {
    class DrawDispatchRCmd final :
        public RenderCommand {
    public:
        using this_type = DrawDispatchRCmd;

    public:
        constexpr DrawDispatchRCmd(
            const u32 instanceCount_,
            const u32 instanceOffset_,
            const u32 vertexCount_,
            const u32 vertexOffset_,
            const bool indexedPrimitive_
        ) noexcept :
            _instanceCount(instanceCount_),
            _instanceOffset(instanceOffset_),
            _vertexCount(vertexCount_),
            _vertexOffset(vertexOffset_),
            _indexedPrimitive(indexedPrimitive_) {}

        constexpr ~DrawDispatchRCmd() noexcept override = default;

    private:
    public:
        const u32 _instanceCount;
        const u32 _instanceOffset;
        const u32 _vertexCount;
        const u32 _vertexOffset;
        const bool _indexedPrimitive;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
