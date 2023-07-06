#include "test.hpp"

#include <Engine.GFX.Acc/AccelerationPass.hpp>
#include <Engine.GFX.Acc/AccelerationStageTransferToken.hpp>
#include <Engine.GFX.Acc.Compile/Spec/SpecificationStorage.hpp>
#include <Engine.GFX.Material/Material.hpp>

#include "RenderGraph.hpp"
#include "__fwd.hpp"
#include "Component/BarrierComponent.hpp"
#include "Component/SubpassAccelComponent.hpp"
#include "Component/SubpassComponent.hpp"

#include "Node/AnchorNode.hpp"
#include "Node/BarrierNode.hpp"
#include "Node/SubpassNode.hpp"
#include "Resolver/Resolver.hpp"
#include "Resolver/ResolverOptions.hpp"
#include "Visitor/MaterialVisitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx::render;
using namespace hg;

using AccToken = engine::gfx::acc::AccelerationStageTransferToken;

extern uptr<engine::gfx::acc::SpecificationStorage> generateSpecStore();

void hg::test_render_graph() {

    uptr<RenderGraph> graph {};
    const auto begin = graph->begin();
    const auto end = graph->end();

    /**/

    {
        auto barrier = make_smr<BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Depth Barrier");

        auto it = barrier.get();
        graph->pushNode(begin, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<SubpassNode>(SubpassAccelMode::eSingle);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                Requirement { AccToken::from("camera").hash },
                Requirement { AccToken::from("mesh").hash },
                Requirement { AccToken::from("model").hash }
            }
        );

        subpassComp->setProvided(
            {
                Provision { AccToken::from("depth").hash },
                //Provision { AccToken::from("stencil").hash }
            }
        );

        auto subpassAccel = subpassNode->getSubpassAcceleration();
        subpassAccel->pushSpecification(generateSpecStore());

        graph->pushNode(it, _STD move(subpassNode));
    }

    {
        auto barrier = make_smr<BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Material Barrier");

        auto it = barrier.get();
        graph->pushNode(begin, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<SubpassNode>(SubpassAccelMode::eMulti);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                Requirement { AccToken::from("camera").hash },
                Requirement { AccToken::from("mesh").hash },
                Requirement { AccToken::from("model").hash },
                Requirement { AccToken::from("depth").hash },
            }
        );

        subpassComp->setProvided(
            {
                Provision { AccToken::from("depth").hash },
                Provision { AccToken::from("position").hash },
                Provision { AccToken::from("normal").hash }
            }
        );

        auto subpassAccel = subpassNode->getSubpassAcceleration();
        subpassAccel->pushSpecification(generateSpecStore());

        graph->pushNode(it, _STD move(subpassNode));
    }

    {
        auto barrier = make_smr<BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Lighting Barrier");

        auto it = barrier.get();
        graph->pushNode(begin, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<SubpassNode>(SubpassAccelMode::eMulti);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                Requirement { AccToken::from("depth").hash },
                Requirement { AccToken::from("position").hash },
                Requirement { AccToken::from("normal").hash }
            }
        );

        subpassComp->setProvided(
            {
                Provision { AccToken::from("color").hash }
            }
        );

        auto subpassAccel = subpassNode->getSubpassAcceleration();
        subpassAccel->pushSpecification(generateSpecStore());

        graph->pushNode(it, _STD move(subpassNode));
    }

    {
        auto barrier = make_smr<BarrierNode>();
        auto comp = barrier->getBarrierComponent();
        comp->setBarrierName("Compositing Barrier");

        auto it = barrier.get();
        graph->pushNode(begin, _STD move(barrier));

        /**/

        auto subpassNode = make_smr<SubpassNode>(SubpassAccelMode::eMulti);
        auto subpassComp = subpassNode->getSubpassComponent();

        subpassComp->setRequirements(
            {
                Requirement { AccToken::from("color").hash }
            }
        );

        subpassComp->setProvided(
            {
                Provision { AccToken::from("color").hash }
            }
        );

        auto subpassAccel = subpassNode->getSubpassAcceleration();
        subpassAccel->pushSpecification(generateSpecStore());

        graph->pushNode(it, _STD move(subpassNode));
    }

    /**/

    Resolver resolver {};
    ResolverOptions options {
        .flags = ResolverPassFlagBits::eBasicLayout
    };

    graph = resolver(_STD move(graph), _STD move(options));

    /**/

    smr<engine::gfx::material::Material> mat0 {}, mat1 {}, mat2 {};
    MaterialVisitor visitor {};
    visitor.addRaisedMaterial(_STD move(mat0));
    visitor.addRaisedMaterial(_STD move(mat1));
    visitor.addRaisedMaterial(_STD move(mat2));

    graph->update(visitor);
}
