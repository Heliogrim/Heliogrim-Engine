#include "test.hpp"

#include <fstream>
#include <map>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Acc/AccelerationEffect.hpp>
#include <Engine.GFX.Acc.Compile/Spec/EffectSpecification.hpp>
#include <Engine.GFX.Acc/Stage/Stage.hpp>
#include <Engine.GFX.Acc/Stage/TransferToken.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.Material/Material.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX.Render.Predefined/Effects/BrdfIrradiance.hpp>
#include <Engine.GFX.Render.Predefined/Effects/BrdfLut.hpp>
#include <Engine.GFX.Render.Predefined/Effects/BrdfPrefilter.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/DeferredNormal.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/DeferredPosition.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Builder/BuilderVisitor.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>
#include <Engine.GFX.RenderGraph/Relation/Provision.hpp>
#include <Engine.GFX.RenderGraph.Compile/RenderGraphCompiler.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/Build/BaseBuilder.hpp>
#include <Engine.GFX.RenderPipeline/Build/StreamBuilder.hpp>
#include <Engine.GFX.RenderPipeline/State/State.hpp>
#include <Engine.Logging/Logger.hpp>

#include "CompileGraph.hpp"
#include "Builder/Builder.hpp"
#include "Component/BarrierComponent.hpp"
#include "Component/ProviderComponent.hpp"
#include "Component/Compile/AccelerationComponent.hpp"
#include "Component/Compile/ImmutableAccelerationComponent.hpp"
#include "Component/Compile/ImmutableCommandRecordComponent.hpp"
#include "Component/Compile/SceneCommandRecordComponent.hpp"
#include "Debug/DebugVisitor.hpp"
#include "Node/Compile/AccelerationSubpassNode.hpp"
#include "Node/Runtime/AnchorNode.hpp"
#include "Node/Runtime/BarrierNode.hpp"
#include "Node/Runtime/ProviderNode.hpp"
#include "Relation/MeshDescription.hpp"
#include "Relation/SceneViewDescription.hpp"
#include "Relation/TextureDescription.hpp"

using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

using AccToken = engine::gfx::acc::TransferToken;

/**/

auto make_test_graph() -> uptr<graph::CompileGraph>;

static _STD map<string, smr<graph::Description>> static_desc_map {};
static _STD map<string, smr<acc::Symbol>> static_symbol_map {};

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
    ref<_STD map<string, smr<acc::Symbol>>> map_
) {

    map_["sceneCamera"] = make_smr<acc::Symbol>(
        acc::Symbol {
            .scope = acc::SymbolScope {
                .type = acc::SymbolScopeType::eGlobal,
                .layer = ""
            },
            .name = "scene::camera",
            .description = desc_.at("camera"),
            .flags = acc::SymbolFlagBits::eUndefined,
            .token = AccToken {}
        }
    );

    /**/

    map_["brdf::albedo"] = make_smr<acc::Symbol>(
        acc::SymbolScope { acc::SymbolScopeType::eGlobal, "" },
        "brdf::albedo",
        desc_.at("linearColorBuffer"),
        acc::SymbolFlagBits::eUndefined,
        AccToken {}
    );
    map_["brdf::arm"] = make_smr<acc::Symbol>(
        acc::SymbolScope { acc::SymbolScopeType::eGlobal, "" },
        "brdf::arm",
        desc_.at("linearColorBuffer"),
        acc::SymbolFlagBits::eUndefined,
        AccToken {}
    );

    // eR16G16Sfloat
    map_["brdf::lut"] = make_smr<acc::Symbol>(
        acc::SymbolScope { acc::SymbolScopeType::eGlobal, "" },
        "brdf::lut",
        desc_.at("v2hfloatTextureBuffer"),
        acc::SymbolFlagBits::eUndefined,
        AccToken {}
    );
    map_["brdf::prefilter"] = make_smr<acc::Symbol>(
        acc::SymbolScope { acc::SymbolScopeType::eGlobal, "" },
        "brdf::prefilter",
        desc_.at("v4hfloatTextureBuffer"),
        acc::SymbolFlagBits::eUndefined,
        AccToken {}
    );
    map_["brdf::irradiance"] = make_smr<acc::Symbol>(
        acc::SymbolScope { acc::SymbolScopeType::eGlobal, "" },
        "brdf::irradiance",
        desc_.at("v4floatTextureBuffer"),
        acc::SymbolFlagBits::eUndefined,
        AccToken {}
    );
}

void setup_graph_expectation(cref<uptr<graph::CompileGraph>> graph_, mref<smr<const acc::Symbol>> ocs_) {
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

auto make_accel_vertex_l0_stage() -> uptr<acc::Stage> {

    auto flag = acc::StageFlagBits::eVertex;
    auto inputs = Vector<acc::StageInput> {};
    auto outputs = Vector<acc::StageOutput> {};

    /**/

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("geometry/position"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("geometry/uvm"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("viewProj"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eF32Mat4x4,
            .rate = acc::DataInputRate::ePerPassInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("model"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eF32Mat4x4,
            .rate = acc::DataInputRate::ePerInstantiation
        }
    );

    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("uvm"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec4,
            .rate = {}
        }
    );

    /**/

    auto stage = make_uptr<acc::Stage>(
        _STD move(flag),
        _STD move(inputs),
        _STD move(outputs)
    );

    /**/

    auto inter = make_smr<acc::lang::Intermediate>();

    auto il = make_uptr<acc::lang::IL>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\depthpass_static.vs)");
    auto snippet = string { code.begin(), code.end() };

    il->_dialect = acc::lang::ILDialect::eVulkanGlsl;
    il->_snippets.push_back(_STD move(snippet));

    inter->setIl(_STD move(il));

    auto ir = make_uptr<acc::lang::IR>();
    inter->setIr(_STD move(ir));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_vertex_l1_stage() -> uptr<acc::Stage> {

    auto flag = acc::StageFlagBits::eVertex;
    auto inputs = Vector<acc::StageInput> {};
    auto outputs = Vector<acc::StageOutput> {};

    /**/

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("geometry/position"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("geometry/uvm"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("geometry/normal"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );

    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("position"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec4,
            .rate = {}
        }
    );
    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("normal"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec4,
            .rate = {}
        }
    );
    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("uvm"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec4,
            .rate = {}
        }
    );

    /**/

    auto stage = make_uptr<acc::Stage>(
        _STD move(flag),
        _STD move(inputs),
        _STD move(outputs)
    );

    /**/

    auto inter = make_smr<acc::lang::Intermediate>();

    auto il = make_uptr<acc::lang::IL>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\mainpass_static.vs)");
    auto snippet = string { code.begin(), code.end() };

    il->_dialect = acc::lang::ILDialect::eVulkanGlsl;
    il->_snippets.push_back(_STD move(snippet));

    inter->setIl(_STD move(il));

    auto ir = make_uptr<acc::lang::IR>();
    inter->setIr(_STD move(ir));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_vertex_l2_stage() -> uptr<acc::Stage> {

    auto flag = acc::StageFlagBits::eVertex;
    auto inputs = Vector<acc::StageInput> {};
    auto outputs = Vector<acc::StageOutput> {};

    /**/

    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("uv"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec2,
            .rate = {}
        }
    );
    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("color"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = {}
        }
    );

    /**/

    auto stage = make_uptr<acc::Stage>(
        _STD move(flag),
        _STD move(inputs),
        _STD move(outputs)
    );

    /**/

    auto inter = make_smr<acc::lang::Intermediate>();

    auto il = make_uptr<acc::lang::IL>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\finalpass.vs)");
    auto snippet = string { code.begin(), code.end() };

    il->_dialect = acc::lang::ILDialect::eVulkanGlsl;
    il->_snippets.push_back(_STD move(snippet));

    inter->setIl(_STD move(il));

    auto ir = make_uptr<acc::lang::IR>();
    inter->setIr(_STD move(ir));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_fragment_l0_stage() -> uptr<acc::Stage> {

    auto stageModule = make_smr<acc::StageModule>();
    auto flag = acc::StageFlagBits::eFragment;
    auto inputs = Vector<acc::StageInput> {};
    auto outputs = Vector<acc::StageOutput> {};

    /**/

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("uvm"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec4,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("marker"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInstantiation
        }
    );

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("dpmm"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eStorage,
            .rate = acc::DataInputRate::ePerInstantiation
        }
    );

    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("data"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eU32,
            .rate = {}
        }
    );

    /**/

    auto stage = make_uptr<acc::Stage>(
        _STD move(flag),
        _STD move(inputs),
        _STD move(outputs)
    );

    /**/

    auto inter = make_smr<acc::lang::Intermediate>();

    auto il = make_uptr<acc::lang::IL>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\depthpass_static.frag)");
    auto snippet = string { code.begin(), code.end() };

    il->_dialect = acc::lang::ILDialect::eVulkanGlsl;
    il->_snippets.push_back(_STD move(snippet));

    inter->setIl(_STD move(il));

    auto ir = make_uptr<acc::lang::IR>();
    inter->setIr(_STD move(ir));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_fragment_l1_stage() -> uptr<acc::Stage> {

    auto stageModule = make_smr<acc::StageModule>();
    auto flag = acc::StageFlagBits::eFragment;
    auto inputs = Vector<acc::StageInput> {};
    auto outputs = Vector<acc::StageOutput> {};

    /**/

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("position"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec4,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("normal"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec4,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("uvm"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("material"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eUniform,
            .rate = acc::DataInputRate::ePerInstantiation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("albedo"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("normal"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("roughness"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("metalness"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("ao"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );

    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("albedo"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eU8Vec4,
            .rate = {}
        }
    );
    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("normal"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = {}
        }
    );
    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("position"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec4,
            .rate = {}
        }
    );
    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("arm"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eU8Vec4,
            .rate = {}
        }
    );

    /**/

    auto stage = make_uptr<acc::Stage>(
        _STD move(flag),
        _STD move(inputs),
        _STD move(outputs)
    );

    /**/

    auto inter = make_smr<acc::lang::Intermediate>();

    auto il = make_uptr<acc::lang::IL>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\mainpass_static.frag)");
    auto snippet = string { code.begin(), code.end() };

    il->_dialect = acc::lang::ILDialect::eVulkanGlsl;
    il->_snippets.push_back(_STD move(snippet));

    inter->setIl(_STD move(il));

    auto ir = make_uptr<acc::lang::IR>();
    inter->setIr(_STD move(ir));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_fragment_l2_stage() -> uptr<acc::Stage> {

    auto stageModule = make_smr<acc::StageModule>();
    auto flag = acc::StageFlagBits::eFragment;
    auto inputs = Vector<acc::StageInput> {};
    auto outputs = Vector<acc::StageOutput> {};

    /**/

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("uv"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec2,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("color"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eF32Vec3,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );

    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("camera"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eUniform,
            .rate = acc::DataInputRate::ePerInstantiation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("albedo"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("normal"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("position"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("arm"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("depth"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("brdf-lut"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("prefilter"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );
    inputs.push_back(
        acc::StageInput {
            .token = AccToken::from("irradiance"),
            .transferType = acc::TransferType::eBinding,
            .dataType = acc::TransferDataType::eSampler,
            .rate = acc::DataInputRate::ePerInvocation
        }
    );

    outputs.push_back(
        acc::StageOutput {
            .token = AccToken::from("color"),
            .transferType = acc::TransferType::eForward,
            .dataType = acc::TransferDataType::eU8Vec4,
            .rate = {}
        }
    );

    /**/

    auto stage = make_uptr<acc::Stage>(
        _STD move(flag),
        _STD move(inputs),
        _STD move(outputs)
    );

    /**/

    auto inter = make_smr<acc::lang::Intermediate>();

    auto il = make_uptr<acc::lang::IL>();

    auto code = read_shader_file(R"(R:\Development\C++\Vulkan API\Game\resources\shader\finalpass.frag)");
    auto snippet = string { code.begin(), code.end() };

    il->_dialect = acc::lang::ILDialect::eVulkanGlsl;
    il->_snippets.push_back(_STD move(snippet));

    inter->setIl(_STD move(il));

    auto ir = make_uptr<acc::lang::IR>();
    inter->setIr(_STD move(ir));

    stage->setIntermediate(_STD move(inter));

    /**/

    return stage;
}

auto make_accel_l0_effect() -> uptr<acc::AccelerationEffect> {

    Vector<smr<acc::Stage>> stages {};
    Vector<smr<const acc::Symbol>> imports {};
    Vector<smr<const acc::Symbol>> exports {};

    /**/

    stages.push_back(make_smr(make_accel_vertex_l0_stage()));
    stages.push_back(make_smr(make_accel_fragment_l0_stage()));

    imports.push_back(
        make_smr<acc::Symbol>(
            acc::Symbol {
                .scope = acc::SymbolScope {
                    .type = acc::SymbolScopeType::eGlobal,
                    .layer = ""
                },
                .name = "camera",
                .description = static_desc_map.at("camera"),
                .flags = acc::SymbolFlagBits::eUndefined,
                .token = AccToken::from("viewProj")
            }
        )
    );

    imports.push_back(
        make_smr<acc::Symbol>(
            acc::Symbol {
                .scope = acc::SymbolScope {
                    .type = acc::SymbolScopeType::eLocal,
                    .layer = ""
                },
                .name = "camera",
                .description = static_desc_map.at(""),
                .flags = acc::SymbolFlagBits::eUndefined,
                .token = AccToken::from("viewProj")
            }
        )
    );

    /**/

    Guid guid;
    GuidGenerate(guid);
    string name = encodeGuid4228(guid);

    auto effect = make_uptr<acc::AccelerationEffect>(
        _STD move(guid),
        _STD move(name),
        _STD move(stages),
        _STD move(imports),
        _STD move(exports)
    );

    return effect;
}

auto make_accel_l1_effect() -> uptr<acc::AccelerationEffect> {

    Vector<smr<acc::Stage>> stages {};
    Vector<smr<const acc::Symbol>> imports {};
    Vector<smr<const acc::Symbol>> exports {};

    /**/

    stages.push_back(make_smr(make_accel_vertex_l1_stage()));
    stages.push_back(make_smr(make_accel_fragment_l1_stage()));

    /**/

    Guid guid;
    GuidGenerate(guid);
    string name = encodeGuid4228(guid);

    auto effect = make_uptr<acc::AccelerationEffect>(
        _STD move(guid),
        _STD move(name),
        _STD move(stages),
        _STD move(imports),
        _STD move(exports)
    );

    return effect;
}

auto make_accel_l2_effect() -> uptr<acc::AccelerationEffect> {

    Vector<smr<acc::Stage>> stages {};
    Vector<smr<const acc::Symbol>> imports {};
    Vector<smr<const acc::Symbol>> exports {};

    /**/

    stages.push_back(make_smr(make_accel_vertex_l2_stage()));
    stages.push_back(make_smr(make_accel_fragment_l2_stage()));

    /**/

    Guid guid;
    GuidGenerate(guid);
    string name = encodeGuid4228(guid);

    auto effect = make_uptr<acc::AccelerationEffect>(
        _STD move(guid),
        _STD move(name),
        _STD move(stages),
        _STD move(imports),
        _STD move(exports)
    );

    return effect;
}

auto make_material_prototype() -> uptr<material::MaterialPrototype> {

    Set<smr<acc::AccelerationEffect>> effects {};
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
        Vector<smr<acc::AccelerationEffect>> {},
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

    Vector<smr<acc::Symbol>> exposedSymbols {};
    Vector<smr<acc::Symbol>> isolatedSymbols {};

    _STD map<string, smr<graph::Description>> descriptions {};
    setup_descriptions(descriptions);

    std::map<string, smr<acc::Symbol>> symbols {};
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

        auto accelComp = make_uptr<graph::CompileImmutableAccelerationComponent>();
        auto recordComp = make_uptr<graph::CompileSceneCommandRecordComponent>();

        // As we expect to have multiple materials, we need to progressively use the depth buffer to refine
        accelComp->addTransform(makeSceneDepthSymbol());

        auto node = make_smr<graph::CompileAccelerationSubpassNode>(_STD move(accelComp), _STD move(recordComp));

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
            auto accelComp = make_uptr<graph::CompileImmutableAccelerationComponent>();
            auto recordComp = make_uptr<graph::CompileImmutableCommandRecordComponent>();

            accelComp->addOutput(symbols.at("brdf::lut"));
            recordComp->setInitializer(
                [
                    effect = makeBrdfLutEffect()
                ](mref<nmpt<cmd::RenderCommandBuffer>> cmd_) {
                    cmd_->begin();
                    cmd_->bindEffect(effect.get());
                    //cmd_->bindStaticMesh(...);
                    cmd_->drawStaticMesh(1uL, 0uL, 2uL, 0uL);
                    cmd_->end();
                }
            );

            auto node = make_smr<graph::CompileAccelerationSubpassNode>(_STD move(accelComp), _STD move(recordComp));

            auto prevIt = it;
            it = node.get();

            graph = graph::Builder::insertNode(_STD move(graph), prevIt, end, _STD move(node));
        }

        /**/

        {
            auto accelComp = make_uptr<graph::CompileImmutableAccelerationComponent>();
            auto recordComp = make_uptr<graph::CompileImmutableCommandRecordComponent>();

            accelComp->addOutput(symbols.at("brdf::prefilter"));
            recordComp->setInitializer(
                [effect = makeBrdfPrefilterEffect()](mref<nmpt<cmd::RenderCommandBuffer>> cmd_) {
                    cmd_->begin();
                    cmd_->bindEffect(effect.get());
                    // TBD: Bind 6 cubic camera matrices
                    cmd_->bindStorage(nullptr);
                    // TBD: Bind roughness and samples
                    cmd_->bindStorage(nullptr);
                    // TBD: Bind skybox cube texture
                    cmd_->bindTexture(static_cast<ptr<const TextureView>>(nullptr));
                    cmd_->drawStaticMesh(1uL, 0uL, 36uL, 0uL);
                    cmd_->end();
                }
            );

            auto node = make_smr<graph::CompileAccelerationSubpassNode>(_STD move(accelComp), _STD move(recordComp));

            auto prevIt = it;
            it = node.get();

            graph = graph::Builder::insertNode(_STD move(graph), prevIt, end, _STD move(node));
        }

        /**/

        {
            auto accelComp = make_uptr<graph::CompileImmutableAccelerationComponent>();
            auto recordComp = make_uptr<graph::CompileImmutableCommandRecordComponent>();

            accelComp->addInput(symbols.at("brdf::prefilter"));
            accelComp->addOutput(symbols.at("brdf::irradiance"));
            recordComp->setInitializer(
                [effect = makeBrdfIrradianceEffect()](mref<nmpt<cmd::RenderCommandBuffer>> cmd_) {
                    cmd_->begin();
                    cmd_->bindEffect(effect.get());
                    // TBD: Bind 6 cubic camera matrices
                    cmd_->bindStorage(nullptr);
                    // TBD: Bind prefiltered
                    cmd_->bindTexture(static_cast<ptr<const TextureView>>(nullptr));
                    // TBD: Bind delta phi and delta theta
                    cmd_->bindStorage(nullptr);
                    cmd_->drawStaticMesh(1uL, 0uL, 5uL, 0uL);
                    cmd_->end();
                }
            );

            auto node = make_smr<graph::CompileAccelerationSubpassNode>(_STD move(accelComp), _STD move(recordComp));

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

        auto accelComp = make_uptr<graph::CompileImmutableAccelerationComponent>();
        auto recordComp = make_uptr<graph::CompileSceneCommandRecordComponent>();

        // We want to use the pre-populated depth buffer for solid geometry
        accelComp->addInput(makeSceneDepthSymbol());

        // @see sceneDepth | We expect to have multiple materials
        accelComp->addTransform(symbols.at("brdf::albedo"));
        accelComp->addTransform(symbols.at("brdf::arm"));
        accelComp->addTransform(makeDeferredNormalSymbol());
        accelComp->addTransform(makeDeferredPositionSymbol());

        auto node = make_smr<graph::CompileAccelerationSubpassNode>(_STD move(accelComp), _STD move(recordComp));

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

        auto accelComp = make_uptr<graph::CompileImmutableAccelerationComponent>();
        auto recordComp = make_uptr<graph::CompileSceneCommandRecordComponent>();

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

        auto node = make_smr<graph::CompileAccelerationSubpassNode>(_STD move(accelComp), _STD move(recordComp));

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

        auto accelComp = make_uptr<graph::CompileImmutableAccelerationComponent>();
        auto recordComp = make_uptr<graph::CompileSceneCommandRecordComponent>();

        accelComp->addTransform(makeSceneColorSymbol());

        auto node = make_smr<graph::CompileAccelerationSubpassNode>(_STD move(accelComp), _STD move(recordComp));

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
