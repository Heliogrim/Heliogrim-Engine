#pragma once

#include "Theme.hpp"

namespace hg::editor::ui {
    class DarkTheme :
        public Theme {
    public:
        DarkTheme();

        ~DarkTheme() noexcept override;

    public:
        void applyButton(cref<sptr<engine::reflow::Button>> button_) const override;

        void applyTextButton(cref<sptr<engine::reflow::Button>> button_) const override;

    public:
        void applyText(cref<sptr<engine::reflow::Text>> text_) const override;

        void applyLabel(cref<sptr<engine::reflow::Text>> text_) const override;

    public:
        void applyIcon72(cref<sptr<engine::reflow::Image>> image_) const override;

        void applyIcon96(cref<sptr<engine::reflow::Image>> image_) const override;
    };
}
