#include "__Test__Renderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc/Stage/Stage.hpp>
#include <Engine.GFX.Acc/Stage/StageFlags.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBufferBase.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Subpass/Impl/TriTestPass.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.GFX.RenderGraph/Node/Compile/CompileSubPassNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Pass/CompilePassContext.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

[[nodiscard]] static smr<engine::gfx::acc::AccelerationEffect> build_test_effect();

[[nodiscard]] static smr<engine::gfx::acc::GraphicsPipeline> build_test_pipeline();

TestRenderer::TestRenderer(
    mref<nmpt<cache::GlobalCacheCtrl>> globalCache_,
    mref<nmpt<memory::GlobalPooledAllocator>> globalGfxAllocator_
) noexcept :
    Renderer(
        Guid {},
        "Test-Renderer",
        makeCompileGraph(),
        nullptr,
        _STD move(globalCache_),
        _STD move(globalGfxAllocator_)
    ) {}

TestRenderer::~TestRenderer() noexcept = default;

uptr<graph::CompileGraph> TestRenderer::makeCompileGraph() noexcept {

    auto graph = make_uptr<graph::CompileGraph>();
    graph->addExpectedProvision(graph::Provision { makeSceneColorSymbol() });

    /**/

    const nmpt<graph::Node> begin = graph->begin().get();
    const nmpt<graph::Node> end = graph->end().get();

    nmpt<graph::Node> cursor { begin };

    smr<void> payload {};
    auto test = new smr<void>(_STD move(payload));

    /**/

    auto beforeBarrier = make_smr<graph::BarrierNode>();
    auto subpass = make_smr<graph::CompileSubPassNode>();
    auto afterBarrier = make_smr<graph::BarrierNode>();

    /**/

    subpass->setSubPassBuilder(
        [](cref<graph::CompilePassContext> ctx_) -> uptr<graph::SubPassNodeBase> {
            auto node = ctx_.getGraphNodeAllocator()->allocate<graph::SubPassNode<TriTestPass>>();
            // acceleration->addOutput(makeSceneColorSymbol());
            return node;
        }
    );

    /**/

    nmpt<graph::Node> nextCursor = beforeBarrier.get();
    graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(beforeBarrier));

    cursor = _STD exchange(nextCursor, subpass.get());
    graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(subpass));

    cursor = _STD exchange(nextCursor, afterBarrier.get());
    graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(afterBarrier));

    /**/

    return graph;
}

/**/

#include <filesystem>
#include <fstream>
using namespace ::hg::engine::gfx::acc;

static string read_shader_file(string name_) {

    const auto root = R"(R:\Development\C++\Vulkan API\Game\resources\shader\)";
    std::filesystem::path file { root };
    file.append(name_);

    if (not std::filesystem::exists(file)) {
        __debugbreak();
        return {};
    }

    auto ifs = _STD ifstream(file, std::ios_base::in | std::ios_base::binary);

    ifs.seekg(0, _STD ios::end);
    const auto fsize = ifs.tellg();
    ifs.seekg(0, _STD ios::beg);

    string tmp {};
    tmp.resize(fsize);

    ifs.read(tmp.data(), fsize);
    assert(!ifs.bad());

    ifs.close();
    return tmp;
}

smr<AccelerationEffect> build_test_effect() {

    auto vertexStage = make_smr<Stage>(
        StageFlagBits::eVertex,
        Vector<StageInput> {},
        Vector<StageOutput> {
            StageOutput { TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec3, DataOutputRate {} }
        }
    );

    auto fragmentStage = make_smr<Stage>(
        StageFlagBits::eFragment,
        Vector<StageInput> {
            StageInput {
                TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec3, DataInputRate::ePerInvocation
            },
        },
        Vector<StageOutput> {
            StageOutput { TransferToken {}, TransferType::eForward, TransferDataType::eU8Vec4, DataOutputRate {} }
        }
    );

    /**/

    const auto vertexShaderCode = read_shader_file("__test__render.vs");

    vertexStage->setIntermediate(
        make_smr<lang::Intermediate>(
            make_uptr<lang::IL>(lang::ILDialect::eVulkanGlsl, Vector<string> { _STD move(vertexShaderCode) }),
            nullptr
        )
    );

    const auto fragmentShaderCode = read_shader_file("__test__render.fs");

    fragmentStage->setIntermediate(
        make_smr<lang::Intermediate>(
            make_uptr<lang::IL>(lang::ILDialect::eVulkanGlsl, Vector<string> { _STD move(fragmentShaderCode) }),
            nullptr
        )
    );

    /**/

    Guid guid;
    GuidGenerate(guid);

    return make_smr<AccelerationEffect>(
        _STD move(guid),
        "test-render-effect",
        Vector<smr<Stage>> { _STD move(vertexStage), _STD move(fragmentStage) },
        Vector<smr<const Symbol>> {},
        Vector<smr<const Symbol>> { makeSceneColorSymbol() }
    );
}

smr<engine::gfx::acc::GraphicsPipeline> build_test_pipeline() {
    return nullptr;
}
