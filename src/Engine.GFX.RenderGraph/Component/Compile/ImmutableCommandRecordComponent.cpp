#include "ImmutableCommandRecordComponent.hpp"
#include "../Runtime/ImmutableCommandRecordComponent.hpp"

#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

void CompileImmutableCommandRecordComponent::setInitializer(mref<ini_stg_type> staticGeometryInitializer_) {
    _initializer = _STD move(staticGeometryInitializer_);
}

void CompileImmutableCommandRecordComponent::setInitializer(mref<ini_fn_type> function_) {
    _initializer = _STD move(function_);
}

uptr<CommandRecordComponent> CompileImmutableCommandRecordComponent::compile(
    cref<CompilePassContext> ctx_
) const noexcept {

    assert(not _initializer.valueless_by_exception());

    // TODO: Check what we want to support for an immutable recording downstream
    // TODO: Possible:
    // TODO:            1. Callback Function
    // TODO:            2. Static Geometry Resource + Effect
    // TODO: (Maybe)    3. Static Geometry Resource + Material

    // Note: Supporting material might be easier than plane effects due to resource bindings,
    //          cause we are able to use the material's parameters directly.

    ImmutableCommandRecordComponent::ini_fn_type generator {};
    if (auto* stgVal = _STD get_if<ini_stg_type>(&_initializer)) {
        generator = [stgVal = clone(*stgVal)](mref<nmpt<cmd::RenderCommandBuffer>> cmd_) {

            cmd_->begin();

            /**/

            // Error // TODO: We need a possibility to bind plane effects instead of whole materials
            // Warning: We will run into problems, due to the missing binding definitions for custom bound effects
            //  e.g. cmd_->bindTexture(...);
            //  e.g. cmd_->bindStorage(...);
            smr<const acc::AccelerationEffect> effect {};
            //cmd_->bindEffect(effect);

            /**/

            const auto guard = stgVal->acquire(resource::ResourceUsageFlag::eRead);
            const auto* const mesh = guard.imm();
            cmd_->bindStaticMesh(mesh);

            if (mesh->indices().get() && mesh->indices()->size() > 0) {
                cmd_->drawStaticMeshIdx(1uL, 0uL, static_cast<u32>(mesh->indices()->size()) / 3uL, 0uL);
            } else {
                cmd_->drawStaticMesh(1uL, 0uL, static_cast<u32>(mesh->vertices()->size()) / 3uL, 0uL);
            }

            /**/

            cmd_->end();
        };

    } else if (auto* fnVal = _STD get_if<ini_fn_type>(&_initializer)) {
        generator = *fnVal;
    }

    return ctx_.getGraphNodeAllocator()->allocate<ImmutableCommandRecordComponent>(
        _STD move(generator)
    );
}
