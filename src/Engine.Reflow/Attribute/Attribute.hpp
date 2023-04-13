#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::reflow {
    class Widget;
}

namespace hg::engine::reflow {
    template <typename ValueType_>
    class Attribute {
    public:
        using this_type = Attribute<ValueType_>;

        using value_type = ValueType_;

    public:
        template <typename Type_ = ValueType_> requires _STD is_default_constructible_v<Type_>
        Attribute(const non_owning_rptr<const Widget> widget_) :
            _widget(widget_),
            _value() {}

        template <typename Type_ = ValueType_>
        Attribute(const non_owning_rptr<const Widget> widget_, Type_&& value_) :
            _widget(widget_),
            _value(_STD forward<Type_>(value_)) {}

    protected:
        non_owning_rptr<const Widget> _widget;
        value_type _value;

    public:
        [[nodiscard]] ValueType_ getValue() const {
            return _value;
        }

        template <typename Type_ = ValueType_>
        void setValue(Type_&& value_) {
            _value = _STD forward<Type_>(value_);
        }

    public:
        [[nodiscard]] const ptr<const ValueType_> operator->() const {
            return &_value;
        }
    };
}
