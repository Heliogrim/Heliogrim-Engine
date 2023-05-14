#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Platform/NativeWindow.hpp>

#include "PopupLayer.hpp"
#include "WindowType.hpp"
#include "../Children.hpp"
#include "../FocusPath.hpp"
#include "../Attribute/StyleAttribute.hpp"
#include "../Style/WindowStyle.hpp"
#include "../Widget/Widget.hpp"
#include "../Widget/__fwd.hpp"

namespace hg::engine::reflow {
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

    public:
        struct Attributes {
            StyleAttribute<WindowStyle> style;
        } attr;

    private:
        void tidy();

    private:
        ptr<platform::NativeWindow> _nativeWindow;

    private:
        WindowType _type;

        math::vec2 _screenPos;
        math::vec2 _clientSize;

        bool _resizable;
        bool _closeable;

        string _title;

    public:
        void setClientSize(cref<math::vec2> nextClientSize_);

    private:
        FixedChildren<2> _children;

    public:
        void setTitleBar(sptr<Widget> titleBar_);

        void setContent(sptr<Widget> content_);

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

    public:
        math::vec2 prefetchDesiredSize(cref<ReflowState> state_, float scale_) const override;

        math::vec2 computeDesiredSize(cref<ReflowPassState> passState_) const override;

        void applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) override;
    };
}
