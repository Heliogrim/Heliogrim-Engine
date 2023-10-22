#include "ComputeIG.hpp"

#include <Engine.GFX.Acc/Pass/VkComputePass.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/IsType.hpp>
#include <Engine.Scene/RenderGraph.hpp>

#include "../Stage/AccelSubpassStage.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx;
using namespace hg;

ComputeIG::ComputeIG() noexcept = default;

ComputeIG::~ComputeIG() noexcept = default;

void ComputeIG::operator()() {

    const AccelSubpassStage* ass {};

    uptr<IGCommandBuffer> buffer {};

    /*
    auto recorded = record(buffer);
    auto translated = translate(recorded, buffer);

    smr<const acc::AccelerationPass> acceleration = resolve(translated, recorded, buffer, ...);
    [[maybe_unused]] auto igpr = (*this)(_STD move(acceleration), _STD move(buffer));
     */
}

IGProcessResult ComputeIG::operator()(
    mref<smr<const acc::AccelerationPass>> acceleration_,
    mref<uptr<IGCommandBuffer>> igcb_
) {

    #ifdef _DEBUG
    assert(IsType<acc::VkComputePass>(*acceleration_));
    #endif

    const auto* const compute = Cast<acc::VkComputePass>(acceleration_.get());
    AccelerationCommandBuffer cmd = *static_cast<AccelerationCommandBuffer*>(nullptr);

    cmd.begin();

    compute->_vkPipeLayout;
    compute->_vkPipe;
    compute->_vkDescLayouts;

    cmd.bindPipeline(reinterpret_cast<ptr<ComputePipeline>>(compute->_vkPipe));
    cmd.bindDescriptor(Vector<vk::DescriptorSet> {});

    cmd.vkCommandBuffer().dispatch(1uL, 1uL, 1uL);

    cmd.end();

    return IGProcessResult::eFailed;
}

tl::expected<AccelerationCommandBuffer, IGError> ComputeIG::finalize() noexcept {
    return tl::expected<AccelerationCommandBuffer, IGError> { tl::unexpect, IGError {} };
}
