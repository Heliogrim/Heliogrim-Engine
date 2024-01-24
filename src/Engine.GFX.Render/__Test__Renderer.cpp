#include "__Test__Renderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Effect/Stage/Stage.hpp>
#include <Engine.Accel.Effect/Stage/StageFlags.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Subpass/Impl/DepthPrePass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/DummyProvider.hpp>
#include <Engine.GFX.Render.Subpass/Impl/MatTestPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/PostProcessPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/SkyBoxPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/TmpBrdfIrradPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/TmpBrdfLutPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/TmpBrdfPrefPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/TmpDirectionalShadowPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/TmpEndPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/TriTestPass.hpp>
#include <Engine.GFX.Render.Subpass/Impl/Visualize.hpp>
#include <Engine.GFX.Render.Subpass/Impl/__tmp_helper.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.GFX.RenderGraph/Node/Compile/CompileSubPassNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Pass/CompilePassContext.hpp>
#include <Engine.Reflow.Render/ReflowPass.hpp>

using namespace hg::engine::render;
using namespace hg;

[[nodiscard]] static smr<engine::accel::AccelerationEffect> build_test_effect();

[[nodiscard]] static smr<engine::accel::GraphicsPipeline> build_test_pipeline();

TestRenderer::TestRenderer(
    mref<nmpt<gfx::cache::GlobalCacheCtrl>> globalCache_,
    mref<nmpt<gfx::memory::GlobalPooledAllocator>> globalGfxAllocator_
) noexcept :
    Renderer(
        Guid {},
        "Test-Renderer",
        makeCompileGraph(),
        nullptr,
        std::move(globalCache_),
        std::move(globalGfxAllocator_)
    ) {}

TestRenderer::~TestRenderer() noexcept = default;

uptr<graph::CompileGraph> TestRenderer::makeCompileGraph() noexcept {

    auto graph = make_uptr<graph::CompileGraph>();
    graph->addExpectedProvision(graph::Provision { makeSceneColorSymbol() });

    /**/

    const nmpt<graph::Node> begin = graph->begin().get();
    const nmpt<graph::Node> end = graph->end().get();

    nmpt<graph::Node> cursor { begin };

    /**/

    auto beforeBarrier = make_smr<graph::BarrierNode>();
    auto brdfLutGen = make_smr<graph::CompileSubPassNode>();
    auto brdfPrefGen = make_smr<graph::CompileSubPassNode>();
    auto brdfIrradGen = make_smr<graph::CompileSubPassNode>();
    auto dummyProvider = make_smr<graph::CompileSubPassNode>();
    auto depthPrePass = make_smr<graph::CompileSubPassNode>();
    auto subpass = make_smr<graph::CompileSubPassNode>();
    auto skyBoxPass = make_smr<graph::CompileSubPassNode>();
    auto dirShadowPass = make_smr<graph::CompileSubPassNode>();
    auto matPass = make_smr<graph::CompileSubPassNode>();
    auto ppPass = make_smr<graph::CompileSubPassNode>();
    auto visualizePass = make_smr<graph::CompileSubPassNode>();
    auto endPass = make_smr<graph::CompileSubPassNode>();
    auto afterBarrier = make_smr<graph::BarrierNode>();

    auto uiPass = make_smr<graph::CompileSubPassNode>();

    /**/

    brdfLutGen->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<TmpBrdfLutPass>>();
            return node;
        }
    );

    brdfPrefGen->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<TmpBrdfPrefPass>>();
            return node;
        }
    );

    brdfIrradGen->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<TmpBrdfIrradPass>>();
            return node;
        }
    );

    dummyProvider->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<DummyProvider>>();
            return node;
        }
    );

    depthPrePass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<DepthPrePass>>();
            return node;
        }
    );

    subpass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<TriTestPass>>();
            // acceleration->addOutput(makeSceneColorSymbol());
            return node;
        }
    );

    skyBoxPass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<SkyBoxPass>>();
            // acceleration->addOutput(makeSceneColorSymbol());
            return node;
        }
    );

    dirShadowPass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<TmpDirectionalShadowPass>>();
            return node;
        }
    );

    matPass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<MatTestPass>>();
            // acceleration->addOutput(makeSceneColorSymbol());
            return node;
        }
    );

    ppPass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<PostProcessPass>>();
            // acceleration->addOutput(makeSceneColorSymbol());
            return node;
        }
    );

    visualizePass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<Visualize>>();
            // acceleration->addOutput(makeSceneColorSymbol());
            return node;
        }
    );

    endPass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()
                            ->allocate<graph::SubPassNode<TmpEndPass>>(vk::ImageLayout::eShaderReadOnlyOptimal);
            return node;
        }
    );

    uiPass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<reflow::render::ReflowPass>>();
            // acceleration->addOutput(makeSceneColorSymbol());
            return node;
        }
    );

    /**/

    nmpt<graph::Node> nextCursor = beforeBarrier.get();
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(beforeBarrier));

    cursor = std::exchange(nextCursor, brdfLutGen.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(brdfLutGen));

    cursor = std::exchange(nextCursor, brdfPrefGen.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(brdfPrefGen));

    cursor = std::exchange(nextCursor, brdfIrradGen.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(brdfIrradGen));

    cursor = std::exchange(nextCursor, dummyProvider.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(dummyProvider));

    cursor = std::exchange(nextCursor, depthPrePass.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(depthPrePass));

    cursor = std::exchange(nextCursor, skyBoxPass.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(skyBoxPass));

    //cursor = std::exchange(nextCursor, subpass.get());
    //graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(subpass));

    cursor = std::exchange(nextCursor, dirShadowPass.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(dirShadowPass));

    cursor = std::exchange(nextCursor, matPass.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(matPass));

    //cursor = std::exchange(nextCursor, ppPass.get());
    //graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(ppPass));

    //cursor = std::exchange(nextCursor, visualizePass.get());
    //graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(visualizePass));

    //cursor = std::exchange(nextCursor, uiPass.get());
    //graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(uiPass));

    cursor = std::exchange(nextCursor, endPass.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(endPass));

    cursor = std::exchange(nextCursor, afterBarrier.get());
    graph = graph::Builder::insertNode(std::move(graph), cursor, end, std::move(afterBarrier));

    /**/

    return graph;
}

/**/

smr<engine::accel::AccelerationEffect> build_test_effect() {

    auto vertexStage = make_smr<engine::accel::Stage>(
        engine::accel::StageFlagBits::eVertex
        // Vector<StageInput> {},
        // Vector<StageOutput> {
        //     StageOutput { TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec3, DataOutputRate {} }
        // }
    );

    auto fragmentStage = make_smr<engine::accel::Stage>(
        engine::accel::StageFlagBits::eFragment
        // Vector<StageInput> {
        //     StageInput {
        //         TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec3, DataInputRate::ePerInvocation
        //     },
        // },
        // Vector<StageOutput> {
        //     StageOutput { TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec4, DataOutputRate {} }
        // }
    );

    /**/

    const auto vertexShaderCode = read_shader_file("__test__render.vs");

    vertexStage->setIntermediate(make_smr<engine::accel::lang::Intermediate>());
    vertexStage->getIntermediate()->lang.dialect = engine::accel::lang::Dialect::eVulkanGlsl460;
    vertexStage->getIntermediate()->lang.text.emplace_back(std::move(vertexShaderCode));

    const auto fragmentShaderCode = read_shader_file("__test__render.fs");

    fragmentStage->setIntermediate(make_smr<engine::accel::lang::Intermediate>());
    fragmentStage->getIntermediate()->lang.dialect = engine::accel::lang::Dialect::eVulkanGlsl460;
    fragmentStage->getIntermediate()->lang.text.emplace_back(std::move(fragmentShaderCode));

    /**/

    Guid guid;
    GuidGenerate(guid);

    return make_smr<engine::accel::AccelerationEffect>(
        std::move(guid),
        "test-render-effect",
        Vector<smr<engine::accel::Stage>> { std::move(vertexStage), std::move(fragmentStage) }
        // Vector<smr<const Symbol>> {},
        // Vector<smr<const Symbol>> { makeSceneColorSymbol() }
    );
}
