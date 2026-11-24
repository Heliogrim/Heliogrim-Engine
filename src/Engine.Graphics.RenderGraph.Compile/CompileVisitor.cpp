#include "CompileVisitor.hpp"

#include <cassert>

/**/

import Engine.Accel.Compile;
import Engine.Common;
import Engine.Graphics.RenderGraph;
import Engine.Reflect;
/**/

using namespace hg::engine::render::graph;
using namespace hg;

/**/

CompileVisitor::CompileVisitor(mref<nmpt<RuntimeGraph>> target_) noexcept :
	Visitor(),
	_target(std::move(target_)) {}

void CompileVisitor::operator()(cref<Node> node_) {
	return node_.rtraverse(*this);
}

void CompileVisitor::operator()(cref<AnchorNode> node_) {

	auto allocated = make_smr<AnchorNode>();
	Builder::insertNode(
		std::as_const(*_target).begin(),
		_target->begin()->getNext().get(),
		std::move(allocated),
		_target
	);

	Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<BarrierNode> node_) {

	auto allocated = make_smr<BarrierNode>();
	Builder::insertNode(
		std::as_const(*_target).begin(),
		_target->begin()->getNext().get(),
		std::move(allocated),
		_target
	);

	Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<ConvergeNode> node_) {

	auto allocated = make_smr<ConvergeNode>();
	Builder::insertNode(
		std::as_const(*_target).begin(),
		_target->begin()->getNext().get(),
		std::move(allocated),
		_target
	);

	Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<DivergeNode> node_) {

	auto allocated = make_smr<DivergeNode>();
	Builder::insertNode(
		std::as_const(*_target).begin(),
		_target->begin()->getNext().get(),
		std::move(allocated),
		_target
	);

	Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<SelectorNode> node_) {

	auto allocated = make_smr<SelectorNode>();
	Builder::insertNode(
		std::as_const(*_target).begin(),
		_target->begin()->getNext().get(),
		std::move(allocated),
		_target
	);

	Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<ProviderNode> node_) {

	auto allocated = make_smr<ProviderNode>();
	Builder::insertNode(
		std::as_const(*_target).begin(),
		_target->begin()->getNext().get(),
		std::move(allocated),
		_target
	);

	Visitor::operator()(node_);
}

void CompileVisitor::operator()(cref<SubPassNodeBase> node_) {
	assert(false);
}

void CompileVisitor::operator()(cref<CompileNode> node_) {

	const CompilePassContext context {};
	auto compiled = node_.compile(context);

	if (compiled.operator bool()) {
		Builder::insertNode(
			std::as_const(*_target).begin(),
			_target->begin()->getNext().get(),
			make_smr(std::move(compiled)),
			_target
		);
	}

	this->operator()(static_cast<cref<Node>>(node_));
}
