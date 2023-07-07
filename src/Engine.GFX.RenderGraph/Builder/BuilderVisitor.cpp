#include "BuilderVisitor.hpp"

#include "../Node/Node.hpp"
#include "../Node/AnchorNode.hpp"
#include "../Node/BarrierNode.hpp"
#include "../Node/ConvergeNode.hpp"
#include "../Node/DivergeNode.hpp"
#include "../Node/ProviderNode.hpp"
#include "../Node/SubpassNode.hpp"
#include "../Node/SelectorNode.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

[[nodiscard]] constexpr static bool acceptor_predicate(cref<Node>) noexcept {
    return true;
}

/**/

BuilderVisitor::BuilderVisitor(
    mref<predicate_type> predicate_,
    mref<smr<Node>> begin_,
    mref<smr<Node>> end_,
    BuilderPredicateMode predicateMode_
) noexcept :
    _predicateMode(_STD move(predicateMode_)),
    _predicate(_STD move(predicate_)),
    _begin(_STD move(begin_)),
    _end(_STD move(end_)),
    _current(),
    _successor() {}

BuilderVisitor::~BuilderVisitor() noexcept = default;

cref<smr<Node>> BuilderVisitor::getCurrentNode() const noexcept {
    return _current;
}

cref<Vector<smr<Node>>> BuilderVisitor::getSuccessorNodes() const noexcept {
    return _successor;
}

void BuilderVisitor::operator()(cref<Node> node_) {
    node_.traverse(*this);
}

void BuilderVisitor::operator()(cref<AnchorNode> node_) {

    if (not _predicate(node_)) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<AnchorNode>>(node_);
    auto next = current.getNext();

    if (_begin) {
        current.setNext(_STD move(_begin));
    }

    if (_end) {
        // Modify current visitor
        _begin = _STD move(next);
        _predicate = acceptor_predicate;

        auto tmp = _STD move(_end);
        tmp->accept(*this);
    }
}

void BuilderVisitor::operator()(cref<BarrierNode> node_) {

    if (not _predicate(node_)) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<BarrierNode>>(node_);
    auto next = current.getNext();

    if (_begin) {
        current.setNext(_STD move(_begin));
    }

    if (_end) {
        // Modify current visitor
        _begin = _STD move(next);
        _predicate = acceptor_predicate;

        auto tmp = _STD move(_end);
        tmp->accept(*this);
    }
}

void BuilderVisitor::operator()(cref<ConvergeNode> node_) {

    if (not _predicate(node_)) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<ConvergeNode>>(node_);
    auto next = current.getNext();

    if (_begin) {
        current.setNext(_STD move(_begin));
    }

    if (_end) {
        // Modify current visitor
        _begin = _STD move(next);
        _predicate = acceptor_predicate;

        auto tmp = _STD move(_end);
        tmp->accept(*this);
    }
}

void BuilderVisitor::operator()(cref<DivergeNode> node_) {

    if (not _predicate(node_)) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<DivergeNode>>(node_);
    auto next = current.getNext();

    throw NotImplementedException();

    if (_begin) {
        //current.setNext(_STD move(_begin));
    }

    if (_end) {
        // Modify current visitor
        //_begin = _STD move(next);
        //_predicate = acceptor_predicate;

        //auto tmp = _STD move(_end);
        //tmp->accept(*this);
    }
}

void BuilderVisitor::operator()(cref<SelectorNode> node_) {

    if (not _predicate(node_)) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<SelectorNode>>(node_);
    auto next = current.getNext();

    throw NotImplementedException();

    if (_begin) {
        //current.setNext(_STD move(_begin));
    }

    if (_end) {
        // Modify current visitor
        //_begin = _STD move(next);
        //_predicate = acceptor_predicate;

        //auto tmp = _STD move(_end);
        //tmp->accept(*this);
    }
}

void BuilderVisitor::operator()(cref<ProviderNode> node_) {

    if (not _predicate(node_)) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<ProviderNode>>(node_);
    auto next = current.getNext();

    if (_begin) {
        current.setNext(_STD move(_begin));
    }

    if (_end) {
        // Modify current visitor
        _begin = _STD move(next);
        _predicate = acceptor_predicate;

        auto tmp = _STD move(_end);
        tmp->accept(*this);
    }
}

void BuilderVisitor::operator()(cref<SubpassNode> node_) {

    if (not _predicate(node_)) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<SubpassNode>>(node_);
    auto next = current.getNext();

    if (_begin) {
        current.setNext(_STD move(_begin));
    }

    if (_end) {
        // Modify current visitor
        _begin = _STD move(next);
        _predicate = acceptor_predicate;

        auto tmp = _STD move(_end);
        tmp->accept(*this);
    }
}
