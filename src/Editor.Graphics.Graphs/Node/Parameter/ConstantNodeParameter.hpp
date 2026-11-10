#pragma once

#include <cassert>

#include "../NodeParameter.hpp"
#include "../NodeLinkValueType.hpp"
#include "ConstantNodeParameterStorage.hpp"

namespace hg::editor::gfx::graph {
    class ConstantNodeParameter :
        public NodeParameter {
    public:
        using this_type = ConstantNodeParameter;

    public:
        ConstantNodeParameter(string_view uniqueName_, NodeLinkValueType valueType_) :
            NodeParameter(uniqueName_),
            _valueType(valueType_) {}

        ~ConstantNodeParameter() override = default;

    private:
        NodeLinkValueType _valueType;
        ConstantNodeParameterStorage _valueStorage;

    public:
        [[nodiscard]] NodeLinkValueType getValueType() const noexcept;

        template <NodeLinkValueType ValueType_>
        [[nodiscard]] auto getStored() const noexcept {
            assert(ValueType_ == _valueType);
            return std::get<typename ConstantNodeParameterTypeStorage<ValueType_>::type>(_valueStorage);
        }

        template <NodeLinkValueType ValueType_>
        void setStored(typename ConstantNodeParameterTypeStorage<ValueType_>::type&& value_) {
            assert(ValueType_ == _valueType);
            _valueStorage.emplace<>(value_);
        }
    };
}
