#include "pch.h"

#include <Engine.Common/Make.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>
#include <Engine.GFX.RenderGraph/Component/AnchorComponent.hpp>
#include <Engine.GFX.RenderGraph/Node/CompileNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/AnchorNode.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>

using namespace hg;
using namespace hg::engine::render::graph;

namespace RenderGraph {
	namespace {
		class CountingVisitor final :
			public Visitor {
		public:
			int anchors = 0;
			int others = 0;

			void operator()(cref<Node> node_) override { ++others; }

			void operator()(cref<CompileNode> node_) override { ++others; }

			void operator()(cref<AnchorNode> node_) override {
				++anchors;
				node_.traverse(*this);
			}
		};

		[[nodiscard]] smr<const Symbol> makeSymbol(const string_view name_) {
			auto symbol = make_smr<Symbol>();
			symbol->name = string { name_ };
			return symbol;
		}
	}

	TEST(RuntimeGraphTest, DefaultGraphLinksBeginToEnd) {
		RuntimeGraph graph {};

		ASSERT_NE(graph.begin(), nullptr);
		ASSERT_NE(graph.end(), nullptr);
		EXPECT_NE(graph.begin().get(), graph.end().get());
		EXPECT_TRUE(graph.begin()->hasNext());
		EXPECT_EQ(graph.begin()->getNext().get(), static_cast<const Node*>(graph.end().get()));
		EXPECT_FALSE(graph.end()->hasNext());
	}

	TEST(RuntimeGraphTest, UpdateVisitsBothAnchors) {
		RuntimeGraph graph {};
		CountingVisitor visitor {};

		graph.update(visitor);

		EXPECT_EQ(visitor.anchors, 2);
		EXPECT_EQ(visitor.others, 0);
	}

	TEST(RuntimeGraphTest, AggregationOfEmptyGraphIsEmpty) {
		const RuntimeGraph graph {};

		EXPECT_TRUE(graph.aggregateProvisions().empty());
		EXPECT_TRUE(graph.aggregateRequirements().empty());
	}

	TEST(CompileGraphTest, DefaultGraphLinksBeginToEnd) {
		CompileGraph graph {};

		EXPECT_TRUE(graph.begin()->hasNext());
		EXPECT_EQ(graph.begin()->getNext().get(), static_cast<const Node*>(graph.end().get()));
	}

	TEST(CompileGraphTest, UpdateStartsAtBeginReverseAtEnd) {
		CompileGraph graph {};
		CountingVisitor forward {}, backward {};

		graph.update(forward);
		graph.reverse(backward);

		EXPECT_EQ(forward.anchors, 2);
		// End anchor has no previous link configured by the minimal graph.
		EXPECT_EQ(backward.anchors, 1);
	}

	TEST(CompileGraphTest, ExpectedProvisionIsDeduplicatedBySymbol) {
		CompileGraph graph {};
		const auto symbol = makeSymbol("color");

		EXPECT_TRUE(graph.addExpectedProvision(Provision { symbol }));
		EXPECT_FALSE(graph.addExpectedProvision(Provision { symbol }));
		EXPECT_TRUE(graph.addExpectedProvision(Provision { makeSymbol("color") }));

		EXPECT_EQ(graph.expectedProvision().size(), 2uLL);
	}

	TEST(CompileGraphTest, ExpectedRequirementIsDeduplicatedBySymbol) {
		CompileGraph graph {};
		const auto symbol = makeSymbol("depth");

		EXPECT_TRUE(graph.addExpectedRequirement(Requirement { symbol }));
		EXPECT_FALSE(graph.addExpectedRequirement(Requirement { symbol }));
	}

	TEST(AnchorNodeTest, OwnsAnchorComponent) {
		AnchorNode node {};

		EXPECT_NE(node.getAnchorComponent(), nullptr);
		EXPECT_FALSE(node.hasNext());
		EXPECT_FALSE(node.hasPrev());
	}
}
