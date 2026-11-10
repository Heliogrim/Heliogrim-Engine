#include "ValueTypeNodeParameter.hpp"

using namespace hg::editor::graphics::graph;
using namespace hg;

NodeLinkValueType ValueTypeNodeParameter::getValueType() const noexcept {
    return _valueType;
}

void ValueTypeNodeParameter::setValueType(NodeLinkValueType valueType_) {
    _valueType = valueType_;
}
