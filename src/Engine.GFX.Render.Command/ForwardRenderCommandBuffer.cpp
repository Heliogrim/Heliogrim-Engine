#include "ForwardRenderCommandBuffer.hpp"

#include <Engine.Accel.Storage/Storage.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>
#include <Engine.Accel.Pipeline/ComputePipeline.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>

#include "Commands/BindPipeline.hpp"

using namespace hg::engine::render::cmd;
using namespace hg::engine::gfx;
using namespace hg;

FORCE_INLINE static void link(ref<nmpt<RenderCommand>> prev_, mref<ptr<RenderCommand>> next_);

/**/

ForwardRenderCommandBuffer::ForwardRenderCommandBuffer() noexcept :
    RenderCommandBuffer() {}

ForwardRenderCommandBuffer::ForwardRenderCommandBuffer(mref<this_type> other_) noexcept :
    RenderCommandBuffer(_STD move(other_)) {}

GraphicsPipelineRComRef ForwardRenderCommandBuffer::createGraphicsPipelineImmediately(
    mref<smr<const accel::AccelerationEffect>> effect_,
    nmpt<void> specification_,
    nmpt<void> profile_
) {

    ptr<accel::AccelStorage> storage {};
    const accel::Permutation permutation {
        static_cast<accel::EffectProfile*>(profile_.get()),
        static_cast<accel::EffectSpecification*>(specification_.get())
    };

    auto pipeline = storage->getAccelPipeline(
        accel::calcStorageHash(permutation),
        accel::calcStorageHash(*effect_)
    );

    GraphicsPipelineRComRef rtr {};
    if (pipeline.empty()) {

        /*
         rtr = _resourceTable.allocate<
            const acc::GraphicsPipeline,
            //ConcurrentSharedResource
            smr
        >();
         */

        // TODO: Compilation Request
        // auto compiler = ...;
        // auto result = compiler.compile( request { ... } );
        // store( rtr, result.value );

    } else {

        /*
        rtr = _resourceTable.allocate<
            const acc::GraphicsPipeline,
            //ConcurrentSharedResource
            smr
        >(
            pipeline.into<acc::GraphicsPipeline>()
        );
         */

    }

    return rtr;
}

/**/

void link(ref<nmpt<RenderCommand>> prev_, mref<ptr<RenderCommand>> next_) {
    prev_->next = next_;
    prev_ = next_;
}
