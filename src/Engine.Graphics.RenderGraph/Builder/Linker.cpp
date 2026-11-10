#include "Linker.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

#include "BLinkVisitor.hpp"
#include "FLinkVisitor.hpp"
#include "../Node/Node.hpp"

using namespace hg::engine::render::graph;
using namespace hg::engine::render;
using namespace hg;

void graph::LinkNodes(mref<nmpt<Node>> predecessor_, mref<smr<Node>> successor_, LinkMode mode_) {

	BLinkVisitor blv { clone(predecessor_), mode_ };
	successor_->accept(blv);

	FLinkVisitor flv { std::move(successor_), mode_ };
	predecessor_->accept(flv);
}
