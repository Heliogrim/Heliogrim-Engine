#pragma once

#include "ReflowContainer.hpp"

#include "../Event/FormSubmitEvent.hpp"

namespace ember::engine::gfx::glow::ui {

    class Form : public ReflowContainer {
    public:
        using this_type = Form;

    public:
        Form();

        ~Form() override;

    public:
        void submitForm();

        void clearForm();

    public:
        [[nodiscard]] u64 onSubmit(mref<_STD function<void(cref<event::FormSubmitEvent> event_)>> listener_);
    };

}
