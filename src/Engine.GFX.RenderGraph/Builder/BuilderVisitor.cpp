#include "BuilderVisitor.hpp"

#include <Engine.Common/Exception/NotImplementedException.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Linker.hpp"
#include "../Node/CompileNode.hpp"
#include "../Node/Node.hpp"
#include "../Node/Compile/CompileSubPassNode.hpp"
#include "../Node/Runtime/AnchorNode.hpp"
#include "../Node/Runtime/BarrierNode.hpp"
#include "../Node/Runtime/ConvergeNode.hpp"
#include "../Node/Runtime/DivergeNode.hpp"
#include "../Node/Runtime/ProviderNode.hpp"
#include "../Node/Runtime/SelectorNode.hpp"
#include "../Node/Runtime/SubpassNode.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

template <typename TargetType_, typename SourceType_>
constexpr cref<TargetType_> override_cast(cref<SourceType_> val_) noexcept {
    return *reinterpret_cast<const TargetType_*>(reinterpret_cast<const void*>(&val_));
}

BuilderVisitor::BuilderVisitor(
    mref<predicate_type> from_,
    mref<predicate_type> to_,
    mref<smr<Node>> begin_,
    mref<smr<Node>> end_,
    BuilderPredicateMode predicateMode_
) noexcept :
    _predicateMode(_STD move(predicateMode_)),
    _from(_STD move(from_)),
    _to(_STD move(to_)),
    _fromNode(),
    _toNode(nullptr),
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

template <class NodeType_>
void BuilderVisitor::simple_splice_insert(cref<NodeType_> node_) {

    if (_fromNode == nullptr && _from(node_)) {
        _fromNode = _STD addressof(node_);
    }

    if (_toNode.empty() && _to(*node_.getNext())) {
        _toNode = clone(node_.getNext());
    }

    if (_fromNode == nullptr || _toNode.empty()) {
        return Visitor::operator()(node_);
    }

    if (_begin) {
        if (_fromNode.get() == _STD addressof(node_)) {

            /**
             * Backward Link :: Begin -> From
             * Forward Link :: From -> Begin
             *
             *      [ ... ]
             *         |
             *    From ~ Current
             *         $
             *       Begin
             *      [ ... ]
             *        End
             *         $
             *      To ~ ???
             *      [ ... ]
             */

            LinkNodes(const_cast<Node*>(_fromNode.get()), _STD move(_begin));

        } else {
            _fromNode->accept(*this);
        }
    }

    if (_end) {

        assert(_toNode.get() != _STD addressof(node_));

        // TODO: If we would like to guarantee no cuts
        //assert(_toNode.get() == node_.getNext().get());

        /**
         * Backward Link :: To -> End
         * Forward Link :: End -> To
         *
         *      [ ... ]
         *         |
         *    From ~ ???
         *         $
         *       Begin
         *      [ ... ]
         *        End
         *         $
         *      To ~ Current
         *      [ ... ]
         */

        LinkNodes(_end.get(), _STD move(_toNode));
        _end.reset();
    }
}

void BuilderVisitor::operator()(cref<AnchorNode> node_) {
    return simple_splice_insert(node_);
}

void BuilderVisitor::operator()(cref<BarrierNode> node_) {
    return simple_splice_insert(node_);
}

void BuilderVisitor::operator()(cref<ConvergeNode> node_) {
    return simple_splice_insert(node_);
}

void BuilderVisitor::operator()(cref<DivergeNode> node_) {

    if (_fromNode == nullptr && _from(node_)) {
        _fromNode = _STD addressof(node_);
    }

    if (_toNode.empty()) {
        for (const auto& child : node_.getNext()) {
            if (_to(*child)) {
                _toNode = clone(child);
                break;
            }
        }
    }

    if (_fromNode == nullptr || _toNode.empty()) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<DivergeNode>>(node_);

    if (_begin) {
        if (_fromNode.get() == _STD addressof(current)) {

            current.removeNext(_toNode);
            LinkNodes(const_cast<Node*>(_fromNode.get()), _STD move(_begin));

        } else {
            _fromNode->accept(*this);
        }
    }

    if (_end) {

        assert(_toNode.get() != _STD addressof(current));

        // TODO: If we would like to guarantee no cuts
        //assert(_toNode.get() == node_.getNext().get());

        LinkNodes(_end.get(), _STD move(_toNode));
        _end.reset();
    }
}

void BuilderVisitor::operator()(cref<SelectorNode> node_) {

    if (_fromNode == nullptr && _from(node_)) {
        _fromNode = _STD addressof(node_);
    }

    if (_toNode.empty()) {
        for (const auto& child : node_.getNext()) {
            if (_to(*child.node)) {
                _toNode = clone(child.node);
                break;
            }
        }
    }

    if (_fromNode == nullptr || _toNode.empty()) {
        return Visitor::operator()(node_);
    }

    auto& current = const_cast<ref<SelectorNode>>(node_);

    if (_begin) {
        if (_fromNode.get() == _STD addressof(current)) {

            // TODO: Check how we want to provide the active masking

            current.removeNext(_toNode);
            LinkNodes(const_cast<Node*>(_fromNode.get()), _STD move(_begin));

        } else {
            _fromNode->accept(*this);
        }
    }

    if (_end) {

        assert(_toNode.get() != _STD addressof(current));

        // TODO: If we would like to guarantee no cuts
        //assert(_toNode.get() == node_.getNext().get());

        LinkNodes(_end.get(), _STD move(_toNode));
        _end.reset();
    }
}

void BuilderVisitor::operator()(cref<ProviderNode> node_) {
    return simple_splice_insert(node_);
}

void BuilderVisitor::operator()(cref<SubPassNodeBase> node_) {
    return simple_splice_insert(node_);
}

void BuilderVisitor::operator()(cref<CompileNode> node_) {
    BuilderVisitor::operator()(override_cast<Node>(node_));
}

void BuilderVisitor::operator()(cref<CompileSubPassNode> node_) {
    return simple_splice_insert(node_);
}
