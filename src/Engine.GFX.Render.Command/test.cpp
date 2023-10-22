#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

#include "PooledRenderCommandBuffer.hpp"
#include "RenderCommand.hpp"
#include "RenderCommandPool.hpp"
#include "RenderCommandAllocator.hpp"
#include "RenderCommandBuffer.hpp"
#include "RenderCommandTranslationResult.hpp"
#include "RenderCommandTranslationUnit.hpp"
#include "Commands/Begin.hpp"
#include "Commands/BindMaterial.hpp"

using namespace hg::engine::gfx::render::cmd;
using namespace hg;

void ttt_test() {

    RenderCommandPool rcp {};
    auto ar = rcp.acquire();
    uptr<PooledRenderCommandBuffer> rcb = _STD move(ar.value());

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
    rcb->nextSubpass();
    /**/

    rcb->bindSkeletalMesh(nullptr);
    rcb->bindSkeletalMeshInstance(nullptr, nullptr);
    rcb->drawSkeletalMeshIdx(instanceCount, instanceOffset, primitiveCount, primitiveOffset);

    /**/
    rcb->nextSubpass();
    /**/

    rcb->bindIndexBuffer(nullptr);
    rcb->bindVertexBuffer(nullptr);
    rcb->bindStorage(nullptr);

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

    auto view = rcp.view(rcb.get()).value();
    RenderCommandTranslationUnit* tu { nullptr };

    const auto result = (*tu)(_STD move(view));

    /**/

    rcp.release(_STD move(rcb));
}
