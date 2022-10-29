#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/__macro.hpp>

#include "Widget.hpp"

namespace ember::engine::gfx::glow::ui {

    class __declspec(novtable) InputBase : public StatefulWidget {
    };

    template <typename InputType_>
    class __declspec(novtable) Input : public InputBase {

    public:
        using this_type = Input<InputType_>;

        using input_type = _STD decay_t<InputType_>;

    public:
        [[nodiscard]] virtual cref<input_type> value() const noexcept = 0;
    };

}
