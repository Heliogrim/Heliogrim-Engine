#pragma once

#include <functional>
#include <Engine.GFX.Render.Command/PooledRenderCommandBuffer.hpp>
#include <Engine.GFX.Render.Command/RenderCommandPool.hpp>

#include "CommandRecordComponent.hpp"

namespace hg::engine::gfx::render::graph {
    class ImmutableCommandRecordComponent final :
        public InheritMeta<ImmutableCommandRecordComponent, CommandRecordComponent> {
    public:
        using this_type = ImmutableCommandRecordComponent;

        using ini_fn_type = _STD function<void(mref<nmpt<cmd::RenderCommandBuffer>> cmd_)>;

    public:
        ImmutableCommandRecordComponent(mref<ini_fn_type> fn_) noexcept;

        ~ImmutableCommandRecordComponent() noexcept override = default;

    private:
        ini_fn_type _fn;

        cmd::RenderCommandPool _renderCmdPool;
        uptr<cmd::PooledRenderCommandBuffer> _renderCmdBuffer;
        cmd_view_enum _renderCmdViews;

    public:
        [[nodiscard]] RecordInvalidationResult iterate(cref<IterationPassContext> ctx_) override;

        [[nodiscard]] cref<cmd_view_enum> getEnumeration() const noexcept override;
    };
}
