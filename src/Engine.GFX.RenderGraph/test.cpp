#include "test.hpp"

#include <Engine.GFX.Acc/Stage/TransferToken.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SimpleSpecificationStorage.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>
#include <Engine.GFX.Material/Material.hpp>

#include "CompileGraph.hpp"
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
using namespace hg::engine::gfx;
using namespace hg;

using AccToken = engine::gfx::acc::TransferToken;

void hg::test_render_graph() {

    uptr<CompileGraph> graph = make_uptr<CompileGraph>();
    const nmpt<Node> begin = graph->begin().get();
    const nmpt<Node> end = graph->end().get();

    nmpt<Node> cursor { begin };

    /**/

    // SceneViewDescription ~ Camera { position, rotation, fov, aspect, ... }
    // MeshDescription ~ { primitive types, topology, static / skeletal, procedural ?!?, ... }
    // TextureDescription ~ { type, format, layer, mip stack, ... }
    // ModelDescription ~ { position, rotation, scale, materials ?!?, ... }

    smr<Description> cameraDescription;
    smr<Description> depthBufferDescription;
    smr<Description> colorBufferDescription;

    {
        cameraDescription = make_smr<>(new SceneViewDescription()).into<Description>();

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

    // TODO: renderer or outer context should provide a list of target symbols to expose externally
    // TODO: in addition we could define a list of symbols carried/looped back by the instantiated render-pass
    // Renderer :: exposedSymbols() -> List < Symbol >
    // Renderer :: isolatedSymbols() -> List < Symbol >
    // e.g. List < Color Symbol, ... > + List < Depth Symbol, ... >

    Vector<smr<acc::Symbol>> exposedSymbols {};
    Vector<smr<acc::Symbol>> isolatedSymbols {};

    smr<acc::Symbol> sceneColorSymbol = make_smr<acc::Symbol>(
        acc::Symbol {
            .scope = acc::SymbolScope {
                .type = acc::SymbolScopeType::eGlobal,
                .layer = ""
            },
            .name = "scene::color",
            .description = colorBufferDescription,
            .flags = acc::SymbolFlagBits::eUndefined,
            .token = AccToken {}
        }
    );

    smr<acc::Symbol> sceneDepthSymbol = make_smr<acc::Symbol>(
        acc::Symbol {
            .scope = acc::SymbolScope {
                .type = acc::SymbolScopeType::eGlobal,
                .layer = ""
            },
            .name = "scene::depth",
            .description = depthBufferDescription,
            .flags = acc::SymbolFlagBits::eUndefined,
            .token = AccToken {}
        }
    );

    /**/

    {
        auto provider = make_smr<ProviderNode>();
        auto comp = provider->getProviderComponent();

        auto cameraSymbol = make_smr<acc::Symbol>(
            acc::Symbol {
                .scope = acc::SymbolScope {
                    .type = acc::SymbolScopeType::eGlobal,
                    .layer = ""
                },
                .name = "scene::camera",
                .description = cameraDescription,
                .flags = acc::SymbolFlagBits::eUndefined,
                .token = AccToken {}
            }
        );

        comp->setProvided(
            {
                Provision { cameraSymbol }
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

        cursor = subpassNode.get();
        graph = Builder::insertNode(_STD move(graph), it, end, _STD move(subpassNode));
    }

    /**/

    Resolver resolver {};
    ResolverOptions options {
        .flags = ResolverPassFlags {} | ResolverPassFlagBits::eBasicLayout | ResolverPassFlagBits::eValidate
    };

    // Warning: Invalid, base graph is no longer resolvable
    //graph = resolver(_STD move(graph), _STD move(options));

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
