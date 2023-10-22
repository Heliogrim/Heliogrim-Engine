#include "StaticMeshIG.hpp"

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX/Framebuffer/Framebuffer.hpp>
#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc/Pass/VkGraphicsPass.hpp>
#include <Engine.GFX/Geometry/PolygonMesh.hpp>
#include <Engine.GFX.RenderPipeline.Factory/StaticMeshLayout.hpp>

#include "IGCommandBuffer.hpp"
#include "../Stage/SubpassStage.hpp"

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

StaticMeshIG::StaticMeshIG() noexcept = default;

StaticMeshIG::~StaticMeshIG() noexcept = default;

smr<MeshDescription> StaticMeshIG::getMeshDescription() {
    return GetStaticMeshLayout();
}

void StaticMeshIG::operator()() {

    const SubpassStage* subpass {};

    uptr<IGCommandBuffer> buffer {};

    auto recorded = record(buffer);
    auto translated = translate(recorded, buffer);

    smr<const acc::AccelerationPass> acceleration = resolve(translated, recorded, buffer, ...);
    [[maybe_unused]] auto igpr = (*this)(_STD move(acceleration), _STD move(buffer));

}

IGProcessResult StaticMeshIG::operator()(
    mref<smr<const acc::AccelerationPass>> acceleration_,
    mref<uptr<IGCommandBuffer>> igcb_
) {

    // TODO: expect( ibcb_ ~> mesh.getType() ).toBe( GetStaticMeshDescription() );
    //assert(IsType<StaticGeometryModel(...));

    auto pool = CommandPool { nullptr };
    AccelerationCommandBuffer acmd = pool.make();

    auto lorp = gfx::pipeline::LORenderPass { nullptr };
    auto frame = Framebuffer { nullptr };

    acmd.begin();
    acmd.beginRenderPass(lorp, frame);

    /**/

    auto vkAccelPass = static_cast<ptr<const acc::VkGraphicsPass>>(acceleration_.get());

    vkAccelPass->_vkDescLayouts;
    vkAccelPass->_vkPipe;
    vkAccelPass->_vkPipeLayout;

    // internal.bindPipeline();
    acmd.vkCommandBuffer().bindPipeline(
        vk::PipelineBindPoint::eGraphics,
        reinterpret_cast<VkPipeline>(vkAccelPass->_vkPipe)
    );

    /**/

    PolygonMesh* mesh {};

    const auto& indices = mesh->indices() /* igcb_->consume() ?!? ->mesh()->indices() */;
    const auto& vertices = mesh->vertices() /* igcb_->consume() ?!? ->mesh()->vertices() */;

    acmd.bindIndexBuffer(indices.get());
    acmd.bindVertexBuffer(0uL, vertices.get());

    acmd.drawIndexed(1uL, 0uL, indices->size(), 0uL, 0uL);

    /**/

    return IGProcessResult::eFailed;
}

tl::expected<engine::gfx::AccelerationCommandBuffer, IGError> StaticMeshIG::finalize() noexcept {

    auto pool = CommandPool { nullptr };
    auto internal = pool.make();

    internal.endRenderPass();
    internal.end();

    /**/

    return tl::expected<AccelerationCommandBuffer, IGError> { tl::unexpect, IGError {} };
}
