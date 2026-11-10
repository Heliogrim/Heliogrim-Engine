#include "Node.hpp"

#include "NodeInput.hpp"
#include "NodeOutput.hpp"
#include "NodeParameter.hpp"

using namespace hg::editor::gfx::graph;
using namespace hg;

cref<string> Node::getName() const noexcept {
    return _name;
}

void Node::setName(string_view name_) {
    _name = name_;
}

bool Node::carriesInternalDependencies() const noexcept {
    return false;
}

bool Node::carriesGraphDependencies() const noexcept {
    return false;
}

bool Node::carriesExternalDependencies() const noexcept {
    return false;
}

bool Node::isAssociative() const noexcept {
    return false;
}

bool Node::isCommutative() const noexcept {
    return false;
}

u64 Node::countInputSlots() const noexcept {
    return _inputs.size();
}

sptr<NodeInput> Node::getInputSlot(const u8 inputIndex_) const noexcept {
    if (_inputs.size() > inputIndex_) {
        return _inputs[inputIndex_];
    }
    return {};
}

sptr<NodeInput> Node::getInputSlot(string_view uniqueName_) const noexcept {
    for (const auto& input : _inputs) {
        if (input->_uniqueName == uniqueName_) {
            return input;
        }
    }
    return {};
}

u64 Node::countOutputSlots() const noexcept {
    return _outputs.size();
}

sptr<NodeOutput> Node::getOutputSlot(const u8 outputIndex_) const noexcept {
    if (_outputs.size() > outputIndex_) {
        return _outputs[outputIndex_];
    }
    return {};
}

sptr<NodeOutput> Node::getOutputSlot(string_view uniqueName_) const noexcept {
    for (const auto& output : _outputs) {
        if (output->_uniqueName == uniqueName_) {
            return output;
        }
    }
    return {};
}

u64 Node::countParameters() const noexcept {
    return _parameter.size();
}

const non_owning_rptr<NodeParameter> Node::getParameter(const u8 parameterIdx_) const noexcept {
    if (_parameter.size() > parameterIdx_) {
        return _parameter[parameterIdx_].get();
    }
    return nullptr;
}

const non_owning_rptr<NodeParameter> Node::getParameter(string_view uniqueName_) const noexcept {
    for (const auto& parameter : _parameter) {
        if (parameter->getUniqueName() == uniqueName_) {
            return parameter.get();
        }
    }
    return nullptr;
}
