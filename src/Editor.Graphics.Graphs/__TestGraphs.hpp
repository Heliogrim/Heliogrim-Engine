#pragma once
#include "Engine.Common/Make.hpp"
#include "Graph/NodeGraph.hpp"
#include "Node/__fwd.hpp"
#include "Node/NodeLink.hpp"
#include "Node/NodeOutput.hpp"
#include "Node/NodeInput.hpp"
#include "Node/Node.hpp"
#include "Node/Input/BufferInputNode.hpp"
#include "Node/Input/ConstantInputNode.hpp"
#include "Node/Input/SubGraphInputNode.hpp"
#include "Node/Math/ScalarPowMathNode.hpp"
#include "Node/Math/ScalarSubMathNode.hpp"
#include "Node/Math/VectorAddMathNode.hpp"
#include "Node/Math/VectorAddScalarMathNode.hpp"
#include "Node/Math/VectorScaleMathNode.hpp"
#include "Node/Output/SubGraphOutputNode.hpp"
#include "Graph/NodeGraph.hpp"

using namespace ::hg::editor::gfx::graph::nodes;
using namespace ::hg::editor::gfx::graph;

namespace hg::test {
    sptr<NodeGraph> fresnelSchlick() {

        auto graph = make_sptr<NodeGraph>();

        /* 1.0 - cosTheta */

        auto* const cosTheta = graph->nodes().makeNode<SubGraphInputNode>();
        auto* const const1f = graph->nodes().makeNode<ConstantInputNode>();
        auto* const sub0 = graph->nodes().makeNode<ScalarSubMathNode>();

        auto* const l0 = graph->links().makeNodeLink();
        l0->setLinkInput(const1f->getOutputSlot(0));
        l0->addLinkOutput(sub0->getInputSlot("minuend"sv));

        auto* const l1 = graph->links().makeNodeLink();
        l1->setLinkInput(cosTheta->getOutputSlot(0));
        l1->addLinkOutput(sub0->getInputSlot("subtrahend"sv));

        /* pow(..., 5.0) */

        auto* const const5f = graph->nodes().makeNode<ConstantInputNode>();
        auto* const pow = graph->nodes().makeNode<ScalarPowMathNode>();

        auto* const l2 = graph->links().makeNodeLink();
        l2->setLinkInput(sub0->getOutputSlot("difference"sv));
        l2->addLinkOutput(pow->getInputSlot("base"sv));

        auto* const l3 = graph->links().makeNodeLink();
        l3->setLinkInput(const5f->getOutputSlot(0));
        l3->addLinkOutput(pow->getInputSlot("exponent"sv));

        /* 1.0 - ... */

        auto* const sub1 = graph->nodes().makeNode<ScalarSubMathNode>();

        auto* const l4 = graph->links().makeNodeLink();
        l4->setLinkInput(const1f->getOutputSlot(0));
        l4->addLinkOutput(sub1->getInputSlot("minuend"sv));

        auto* const l5 = graph->links().makeNodeLink();
        l5->setLinkInput(pow->getOutputSlot(0));
        l5->addLinkOutput(sub1->getInputSlot("subtrahend"sv));

        /* f0 * ... */

        auto* const f0 = graph->nodes().makeNode<SubGraphInputNode>();
        auto* const vecScale = graph->nodes().makeNode<VectorScaleMathNode>();

        auto* const l6 = graph->links().makeNodeLink();
        l6->setLinkInput(f0->getOutputSlot(0));
        l6->addLinkOutput(vecScale->getInputSlot("vector"sv));

        auto* const l7 = graph->links().makeNodeLink();
        l7->setLinkInput(sub1->getOutputSlot("difference"sv));
        l7->addLinkOutput(vecScale->getInputSlot("scalar"sv));

        /* ... + ... */

        auto* const vecAdd = graph->nodes().makeNode<VectorAddScalarMathNode>();

        auto* const l8 = graph->links().makeNodeLink();
        l8->setLinkInput(pow->getOutputSlot(0));
        l8->addLinkOutput(vecAdd->getInputSlot("scalar"sv));

        auto* const l9 = graph->links().makeNodeLink();
        l9->setLinkInput(vecScale->getOutputSlot(0));
        l9->addLinkOutput(vecAdd->getInputSlot("vector"sv));

        /**/

        auto* const out = graph->nodes().makeNode<SubGraphOutputNode>();

        auto* const l10 = graph->links().makeNodeLink();
        l10->setLinkInput(vecAdd->getOutputSlot(0));
        l10->addLinkOutput(out->getInputSlot(0));

        /**/

        graph->exposeNodes();
        return graph;
    }
}
