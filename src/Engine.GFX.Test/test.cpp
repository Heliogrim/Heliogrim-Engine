#include "pch.h"

#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.Render/Builder/BaseBuilder.hpp>
#include <Engine.GFX.Render/Builder/BuilderLayerVisitor.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>
#include <Engine.GFX.RenderGraph/Component/Subpass/SubpassAccelComponent.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/__fwd.hpp>
#include <Engine.GFX.RenderPipeline/State/State.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Logging/Logger.hpp>

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

auto make_test_graph() -> uptr<graph::CompileGraph>;

TEST(___Debug___, Check) {

    IM_CORE_LOG("Creating Graphs to process...");
    uptr<graph::CompileGraph> graph = make_test_graph();

    /**/

    IM_CORE_LOG("Building render pipeline from compile graph...");
    const BaseBuilder builder {};
    auto pipe = builder(graph.get());

    IM_CORE_LOG("Executing render pipeline...");
    pipe->operator()(nullptr);
}

#include <Engine.GFX.Acc.Compile/Spec/SimpleSpecificationStorage.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>
#include <Engine.GFX.Acc/Stage/TransferToken.hpp>
#include <Engine.GFX.Material/Material.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.GFX.RenderGraph/Builder/BuilderVisitor.hpp>
#include <Engine.GFX.RenderGraph/Component/AnchorComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/BarrierComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/ProviderComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/Compile/CompileSubpassComponent.hpp>
#include <Engine.GFX.RenderGraph/Debug/DebugVisitor.hpp>
#include <Engine.GFX.RenderGraph/Node/AnchorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Compile/CompileSubpassNode.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/ModelDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/Provision.hpp>
#include <Engine.GFX.RenderGraph/Relation/Requirement.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneViewDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Visitor/MaterialVisitor.hpp>

using AccToken = engine::gfx::acc::TransferToken;

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
}

void setup_symbols(
    cref<_STD map<string, smr<graph::Description>>> desc_,
    ref<_STD map<string, smr<acc::Symbol>>> map_
) {

    map_["sceneColor"] = make_smr<acc::Symbol>(
        acc::Symbol {
            .scope = acc::SymbolScope {
                .type = acc::SymbolScopeType::eGlobal,
                .layer = ""
            },
            .name = "scene::color",
            .description = desc_.at("displayColorBuffer"),
            .flags = acc::SymbolFlagBits::eUndefined,
            .token = AccToken {}
        }
    );
    map_["sceneDepth"] = make_smr<acc::Symbol>(
        acc::Symbol {
            .scope = acc::SymbolScope {
                .type = acc::SymbolScopeType::eGlobal,
                .layer = ""
            },
            .name = "scene::depth",
            .description = desc_.at("depthBuffer"),
            .flags = acc::SymbolFlagBits::eUndefined,
            .token = AccToken {}
        }
    );
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
}

void setup_graph_expectation(cref<uptr<graph::CompileGraph>> graph_, mref<smr<acc::Symbol>> ocs_) {
    graph_->addExpectedProvision(graph::Provision { _STD move(ocs_) });
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

    _STD map<string, smr<acc::Symbol>> symbols {};
    setup_symbols(descriptions, symbols);

    /**/

    setup_graph_expectation(graph, clone(symbols.at("sceneColor")));

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

        auto subpassNode = make_smr<graph::CompileSubpassNode>(graph::SubpassAccelMode::eSingle);
        auto subpassComp = subpassNode->getSubpassComponent();

        cursor = subpassNode.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    {
        auto barrier = make_smr<graph::BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Material Barrier");

        auto it = barrier.get();
        graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<graph::CompileSubpassNode>(graph::SubpassAccelMode::eMaterial);
        auto subpassComp = subpassNode->getSubpassComponent();

        /**/

        cursor = subpassNode.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    {
        auto barrier = make_smr<graph::BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Compositing Barrier");

        auto it = barrier.get();
        graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<graph::CompileSubpassNode>(graph::SubpassAccelMode::eMulti);
        auto subpassComp = subpassNode->getSubpassComponent();

        cursor = subpassNode.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    /**/

    {
        graph::DebugVisitor visitor {};
        graph->begin()->accept(visitor);
    }

    /**/

    smr<engine::gfx::MaterialResource> mat0 {}, mat1 {}, mat2 {};

    mat0 = nullptr;

    Guid guid;
    GuidGenerate(guid);
    mat1 = make_smr<engine::resource::UniqueResource<material::Material>>(
        _STD move(guid),
        smr<material::MaterialPrototype> { nullptr },
        Vector<material::MaterialParameter> {}
    ).into<MaterialResource>();

    GuidGenerate(guid);
    mat2 = make_smr<engine::resource::UniqueResource<material::Material>>(
        _STD move(guid),
        smr<material::MaterialPrototype> { nullptr },
        Vector<material::MaterialParameter> {}
    ).into<MaterialResource>();

    graph::MaterialVisitor visitor {};
    visitor.addRaisedMaterial(_STD move(mat0));
    visitor.addRaisedMaterial(_STD move(mat1));
    visitor.addRaisedMaterial(_STD move(mat2));

    graph->update(visitor);

    return graph;
}
