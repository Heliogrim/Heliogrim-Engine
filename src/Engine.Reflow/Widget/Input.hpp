#pragma once

#include "Widget.hpp"

namespace ember::engine::reflow {

    namespace {
        class __declspec(novtable) InputBase : public Widget {};
    };

    template <typename InputType_>
    class Input :
        public InputBase {
    public:
        using this_type = Input<InputType_>;
        using input_type = _STD decay_t<InputType_>;

    protected:
        Input() = default;

    public:
        ~Input() override = default;

    public:
        [[nodiscard]] virtual input_type value() const noexcept = 0;
    };

}
