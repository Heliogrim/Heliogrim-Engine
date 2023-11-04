#include "__Test__Renderer.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.Acc/Stage/StageFlags.hpp>
#include <Engine.GFX.Acc/Stage/Stage.hpp>
#include <Engine.GFX.Acc/Symbol/Symbol.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/ImmutableAccelerationComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/ImmutableCommandRecordComponent.hpp>

#include <Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Compile/AccelerationSubpassNode.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

[[nodiscard]] static smr<engine::gfx::acc::AccelerationEffect> build_test_effect();

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

    /**/

    auto acceleration = make_uptr<graph::CompileImmutableAccelerationComponent>();
    auto record = make_uptr<graph::CompileImmutableCommandRecordComponent>();

    acceleration->addOutput(makeSceneColorSymbol());
    record->setInitializer(
        [
            effect = build_test_effect()
        ](mref<nmpt<cmd::RenderCommandBuffer>> cmd_) {
            cmd_->begin();
            cmd_->bindEffect(effect.get());
            cmd_->drawDispatch(1uL, 0uL, 3uL, 0uL);
            cmd_->end();
        }
    );

    /**/

    auto beforeBarrier = make_smr<graph::BarrierNode>();
    auto subpass = make_smr<graph::CompileAccelerationSubpassNode>(
        _STD move(acceleration),
        _STD move(record)
    );
    auto afterBarrier = make_smr<graph::BarrierNode>();

    /**/

    nmpt<graph::Node> nextCursor = afterBarrier.get();
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
        Vector<StageOutput> {}
    );

    auto fragmentStage = make_smr<Stage>(
        StageFlagBits::eFragment,
        Vector<StageInput> {},
        Vector<StageOutput> {}
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
            make_uptr<lang::IL>(lang::ILDialect::eVulkanGlsl, Vector<string> { _STD move(fragmentStage) }),
            nullptr
        )
    );

    /**/

    Guid guid;
    GuidGenerate(guid);

    return make_smr<AccelerationEffect>(
        _STD move(guid),
        "test-render-effect",
        Vector { _STD move(vertexStage), _STD move(fragmentStage) },
        Vector<smr<const Symbol>> {},
        Vector { makeSceneColorSymbol() }
    );
}
