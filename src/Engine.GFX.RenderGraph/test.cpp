#include "test.hpp"

#include <fstream>
#include <map>
#include <Engine.Accel.Pipeline/Stage/StageModule.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.Material/Material.hpp>
#include <Engine.GFX.Material/MaterialPrototype.hpp>
#include <Engine.GFX.Material/MaterialParameter.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBufferBase.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/DeferredNormal.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/BuilderVisitor.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>
#include <Engine.GFX.RenderGraph/Relation/Provision.hpp>
#include <Engine.GFX.RenderGraph.Compile/RenderGraphCompiler.hpp>
#include <Engine.Resource/Manage/UniqueResource.hpp>
#include <Engine.Logging/Logger.hpp>

#include "CompileGraph.hpp"
#include "Builder/Builder.hpp"
#include "Component/BarrierComponent.hpp"
#include "Component/ProviderComponent.hpp"
#include "Debug/DebugVisitor.hpp"
#include "Node/Runtime/AnchorNode.hpp"
#include "Node/Runtime/BarrierNode.hpp"
#include "Node/Runtime/ProviderNode.hpp"
#include "Node/Compile/CompileSubPassNode.hpp"
#include "Relation/MeshDescription.hpp"
#include "Relation/SceneViewDescription.hpp"
#include "Relation/TextureDescription.hpp"

using namespace hg::engine::render;
using namespace hg::engine::gfx;
using namespace hg;

/**/

auto make_test_graph() -> uptr<graph::CompileGraph>;

static _STD map<string, smr<graph::Description>> static_desc_map {};
static _STD map<string, smr<graph::Symbol>> static_symbol_map {};

void hg::test_graph_process() {

    /* Prepare */

    IM_CORE_LOG("Creating Graphs to process...");
    const auto compileGraph = make_smr<graph::CompileGraph>(make_test_graph());

    IM_CORE_LOG("Compile graph to runtime graph...");
    graph::RenderGraphCompiler rgc {};

    /* Initial */

    auto runtimeGraph = rgc(
        graph::CompileRequest {
            compileGraph, { makeSceneColorSymbol() }
        }
    );

    IM_CORE_LOG("Building render pipeline from runtime graph...");
    //const render::pipeline::BaseBuilder baseBuilder {};
    //auto pipe = baseBuilder(_STD move(runtimeGraph));

    /* Iterative Update */

    //const render::pipeline::StreamBuilder streamBuilder {};

    //while (true) {
    //
    //    auto nextRuntimeGraph = rgc(
    //        graph::CompileRequest {
    //            compileGraph, { makeSceneColorSymbol() }
    //        }
    //    );
    //
    //    /**/
    //
    //    IM_CORE_LOG("Iterative update of render pipeline with next runtime graph...");
    //    pipe = streamBuilder(_STD move(pipe), _STD move(nextRuntimeGraph));
    //
    //    IM_CORE_LOG("Executing updated render pipeline...");
    //    pipe->operator()(nullptr);
    //}
}

void setup_descriptions(ref<_STD map<string, smr<graph::Description>>> map_) {

    map_["camera"] = make_smr<>(new graph::SceneViewDescription()).into<graph::Description>();
    map_["depthBuffer"] = make_smr<graph::TextureDescription>(
        new graph::TextureDescription(
            graph::TextureDescription {
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                { graph::DescriptionValueMatchingMode::eCovariant, engine::gfx::TextureFormat::eD32Sfloat },
                { graph::DescriptionValueMatchingMode::eInvariant, 1ui32 },
                {}
            }
        )
    ).into<graph::Description>();
    map_["linearColorBuffer"] = make_smr<graph::TextureDescription>(
        new graph::TextureDescription(
            graph::TextureDescription {
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR8G8B8A8Unorm },
                { graph::DescriptionValueMatchingMode::eCovariant, 1ui32 },
                {}
            }
        )
    ).into<graph::Description>();
    map_["displayColorBuffer"] = make_smr<graph::TextureDescription>(
        new graph::TextureDescription(
            graph::TextureDescription {
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eB8G8R8A8Unorm },
                { graph::DescriptionValueMatchingMode::eCovariant, 1ui32 },
                {}
            }
        )
    ).into<graph::Description>();

    /**/

    map_["v2hfloatTextureBuffer"] = make_smr<graph::TextureDescription>(
        new graph::TextureDescription(
            graph::TextureDescription {
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR16G16Sfloat },
                { graph::DescriptionValueMatchingMode::eInvariant, 1ui32 },
                {}
            }
        )
    ).into<graph::Description>();

    map_["v4hfloatTextureBuffer"] = make_smr<graph::TextureDescription>(
        new graph::TextureDescription(
            graph::TextureDescription {
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR16G16B16A16Sfloat },
                { graph::DescriptionValueMatchingMode::eInvariant, 1ui32 },
                {}
            }
        )
    ).into<graph::Description>();

    map_["v4floatTextureBuffer"] = make_smr<graph::TextureDescription>(
        new graph::TextureDescription(
            graph::TextureDescription {
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR32G32B32A32Sfloat },
                { graph::DescriptionValueMatchingMode::eInvariant, 1ui32 },
                {}
            }
        )
    ).into<graph::Description>();
}

void setup_symbols(
    cref<_STD map<string, smr<graph::Description>>> desc_,
    ref<_STD map<string, smr<graph::Symbol>>> map_
) {

    map_["sceneCamera"] = make_smr<graph::Symbol>(
        graph::Symbol {
            .flags = graph::SymbolFlagBits::eUndefined,
            .scope = graph::SymbolScope {
                .type = graph::SymbolScopeType::eGlobal,
                .layer = ""
            },
            .name = "scene::camera",
            .description = desc_.at("camera")
        }
    );

    /**/

    map_["brdf::albedo"] = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope { graph::SymbolScopeType::eGlobal, "" },
        "brdf::albedo",
        desc_.at("linearColorBuffer")
    );
    map_["brdf::arm"] = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope { graph::SymbolScopeType::eGlobal, "" },
        "brdf::arm",
        desc_.at("linearColorBuffer")
    );

    // eR16G16Sfloat
    map_["brdf::lut"] = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope { graph::SymbolScopeType::eGlobal, "" },
        "brdf::lut",
        desc_.at("v2hfloatTextureBuffer")
    );
    map_["brdf::prefilter"] = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope { graph::SymbolScopeType::eGlobal, "" },
        "brdf::prefilter",
        desc_.at("v4hfloatTextureBuffer")
    );
    map_["brdf::irradiance"] = make_smr<graph::Symbol>(
        graph::SymbolFlagBits::eUndefined,
        graph::SymbolScope { graph::SymbolScopeType::eGlobal, "" },
        "brdf::irradiance",
        desc_.at("v4floatTextureBuffer")
    );
}

void setup_graph_expectation(cref<uptr<graph::CompileGraph>> graph_, mref<smr<const graph::Symbol>> ocs_) {
    graph_->addExpectedProvision(graph::Provision { _STD move(ocs_) });
}

inline _STD vector<char> read_shader_file(const char* file_) {
    _STD vector<char> spirv {};

    _STD ifstream in {};
    in.open(file_, _STD ios::ate | _STD ios::in | _STD ios::binary);

    assert(in.is_open());

    const size_t size = in.tellg();
    spirv.resize(size, 0);

    in.seekg(0, _STD ios::beg);
    in.read(spirv.data(), size);

    in.close();
    return spirv;
}

auto make_accel_vertex_l0_stage() -> uptr<engine::accel::Stage> {

    auto flag = engine::accel::StageFlagBits::eVertex;

    /**/

    auto stage = make_uptr<engine::accel::Stage>(
        _STD move(flag)
    );

    /**/

    auto inter = make_smr<engine::accel::lang::Intermediate>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\depthpass_static.vs)");
    auto snippet = string { code.begin(), code.end() };

    inter->lang.dialect = engine::accel::lang::Dialect::eVulkanGlsl460;
    inter->lang.text.emplace_back(_STD move(snippet));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_vertex_l1_stage() -> uptr<engine::accel::Stage> {

    auto flag = engine::accel::StageFlagBits::eVertex;

    /**/

    auto stage = make_uptr<engine::accel::Stage>(
        _STD move(flag)
    );

    /**/

    auto inter = make_smr<engine::accel::lang::Intermediate>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\mainpass_static.vs)");
    auto snippet = string { code.begin(), code.end() };

    inter->lang.dialect = engine::accel::lang::Dialect::eVulkanGlsl460;
    inter->lang.text.emplace_back(_STD move(snippet));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_vertex_l2_stage() -> uptr<engine::accel::Stage> {

    auto flag = engine::accel::StageFlagBits::eVertex;

    /**/

    auto stage = make_uptr<engine::accel::Stage>(
        _STD move(flag)
    );

    /**/

    auto inter = make_smr<engine::accel::lang::Intermediate>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\finalpass.vs)");
    auto snippet = string { code.begin(), code.end() };

    inter->lang.dialect = engine::accel::lang::Dialect::eVulkanGlsl460;
    inter->lang.text.emplace_back(_STD move(snippet));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_fragment_l0_stage() -> uptr<engine::accel::Stage> {

    auto stageModule = make_smr<engine::accel::StageModule>();
    auto flag = engine::accel::StageFlagBits::eFragment;

    /**/

    auto stage = make_uptr<engine::accel::Stage>(
        _STD move(flag)
    );

    /**/

    auto inter = make_smr<engine::accel::lang::Intermediate>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\depthpass_static.frag)");
    auto snippet = string { code.begin(), code.end() };

    inter->lang.dialect = engine::accel::lang::Dialect::eVulkanGlsl460;
    inter->lang.text.emplace_back(_STD move(snippet));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_fragment_l1_stage() -> uptr<engine::accel::Stage> {

    auto stageModule = make_smr<engine::accel::StageModule>();
    auto flag = engine::accel::StageFlagBits::eFragment;

    /**/

    auto stage = make_uptr<engine::accel::Stage>(
        _STD move(flag)
    );

    /**/

    auto inter = make_smr<engine::accel::lang::Intermediate>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\mainpass_static.frag)");
    auto snippet = string { code.begin(), code.end() };

    inter->lang.dialect = engine::accel::lang::Dialect::eVulkanGlsl460;
    inter->lang.text.emplace_back(_STD move(snippet));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_fragment_l2_stage() -> uptr<engine::accel::Stage> {

    auto stageModule = make_smr<engine::accel::StageModule>();
    auto flag = engine::accel::StageFlagBits::eFragment;

    /**/

    auto stage = make_uptr<engine::accel::Stage>(
        _STD move(flag)
    );

    /**/

    auto inter = make_smr<engine::accel::lang::Intermediate>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\finalpass.frag)");
    auto snippet = string { code.begin(), code.end() };

    inter->lang.dialect = engine::accel::lang::Dialect::eVulkanGlsl460;
    inter->lang.text.emplace_back(_STD move(snippet));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_l0_effect() -> uptr<engine::accel::AccelerationEffect> {

    Vector<smr<engine::accel::Stage>> stages {};

    /**/

    stages.push_back(make_smr(make_accel_vertex_l0_stage()));
    stages.push_back(make_smr(make_accel_fragment_l0_stage()));

    /**/

    Guid guid;
    GuidGenerate(guid);
    string name = encodeGuid4228(guid);

    auto effect = make_uptr<engine::accel::AccelerationEffect>(
        _STD move(guid),
        _STD move(name),
        _STD move(stages)
    );

    return effect;
}

auto make_accel_l1_effect() -> uptr<engine::accel::AccelerationEffect> {

    Vector<smr<engine::accel::Stage>> stages {};

    /**/

    stages.push_back(make_smr(make_accel_vertex_l1_stage()));
    stages.push_back(make_smr(make_accel_fragment_l1_stage()));

    /**/

    Guid guid;
    GuidGenerate(guid);
    string name = encodeGuid4228(guid);

    auto effect = make_uptr<engine::accel::AccelerationEffect>(
        _STD move(guid),
        _STD move(name),
        _STD move(stages)
    );

    return effect;
}

auto make_accel_l2_effect() -> uptr<engine::accel::AccelerationEffect> {

    Vector<smr<engine::accel::Stage>> stages {};

    /**/

    stages.push_back(make_smr(make_accel_vertex_l2_stage()));
    stages.push_back(make_smr(make_accel_fragment_l2_stage()));

    /**/

    Guid guid;
    GuidGenerate(guid);
    string name = encodeGuid4228(guid);

    auto effect = make_uptr<engine::accel::AccelerationEffect>(
        _STD move(guid),
        _STD move(name),
        _STD move(stages)
    );

    return effect;
}

auto make_material_prototype() -> uptr<material::MaterialPrototype> {

    Set<smr<engine::accel::AccelerationEffect>> effects {};
    effects.insert(make_smr(make_accel_l0_effect()));
    effects.insert(make_smr(make_accel_l1_effect()));
    effects.insert(make_smr(make_accel_l2_effect()));

    /**/

    Guid guid;
    GuidGenerate(guid);
    string name = encodeGuid4228(guid);

    auto proto = make_uptr<material::MaterialPrototype>(
        _STD move(guid),
        _STD move(name),
        InlineAutoArray<material::MaterialEffect> {},
        Vector<material::MaterialPrototypeParameter> {}
    );

    return proto;
}

auto make_material() -> smr<MaterialResource> {

    auto proto = make_smr(make_material_prototype());

    /**/

    Guid guid;
    GuidGenerate(guid);

    return make_smr<engine::resource::UniqueResource<material::Material>>(
        _STD move(guid),
        _STD move(proto),
        Vector<material::MaterialParameter> {}
    ).into<MaterialResource>();
}

auto make_test_graph() -> uptr<graph::CompileGraph> {

    uptr<graph::CompileGraph> graph = make_uptr<graph::CompileGraph>();
    const nmpt<graph::Node> begin = graph->begin().get();
    const nmpt<graph::Node> end = graph->end().get();

    nmpt<graph::Node> cursor { begin };

    /**/

    // SceneViewDescription ~ Camera { position, rotation, fov, aspect, ... }
    // MeshDescription ~ { primitive types, topology, static / skeletal, procedural ?!?, ... }
    // TextureDescription ~ { type, format, layer, mip stack, ... }
    // ModelDescription ~ { position, rotation, scale, materials ?!?, ... }

    /**/

    // TODO: renderer or outer context should provide a list of target symbols to expose externally
    // TODO: in addition we could define a list of symbols carried/looped back by the instantiated render-pass
    // Renderer :: exposedSymbols() -> List < Symbol >
    // Renderer :: isolatedSymbols() -> List < Symbol >
    // e.g. List < Color Symbol, ... > + List < Depth Symbol, ... >

    Vector<smr<graph::Symbol>> exposedSymbols {};
    Vector<smr<graph::Symbol>> isolatedSymbols {};

    _STD map<string, smr<graph::Description>> descriptions {};
    setup_descriptions(descriptions);

    std::map<string, smr<graph::Symbol>> symbols {};
    setup_symbols(descriptions, symbols);

    /**/

    setup_graph_expectation(graph, makeSceneColorSymbol());

    /**/

    {
        auto provider = make_smr<graph::ProviderNode>();
        auto comp = provider->getProviderComponent();

        comp->setProvided(
            {
                graph::Provision { symbols.at("sceneCamera") }
            }
        );

        cursor = provider.get();
        graph = graph::Builder::insertNode(_STD move(graph), begin, end, _STD move(provider));
    }

    {
        auto barrier = make_smr<graph::BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Depth Barrier");

        auto it = barrier.get();
        graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto node = make_smr<graph::CompileSubPassNode>();

        // As we expect to have multiple materials, we need to progressively use the depth buffer to refine
        //accelComp->addTransform(makeSceneDepthSymbol());

        // TODO: subpassComp->addExpectedRequirement(graph::Requirement { symbols.at("sceneCamera") });
        // TODO: subpassComp->addExpectedProvision(graph::Provision { symbols.at("sceneDepth") });

        cursor = node.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(node));
    }

    {
        auto barrier = make_smr<graph::BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("BRDF Helper Barrier");

        graph::Node* it = barrier.get();
        graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        {
            auto node = make_smr<graph::CompileSubPassNode>();

            /*
            accelComp->addOutput(symbols.at("brdf::lut"));
            
            recordComp->setInitializer(
                [
                    pipeline = makeBrdfLutPipeline()
                ](mref<nmpt<cmd::RenderCommandBufferBase>> cmd_) {
                    cmd_->begin();
                    cmd_->bindGraphicsPipeline(smr { pipeline });
                    //cmd_->bindStaticMesh(...);
                    cmd_->drawStaticMesh(1uL, 0uL, 2uL, 0uL);
                    cmd_->end();
                }
            );
            */

            auto prevIt = it;
            it = node.get();

            graph = graph::Builder::insertNode(_STD move(graph), prevIt, end, _STD move(node));
        }

        /**/

        {
            auto node = make_smr<graph::CompileSubPassNode>();

            /*
            accelComp->addOutput(symbols.at("brdf::prefilter"));
            recordComp->setInitializer(
                [pipeline = makeBrdfPrefilterPipeline()](mref<nmpt<cmd::RenderCommandBufferBase>> cmd_) {
                    cmd_->begin();
                    cmd_->bindGraphicsPipeline(smr { pipeline });
                    // TBD: Bind 6 cubic camera matrices
                    cmd_->bindStorage(nullptr, nullptr);
                    // TBD: Bind roughness and samples
                    cmd_->bindStorage(nullptr, nullptr);
                    // TBD: Bind skybox cube texture
                    cmd_->bindTexture(nullptr, static_cast<ptr<const TextureView>>(nullptr));
                    cmd_->drawStaticMesh(1uL, 0uL, 36uL, 0uL);
                    cmd_->end();
                }
            );
             */

            auto prevIt = it;
            it = node.get();

            graph = graph::Builder::insertNode(_STD move(graph), prevIt, end, _STD move(node));
        }

        /**/

        {
            auto node = make_smr<graph::CompileSubPassNode>();

            /*
            accelComp->addInput(symbols.at("brdf::prefilter"));
            accelComp->addOutput(symbols.at("brdf::irradiance"));
            recordComp->setInitializer(
                [pipeline = makeBrdfIrradiancePipeline()](mref<nmpt<cmd::RenderCommandBufferBase>> cmd_) {
                    cmd_->begin();
                    cmd_->bindGraphicsPipeline(smr { pipeline });
                    // TBD: Bind 6 cubic camera matrices
                    cmd_->bindStorage(nullptr, nullptr);
                    // TBD: Bind prefiltered
                    cmd_->bindTexture(nullptr, static_cast<ptr<const TextureView>>(nullptr));
                    // TBD: Bind delta phi and delta theta
                    cmd_->bindStorage(nullptr, nullptr);
                    cmd_->drawStaticMesh(1uL, 0uL, 5uL, 0uL);
                    cmd_->end();
                }
            );
             */

            auto prevIt = it;
            it = node.get();

            graph = graph::Builder::insertNode(_STD move(graph), prevIt, end, _STD move(node));
        }
    }

    {
        auto barrier = make_smr<graph::BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Material Barrier");

        auto it = barrier.get();
        graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto node = make_smr<graph::CompileSubPassNode>();

        /*
        // We want to use the pre-populated depth buffer for solid geometry
        accelComp->addInput(makeSceneDepthSymbol());

        // @see sceneDepth | We expect to have multiple materials
        accelComp->addTransform(symbols.at("brdf::albedo"));
        accelComp->addTransform(symbols.at("brdf::arm"));
        accelComp->addTransform(makeDeferredNormalSymbol());
        accelComp->addTransform(makeDeferredPositionSymbol());
         */

        // TODO: subpassComp->addExpectedRequirement(graph::Requirement { symbols.at("sceneCamera") });
        // TODO: subpassComp->addExpectedRequirement(graph::Requirement { symbols.at("sceneDepth") });
        // TODO: subpassComp->addExpectedProvision(graph::Provision { symbols.at("sceneColor") });

        /**/

        cursor = node.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(node));
    }

    {
        auto barrier = make_smr<graph::BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Lighting Barrier");

        auto it = barrier.get();
        graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto node = make_smr<graph::CompileSubPassNode>();

        /*
        //
        accelComp->addInput(makeSceneDepthSymbol());
        //
        accelComp->addInput(symbols.at("brdf::albedo"));
        accelComp->addInput(symbols.at("brdf::arm"));
        accelComp->addInput(makeDeferredNormalSymbol());
        accelComp->addInput(makeDeferredPositionSymbol());
        //
        accelComp->addInput(symbols.at("brdf::lut"));
        accelComp->addInput(symbols.at("brdf::prefilter"));
        accelComp->addInput(symbols.at("brdf::irradiance"));
        //
        accelComp->addTransform(makeSceneColorSymbol());
         */

        cursor = node.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(node));
    }

    {
        auto barrier = make_smr<graph::BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Compositing Barrier");

        auto it = barrier.get();
        graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto node = make_smr<graph::CompileSubPassNode>();

        /*
        accelComp->addTransform(makeSceneColorSymbol());
        */

        // TODO: subpassComp->addExpectedRequirement(graph::Requirement { symbols.at("sceneColor") });
        // TODO: subpassComp->addExpectedProvision(graph::Provision { symbols.at("sceneColor") });

        cursor = node.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(node));
    }

    /**/

    {
        graph::DebugVisitor visitor {};
        graph->begin()->accept(visitor);
    }

    /**/
    static_desc_map = _STD move(descriptions);
    static_symbol_map = _STD move(symbols);
    /**/

    return graph;
}
