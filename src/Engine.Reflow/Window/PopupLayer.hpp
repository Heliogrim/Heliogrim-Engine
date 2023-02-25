#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "__fwd.hpp"
#include "../Widget/__fwd.hpp"

namespace ember::engine::reflow {
    class PopupLayer {
    public:
        using this_type = PopupLayer;

    public:
        PopupLayer(cref<sptr<Window>> window_, cref<sptr<Popup>> popup_);

        ~PopupLayer();

    private:
        sptr<Window> _window;

    public:
        [[nodiscard]] sptr<Window> getWindow() const noexcept;

    private:
        math::vec2 _screenPos;

    public:
        void setScreenPos(cref<math::vec2> screenPos_);

        [[nodiscard]] cref<math::vec2> getScreenPos() const noexcept;

    private:
        math::vec2 _screenSize;

    public:
        void overrideScreenSize(cref<math::vec2> screenSize_);

        [[nodiscard]] bool hasOverrideSize() const noexcept;

        [[nodiscard]] cref<math::vec2> getOverrideSize() const noexcept;

    private:
        sptr<Popup> _content;

    public:
        [[nodiscard]] sptr<Popup> getContent() const noexcept;
    };
}
