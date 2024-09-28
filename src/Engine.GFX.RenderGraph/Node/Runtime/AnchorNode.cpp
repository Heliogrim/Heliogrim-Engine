#include "AnchorNode.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Common/Make.hpp>

#include "../../Component/AnchorComponent.hpp"
#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

AnchorNode::AnchorNode() noexcept :
	RuntimeNode(make_smr<Auxiliary>()) {
	_auxiliary->add(uptr<AnchorComponent>(new AnchorComponent()));
}

AnchorNode::~AnchorNode() noexcept = default;

void AnchorNode::accept(ref<Visitor> visitor_) const {
	visitor_(*this);
}

void AnchorNode::traverse(ref<Visitor> visitor_) const {
	if (!hasNext()) {
		return;
	}

	_next->accept(visitor_);
}

void AnchorNode::rtraverse(ref<Visitor> visitor_) const {
	if (!hasPrev()) {
		return;
	}

	_prev->accept(visitor_);
}

bool AnchorNode::hasNext() const noexcept {
	return not _next.empty();
}

void AnchorNode::setNext(mref<smr<Node>> next_) {
	_next = std::move(next_);
}

smr<Node> AnchorNode::getNext() const noexcept {
	return _next;
}

bool AnchorNode::hasPrev() const noexcept {
	return static_cast<bool>(_prev);
}

void AnchorNode::setPrev(mref<nmpt<const Node>> prev_) {
	_prev = std::move(prev_);
}

nmpt<const Node> AnchorNode::getPrev() const noexcept {
	return _prev;
}

nmpt<const AnchorComponent> AnchorNode::getAnchorComponent() const noexcept {
	return const_cast<ptr<this_type>>(this)->getAnchorComponent();
}

nmpt<AnchorComponent> AnchorNode::getAnchorComponent() {
	const auto it = std::ranges::find_if(
		_auxiliary->components(),
		[](const ptr<Component> component_) {
			return component_->getMetaClass()->exact<AnchorComponent>();
		}
	);
	return it != _auxiliary->components().end() ?
		static_cast<ptr<AnchorComponent>>(*it) :
		nmpt<AnchorComponent> {};
}

IterationResult AnchorNode::iterate(cref<IterationPassContext> ctx_) noexcept {
	return IterationResultBits::eNone;
}

IterationResult AnchorNode::resolve(cref<ResolvePassContext> ctx_) noexcept {
	return IterationResultBits::eNone;
}

IterationResult AnchorNode::execute(cref<ExecutionPassContext> ctx_) noexcept {
	return IterationResultBits::eNone;
}
