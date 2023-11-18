#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    struct ResourceTable;
}

namespace hg::engine::gfx::render::cmd {
    class BindResourceTablerRCmd :
        public RenderCommand {
    public:
        using this_type = BindResourceTablerRCmd;

    public:
        constexpr BindResourceTablerRCmd(const ptr<const ResourceTable> resourceTable_) noexcept :
            RenderCommand(),
            _resourceTable(_STD move(resourceTable_)) {}

        constexpr ~BindResourceTablerRCmd() noexcept override = default;

    private:
    public:
        const ptr<const ResourceTable> _resourceTable;

    public:
        void operator()(
            ptr<RenderCommandTranslator::State> state_,
            ptr<RenderCommandTranslator> translator_
        ) const noexcept override;
    };
}
