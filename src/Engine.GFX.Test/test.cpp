#include "pch.h"

#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.Render/Builder/BaseBuilder.hpp>
#include <Engine.GFX.Render/Builder/BuilderLayerVisitor.hpp>
#include <Engine.GFX.Render/Builder/StreamBuilder.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>
#include <Engine.GFX.RenderPipeline/__fwd.hpp>
#include <Engine.GFX.RenderPipeline/RenderPipeline.hpp>
#include <Engine.GFX.RenderPipeline/State/State.hpp>

TEST(__DummyTest__, Exists) {
    EXPECT_TRUE(true);
}

using namespace hg::engine::gfx::render;
using namespace hg::engine::gfx;
using namespace hg;

auto make_test_graph() -> uptr<RenderGraph>;

TEST(___Debug___, Check) {

    uptr<RenderGraph> graph = make_test_graph();

    /**/

    const BaseBuilder builder {};
    auto pipe = builder(graph.get());

    pipe->operator()(nullptr);

}

#include <Engine.GFX.Acc/AccelerationStageTransferToken.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SimpleSpecificationStorage.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>
#include <Engine.GFX.Material/Material.hpp>
#include <Engine.GFX.RenderGraph/Debug/DebugVisitor.hpp>
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderGraph/Relation/Description.hpp>
#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/ModelDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/Provision.hpp>
#include <Engine.GFX.RenderGraph/Relation/Requirement.hpp>
#include <Engine.GFX.RenderGraph/Relation/SceneViewDescription.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Resolver/ResolverOptions.hpp>
#include <Engine.GFX.RenderGraph/Visitor/MaterialVisitor.hpp>
#include <Engine.GFX.RenderGraph/RenderGraph.hpp>
#include <Engine.GFX.RenderGraph/Component/SubpassComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/BarrierComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/AnchorComponent.hpp>
#include <Engine.GFX.RenderGraph/Component/ProviderComponent.hpp>
#include <Engine.GFX.RenderGraph/Node/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/AnchorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Builder/Builder.hpp>
#include <Engine.GFX.RenderGraph/Builder/BuilderVisitor.hpp>
#include <Engine.GFX.RenderGraph/Resolver/Resolver.hpp>

using AccToken = engine::gfx::acc::AccelerationStageTransferToken;

auto make_test_graph() -> uptr<RenderGraph> {

    uptr<RenderGraph> graph = make_uptr<RenderGraph>();
    const nmpt<graph::Node> begin = graph->begin().get();
    const nmpt<graph::Node> end = graph->end().get();

    nmpt<graph::Node> cursor { begin };

    /**/

    // SceneViewDescription ~ Camera { position, rotation, fov, aspect, ... }
    // MeshDescription ~ { primitive types, topology, static / skeletal, procedural ?!?, ... }
    // TextureDescription ~ { type, format, layer, mip stack, ... }
    // ModelDescription ~ { position, rotation, scale, materials ?!?, ... }

    smr<graph::Description> cameraDescription;
    smr<graph::Description> meshDescription;
    smr<graph::Description> modelDescription;
    smr<graph::Description> depthBufferDescription;
    smr<graph::Description> positionBufferDescription;
    smr<graph::Description> normalBufferDescription;
    smr<graph::Description> colorBufferDescription;

    {
        cameraDescription = make_smr<>(new graph::SceneViewDescription()).into<graph::Description>();
        meshDescription = make_smr<>(new graph::MeshDescription()).into<graph::Description>();
        modelDescription = make_smr<>(new graph::ModelDescription()).into<graph::Description>();

        depthBufferDescription = make_smr<graph::TextureDescription>(
            new graph::TextureDescription(
                graph::TextureDescription {
                    { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                    { graph::DescriptionValueMatchingMode::eCovariant, engine::gfx::TextureFormat::eD32Sfloat },
                    { graph::DescriptionValueMatchingMode::eInvariant, 1ui32 },
                    {}
                }
            )
        ).into<graph::Description>();

        positionBufferDescription = make_smr<graph::TextureDescription>(
            new graph::TextureDescription(
                graph::TextureDescription {
                    { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                    { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR32G32B32Sfloat },
                    { graph::DescriptionValueMatchingMode::eInvariant, 1ui32 },
                    {}
                }
            )
        ).into<graph::Description>();

        normalBufferDescription = make_smr<graph::TextureDescription>(
            new graph::TextureDescription(
                graph::TextureDescription {
                    { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                    { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR32G32B32Sfloat },
                    { graph::DescriptionValueMatchingMode::eInvariant, 1ui32 },
                    {}
                }
            )
        ).into<graph::Description>();

        colorBufferDescription = make_smr<graph::TextureDescription>(
            new graph::TextureDescription(
                graph::TextureDescription {
                    { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                    { graph::DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR8G8B8A8Unorm },
                    { graph::DescriptionValueMatchingMode::eCovariant, 1ui32 },
                    {}
                }
            )
        ).into<graph::Description>();
    }

    /**/

    {
        auto provider = make_smr<graph::ProviderNode>();
        auto comp = provider->getProviderComponent();

        comp->setProvided(
            {
                // TODO: Add descriptions to required and provisioned data
                graph::Provision { AccToken::from("camera").hash, cameraDescription },
                graph::Provision { AccToken::from("mesh").hash, meshDescription },
                graph::Provision { AccToken::from("model").hash, modelDescription }
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

        auto subpassNode = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eSingle);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                graph::Requirement { AccToken::from("camera").hash, cameraDescription },
                graph::Requirement { AccToken::from("mesh").hash, meshDescription },
                graph::Requirement { AccToken::from("model").hash, modelDescription }
            }
        );

        subpassComp->setProvided(
            {
                graph::Provision { AccToken::from("depth").hash, depthBufferDescription },
                //Provision { AccToken::from("stencil").hash }
            }
        );

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

        auto subpassNode1 = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMaterial);
        auto subpassComp1 = subpassNode1->getSubpassComponent();

        subpassComp1->setRequirements(
            {
                graph::Requirement { AccToken::from("camera").hash },
                graph::Requirement { AccToken::from("mesh").hash },
                graph::Requirement { AccToken::from("model").hash },
                graph::Requirement { AccToken::from("depth").hash, depthBufferDescription },
            }
        );

        subpassComp1->setProvided(
            {
                graph::Provision { AccToken::from("depth").hash, depthBufferDescription },
                graph::Provision { AccToken::from("position").hash, positionBufferDescription },
                graph::Provision { AccToken::from("normal").hash, normalBufferDescription }
            }
        );

        /**/

        auto subpassNode2 = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMaterial);
        auto subpassComp2 = subpassNode2->getSubpassComponent();

        subpassComp2->setRequirements(
            {
                graph::Requirement { AccToken::from("camera").hash },
                graph::Requirement { AccToken::from("mesh").hash },
                graph::Requirement { AccToken::from("model").hash },
                graph::Requirement { AccToken::from("depth").hash, depthBufferDescription },
            }
        );

        subpassComp2->setProvided(
            {
                graph::Provision { AccToken::from("depth").hash, depthBufferDescription },
                graph::Provision { AccToken::from("position").hash, positionBufferDescription },
                graph::Provision { AccToken::from("normal").hash, normalBufferDescription }
            }
        );

        /**/

        auto subpassNode3 = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMaterial);
        auto subpassComp3 = subpassNode3->getSubpassComponent();

        subpassComp3->setRequirements(
            {
                graph::Requirement { AccToken::from("camera").hash },
                graph::Requirement { AccToken::from("mesh").hash },
                graph::Requirement { AccToken::from("model").hash },
                graph::Requirement { AccToken::from("depth").hash, depthBufferDescription },
            }
        );

        subpassComp3->setProvided(
            {
                graph::Provision { AccToken::from("depth").hash, depthBufferDescription },
                graph::Provision { AccToken::from("position").hash, positionBufferDescription },
                graph::Provision { AccToken::from("normal").hash, normalBufferDescription }
            }
        );

        /**/

        auto sn4ref = subpassNode3.get();
        auto subpassNode4 = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMaterial);
        auto subpassComp4 = subpassNode4->getSubpassComponent();

        subpassComp4->setRequirements(
            {
                graph::Requirement { AccToken::from("camera").hash },
                graph::Requirement { AccToken::from("mesh").hash },
                graph::Requirement { AccToken::from("model").hash },
                graph::Requirement { AccToken::from("depth").hash, depthBufferDescription },
            }
        );

        subpassComp4->setProvided(
            {
                graph::Provision { AccToken::from("depth").hash, depthBufferDescription },
                graph::Provision { AccToken::from("position").hash, positionBufferDescription },
                graph::Provision { AccToken::from("normal").hash, normalBufferDescription }
            }
        );

        /**/

        /**/

        auto sn5ref = subpassNode2.get();
        auto subpassNode5 = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMaterial);
        auto subpassComp5 = subpassNode2->getSubpassComponent();

        subpassComp4->setRequirements(
            {
                graph::Requirement { AccToken::from("camera").hash },
                graph::Requirement { AccToken::from("mesh").hash },
                graph::Requirement { AccToken::from("model").hash },
                graph::Requirement { AccToken::from("depth").hash, depthBufferDescription },
            }
        );

        subpassComp4->setProvided(
            {
                graph::Provision { AccToken::from("depth").hash, depthBufferDescription },
                graph::Provision { AccToken::from("position").hash, positionBufferDescription },
                graph::Provision { AccToken::from("normal").hash, normalBufferDescription }
            }
        );

        /**/

        auto diverge = make_smr<graph::DivergeNode>();

        auto dit = diverge.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(diverge));

        /**/

        auto converge = make_smr<graph::ConvergeNode>();

        cursor = converge.get();
        graph = graph::Builder::insertNode(_STD move(graph), dit, end, _STD move(converge));

        /**/

        graph = graph::Builder::insertNode(_STD move(graph), dit, cursor, _STD move(subpassNode1));
        graph = graph::Builder::insertNode(_STD move(graph), dit, cursor, _STD move(subpassNode2));
        graph = graph::Builder::insertNode(_STD move(graph), dit, cursor, _STD move(subpassNode3));
        graph = graph::Builder::insertNode(_STD move(graph), sn4ref, cursor, _STD move(subpassNode4));
        graph = graph::Builder::insertNode(_STD move(graph), sn5ref, cursor, _STD move(subpassNode5));
    }

    {
        auto barrier = make_smr<graph::BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Lighting Barrier");

        auto it = barrier.get();
        graph = graph::Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMulti);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                graph::Requirement { AccToken::from("camera").hash, cameraDescription },
                graph::Requirement { AccToken::from("depth").hash, depthBufferDescription },
                graph::Requirement { AccToken::from("position").hash, positionBufferDescription },
                graph::Requirement { AccToken::from("normal").hash, normalBufferDescription }
            }
        );

        subpassComp->setProvided(
            {
                graph::Provision { AccToken::from("color").hash, colorBufferDescription }
            }
        );

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

        auto subpassNode = make_smr<graph::SubpassNode>(graph::SubpassAccelMode::eMulti);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                graph::Requirement { AccToken::from("color").hash, colorBufferDescription }
            }
        );

        subpassComp->setProvided(
            {
                graph::Provision { AccToken::from("color").hash, colorBufferDescription }
            }
        );

        cursor = subpassNode.get();
        graph = graph::Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    /**/

    graph::Resolver resolver {};
    graph::ResolverOptions options {
        .flags = graph::ResolverPassFlags {} | graph::ResolverPassFlagBits::eBasicLayout |
        graph::ResolverPassFlagBits::eValidate
    };

    graph = resolver(_STD move(graph), _STD move(options));

    /**/

    {
        graph::DebugVisitor visitor {};
        graph->begin()->accept(visitor);
    }

    /**/

    smr<engine::gfx::MaterialResource> mat0 {}, mat1 {}, mat2 {};
    graph::MaterialVisitor visitor {};
    visitor.addRaisedMaterial(_STD move(mat0));
    visitor.addRaisedMaterial(_STD move(mat1));
    visitor.addRaisedMaterial(_STD move(mat2));

    graph->update(visitor);

    return graph;
}
