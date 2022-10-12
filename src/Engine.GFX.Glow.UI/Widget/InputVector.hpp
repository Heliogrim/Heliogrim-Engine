#pragma once

#include <concepts>

#include "Input.hpp"

namespace ember::engine::gfx::glow::ui {

    /*
    template <template<typename> typename Type_, typename InputType_>
    concept IsInput = _STD is_base_of_v<Input<InputType_>, Type_<InputType_>>;
     */
    template <typename Type_>
    concept IsInput = _STD is_base_of_v<InputBase, Type_>;

    template <IsInput... InputTypes_>
    class InputVector : public Input<_STD tuple<typename InputTypes_::input_type...>> {
    public:
        using this_type = InputVector<InputTypes_...>;
        using underlying_type = Input<_STD tuple<typename InputTypes_::input_type...>>;

    public:
        void render(const ptr<UICommandBuffer> cmd_) override {}

    public:
        cref<typename underlying_type::input_type> value() const noexcept override {
            return {};
        }
    };

}
