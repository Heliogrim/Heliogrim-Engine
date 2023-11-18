#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

#include "RenderCommandBuffer.hpp"
#include "RenderCommand.hpp"
#include "RenderCommandAllocatorBase.hpp"
#include "RenderCommandBufferBase.hpp"
#include "RenderCommandTranslator.hpp"
#include "Commands/Begin.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

void ttt_test() {
    auto rcb = make_uptr<RenderCommandBuffer>();

    constexpr auto instanceCount = 1uL;
    constexpr auto instanceOffset = 0uL;
    constexpr auto primitiveCount = 20uL;
    constexpr auto primitiveOffset = 0uL;

    constexpr engine::gfx::MeshDescription* meshDescription = nullptr;

    rcb->begin();

    rcb->bindStaticMesh(nullptr);
    rcb->bindStaticMeshInstance(nullptr);
    rcb->drawStaticMeshIdx(instanceCount, instanceOffset, primitiveCount, primitiveOffset);

    /**/
    rcb->nextSubPass();
    /**/

    rcb->bindSkeletalMesh(nullptr);
    rcb->bindSkeletalMeshInstance(nullptr, nullptr);
    rcb->drawSkeletalMeshIdx(instanceCount, instanceOffset, primitiveCount, primitiveOffset);

    /**/
    rcb->nextSubPass();
    /**/

    rcb->bindIndexBuffer(nullptr);
    rcb->bindVertexBuffer(nullptr);
    rcb->bindStorage(nullptr, nullptr);

    rcb->drawMesh(
        meshDescription,
        instanceCount,
        instanceOffset,
        primitiveCount,
        primitiveOffset
    );
    rcb->drawMeshIdx(
        meshDescription,
        instanceCount,
        instanceOffset,
        primitiveCount,
        primitiveOffset
    );

    /**/

    rcb->end();

    /**/

    RenderCommandTranslator* tu { nullptr };

    const auto result = (*tu)(rcb.get());
    result->commitAndDispose();

    /**/

    rcb.release();
}
