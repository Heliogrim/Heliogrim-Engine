#include "Builder.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "BuilderVisitor.hpp"
#include "../CompileGraph.hpp"
#include "../RuntimeGraph.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

void Builder::insertNode(
	nmpt<const Node> from_,
	nmpt<const Node> to_,
	mref<smr<Node>> node_,
	nmpt<RuntimeGraph> graph_
) {
	return insertSubGraph(std::move(from_), std::move(to_), clone(node_), std::move(node_), std::move(graph_));
}

uptr<CompileGraph> Builder::insertNode(
	mref<uptr<CompileGraph>> graph_,
	nmpt<const Node> from_,
	nmpt<const Node> to_,
	mref<smr<Node>> node_
) {
	return insertSubGraph(std::move(graph_), std::move(from_), std::move(to_), clone(node_), std::move(node_));
}

void Builder::insertSubGraph(
	nmpt<const Node> from_,
	nmpt<const Node> to_,
	mref<smr<Node>> begin_,
	mref<smr<Node>> end_,
	nmpt<RuntimeGraph> graph_
) {

	BuilderVisitor visitor {
		[from_](cref<Node> node_) {
			return std::addressof(node_) == from_.get();
		},
		[to_](cref<Node> node_) {
			return std::addressof(node_) == to_.get();
		},
		std::move(begin_),
		std::move(end_)
	};

	graph_->update(visitor);
}

uptr<CompileGraph> Builder::insertSubGraph(
	mref<uptr<CompileGraph>> graph_,
	nmpt<const Node> from_,
	nmpt<const Node> to_,
	mref<smr<Node>> begin_,
	mref<smr<Node>> end_
) {

	BuilderVisitor visitor {
		[from_](cref<Node> node_) {
			return std::addressof(node_) == from_.get();
		},
		[to_](cref<Node> node_) {
			return std::addressof(node_) == to_.get();
		},
		std::move(begin_),
		std::move(end_)
	};

	graph_->update(visitor);
	return graph_;
}

uptr<RuntimeGraph> Builder::eraseNode(
	mref<uptr<RuntimeGraph>> graph_,
	nmpt<const Node> where_,
	mref<smr<Node>> node_
) {
	::hg::todo_panic();
	return graph_;
}

uptr<CompileGraph> Builder::eraseNode(
	mref<uptr<CompileGraph>> graph_,
	nmpt<const Node> where_,
	mref<smr<Node>> node_
) {
	::hg::todo_panic();
	return graph_;
}
