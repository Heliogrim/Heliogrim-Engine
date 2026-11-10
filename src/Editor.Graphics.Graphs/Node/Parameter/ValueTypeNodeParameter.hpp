#pragma once

#include "../NodeParameter.hpp"
#include "../NodeLinkValueType.hpp"

namespace hg::editor::gfx::graph {
    class ValueTypeNodeParameter :
        public NodeParameter {
    public:
        using this_type = ValueTypeNodeParameter;

    public:
        ValueTypeNodeParameter(string_view uniqueName_, NodeLinkValueType defaultValueType_) :
            NodeParameter(uniqueName_),
            _valueType(defaultValueType_) {}

        ~ValueTypeNodeParameter() override = default;

    private:
        NodeLinkValueType _valueType;

    public:
        [[nodiscard]] NodeLinkValueType getValueType() const noexcept;

        void setValueType(NodeLinkValueType valueType_);
    };
}
