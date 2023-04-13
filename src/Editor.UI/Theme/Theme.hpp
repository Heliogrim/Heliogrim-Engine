#pragma once

#include <Engine.Common/Wrapper.hpp>

#include <Engine.Reflow/Widget/__fwd.hpp>

namespace hg::editor::ui {
    class Theme {
    public:
        using this_type = Theme;

    public:
        [[nodiscard]] static const non_owning_rptr<const Theme> get();

    protected:
        Theme() = default;

    public:
        virtual ~Theme() noexcept;

    public:
        virtual void applyButton(cref<sptr<engine::reflow::Button>> button_) const = 0;

        virtual void applyTextButton(cref<sptr<engine::reflow::Button>> button_) const = 0;

    public:
        virtual void applyText(cref<sptr<engine::reflow::Text>> text_) const = 0;

        virtual void applyLabel(cref<sptr<engine::reflow::Text>> text_) const = 0;

    public:
        virtual void applyIcon72(cref<sptr<engine::reflow::Image>> image_) const = 0;

        virtual void applyIcon96(cref<sptr<engine::reflow::Image>> image_) const = 0;
    };
}
