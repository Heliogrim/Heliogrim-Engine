#pragma once

#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Session/Window.hpp>

#include "../Style/__fwd.hpp"
#include "../Widget/__fwd.hpp"
#include "../Widget/Widget.hpp"
#include "../Children.hpp"
#include "../FocusPath.hpp"
#include "WindowType.hpp"
#include "PopupLayer.hpp"

namespace ember::engine::reflow {

    class Window :
        public Widget {
    public:
        friend class EventDispatcher;

    public:
        using this_type = Window;

    public:
        Window();

        ~Window() override;

    public:
        [[nodiscard]] string getTag() const noexcept override;

    private:
        void tidy();

    private:
        WindowType _type;

        math::vec2 _screenPos;
        math::vec2 _clientSize;

        bool _resizable;
        bool _closeable;

        string _title;

    private:
        sptr<Widget> _titleBar;
        sptr<Widget> _content;

        Children _children;

        sptr<BoundStyleSheet> _style;

    private:
        void repackChildren();

    public:
        void setTitleBar(sptr<Widget> titleBar_);

        void setContent(sptr<Widget> content_);

    private:
        ptr<session::Window> _nativeWindow;

    public:
        [[nodiscard]] sptr<Widget> root() const override;

        [[nodiscard]] const ptr<const Children> children() const override;

    private:
        Vector<uptr<PopupLayer>> _popupLayers;

    public:
        non_owning_rptr<PopupLayer> pushPopLayer(cref<sptr<Popup>> popup_);

        void dropPopLayer(const non_owning_rptr<PopupLayer> layer_ = nullptr);

    private:
        FocusPath _focus;

    public:
        [[nodiscard]] cref<FocusPath> getFocusPath() const noexcept;

        [[nodiscard]] sptr<Widget> getFocusTarget() const noexcept;

    public:
        void render(const ptr<ReflowCommandBuffer> cmd_) override;

        void flow(cref<FlowContext> ctx_, cref<math::vec2> space_, cref<math::vec2> limit_, ref<StyleKeyStack> styleStack_) override;

        void shift(cref<FlowContext> ctx_, cref<math::vec2> offset_) override;

    public:
        [[nodiscard]] math::vec2 outerSize() const noexcept override;

        [[nodiscard]] math::vec2 innerSize() const noexcept override;
    };

}
