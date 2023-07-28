#include "test.hpp"

#include <Engine.GFX.Acc/AccelerationStageTransferToken.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SimpleSpecificationStorage.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>
#include <Engine.GFX.Material/Material.hpp>

#include "RenderGraph.hpp"
#include "Builder/Builder.hpp"
#include "Component/BarrierComponent.hpp"
#include "Component/ProviderComponent.hpp"
#include "Component/SubpassComponent.hpp"
#include "Debug/DebugVisitor.hpp"

#include "Node/AnchorNode.hpp"
#include "Node/BarrierNode.hpp"
#include "Node/SubpassNode.hpp"
#include "Node/ProviderNode.hpp"
#include "Relation/MeshDescription.hpp"
#include "Relation/ModelDescription.hpp"
#include "Relation/SceneViewDescription.hpp"
#include "Relation/TextureDescription.hpp"
#include "Resolver/Resolver.hpp"
#include "Resolver/ResolverOptions.hpp"
#include "Visitor/MaterialVisitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::render;
using namespace hg;

using AccToken = engine::gfx::acc::AccelerationStageTransferToken;

void hg::test_render_graph() {

    uptr<RenderGraph> graph = make_uptr<RenderGraph>();
    const nmpt<Node> begin = graph->begin().get();
    const nmpt<Node> end = graph->end().get();

    nmpt<Node> cursor { begin };

    /**/

    // SceneViewDescription ~ Camera { position, rotation, fov, aspect, ... }
    // MeshDescription ~ { primitive types, topology, static / skeletal, procedural ?!?, ... }
    // TextureDescription ~ { type, format, layer, mip stack, ... }
    // ModelDescription ~ { position, rotation, scale, materials ?!?, ... }

    smr<Description> cameraDescription;
    smr<Description> meshDescription;
    smr<Description> modelDescription;
    smr<Description> depthBufferDescription;
    smr<Description> positionBufferDescription;
    smr<Description> normalBufferDescription;
    smr<Description> colorBufferDescription;

    {
        cameraDescription = make_smr<>(new SceneViewDescription()).into<Description>();
        meshDescription = make_smr<>(new MeshDescription()).into<Description>();
        modelDescription = make_smr<>(new ModelDescription()).into<Description>();

        depthBufferDescription = make_smr<TextureDescription>(
            new TextureDescription(
                TextureDescription {
                    { DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                    { DescriptionValueMatchingMode::eCovariant, engine::gfx::TextureFormat::eD32Sfloat },
                    { DescriptionValueMatchingMode::eInvariant, 1ui32 },
                    {}
                }
            )
        ).into<Description>();

        positionBufferDescription = make_smr<TextureDescription>(
            new TextureDescription(
                TextureDescription {
                    { DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                    { DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR32G32B32Sfloat },
                    { DescriptionValueMatchingMode::eInvariant, 1ui32 },
                    {}
                }
            )
        ).into<Description>();

        normalBufferDescription = make_smr<TextureDescription>(
            new TextureDescription(
                TextureDescription {
                    { DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                    { DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR32G32B32Sfloat },
                    { DescriptionValueMatchingMode::eInvariant, 1ui32 },
                    {}
                }
            )
        ).into<Description>();

        colorBufferDescription = make_smr<TextureDescription>(
            new TextureDescription(
                TextureDescription {
                    { DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureType::e2d },
                    { DescriptionValueMatchingMode::eInvariant, engine::gfx::TextureFormat::eR8G8B8A8Unorm },
                    { DescriptionValueMatchingMode::eCovariant, 1ui32 },
                    {}
                }
            )
        ).into<Description>();
    }

    /**/

    {
        auto provider = make_smr<ProviderNode>();
        auto comp = provider->getProviderComponent();

        comp->setProvided(
            {
                // TODO: Add descriptions to required and provisioned data
                Provision { AccToken::from("camera").hash, cameraDescription },
                Provision { AccToken::from("mesh").hash, meshDescription },
                Provision { AccToken::from("model").hash, modelDescription }
            }
        );

        cursor = provider.get();
        graph = Builder::insertNode(_STD move(graph), begin, end, _STD move(provider));
    }

    {
        auto barrier = make_smr<BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Depth Barrier");

        auto it = barrier.get();
        graph = Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<SubpassNode>(SubpassAccelMode::eSingle);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                Requirement { AccToken::from("camera").hash, cameraDescription },
                Requirement { AccToken::from("mesh").hash, meshDescription },
                Requirement { AccToken::from("model").hash, modelDescription }
            }
        );

        subpassComp->setProvided(
            {
                Provision { AccToken::from("depth").hash, depthBufferDescription },
                //Provision { AccToken::from("stencil").hash }
            }
        );

        cursor = subpassNode.get();
        graph = Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    {
        auto barrier = make_smr<BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Material Barrier");

        auto it = barrier.get();
        graph = Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<SubpassNode>(SubpassAccelMode::eMaterial);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                Requirement { AccToken::from("camera").hash },
                Requirement { AccToken::from("mesh").hash },
                Requirement { AccToken::from("model").hash },
                Requirement { AccToken::from("depth").hash, depthBufferDescription },
            }
        );

        subpassComp->setProvided(
            {
                Provision { AccToken::from("depth").hash, depthBufferDescription },
                Provision { AccToken::from("position").hash, positionBufferDescription },
                Provision { AccToken::from("normal").hash, normalBufferDescription }
            }
        );

        cursor = subpassNode.get();
        graph = Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    {
        auto barrier = make_smr<BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Lighting Barrier");

        auto it = barrier.get();
        graph = Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<SubpassNode>(SubpassAccelMode::eMulti);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                Requirement { AccToken::from("camera").hash, cameraDescription },
                Requirement { AccToken::from("depth").hash, depthBufferDescription },
                Requirement { AccToken::from("position").hash, positionBufferDescription },
                Requirement { AccToken::from("normal").hash, normalBufferDescription }
            }
        );

        subpassComp->setProvided(
            {
                Provision { AccToken::from("color").hash, colorBufferDescription }
            }
        );

        cursor = subpassNode.get();
        graph = Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    {
        auto barrier = make_smr<BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Compositing Barrier");

        auto it = barrier.get();
        graph = Builder::insertNode(_STD move(graph), cursor, end, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<SubpassNode>(SubpassAccelMode::eMulti);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                Requirement { AccToken::from("color").hash, colorBufferDescription }
            }
        );

        subpassComp->setProvided(
            {
                Provision { AccToken::from("color").hash, colorBufferDescription }
            }
        );

        cursor = subpassNode.get();
        graph = Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    /**/

    Resolver resolver {};
    ResolverOptions options {
        .flags = ResolverPassFlags {} | ResolverPassFlagBits::eBasicLayout | ResolverPassFlagBits::eValidate
    };

    graph = resolver(_STD move(graph), _STD move(options));

    /**/

    {
        DebugVisitor visitor {};
        graph->begin()->accept(visitor);
    }

    /**/

    smr<engine::gfx::MaterialResource> mat0 {}, mat1 {}, mat2 {};
    MaterialVisitor visitor {};
    visitor.addRaisedMaterial(_STD move(mat0));
    visitor.addRaisedMaterial(_STD move(mat1));
    visitor.addRaisedMaterial(_STD move(mat2));

    graph->update(visitor);
}
