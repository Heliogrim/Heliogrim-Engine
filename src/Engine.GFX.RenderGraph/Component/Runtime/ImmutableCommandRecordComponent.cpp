#include "ImmutableCommandRecordComponent.hpp"

#include <Engine.GFX.Render.Command/RenderCommandPool.hpp>
#include <Engine.GFX.Render.Command/PooledRenderCommandBuffer.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg;

ImmutableCommandRecordComponent::ImmutableCommandRecordComponent(mref<ini_fn_type> fn_) noexcept :
    _fn(_STD move(fn_)) {}

RecordInvalidationResult ImmutableCommandRecordComponent::iterate(cref<IterationPassContext> ctx_) {

    const auto allocated = false;
    if (allocated) {
        return RecordInvalidationResult::eNever;
    }

    assert(_renderCmdViews.empty());

    _renderCmdBuffer = _renderCmdPool.acquire().value();
    _fn(_renderCmdBuffer.get());
    _renderCmdViews.push_back(_renderCmdPool.view(_renderCmdBuffer.get()).value());

    return RecordInvalidationResult::eAlways;
}

cref<CommandRecordComponent::cmd_view_enum> ImmutableCommandRecordComponent::getEnumeration() const noexcept {
    return _renderCmdViews;
}
