#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "__fwd.hpp"
#include "../Event/EventDispatcher.hpp"

namespace ember::engine::scene {
    class IRenderScene;
}

namespace ember::engine::reflow {
    class WindowManager :
        public EventDispatcher {
    public:
        using this_type = WindowManager;

    public:
        [[nodiscard]] static cref<sptr<this_type>> get();

        static cref<sptr<this_type>> make();

        static void destroy();

    protected:
        /**
         * Singleton Instance
         */
        static sptr<WindowManager> _instance;

        WindowManager();

    public:
        ~WindowManager();

    private:
        void setup();

        void tidy();

    private:
        Vector<uptr<BoundWindow>> _windows;

    private:
        sptr<Window> resolveWindow(cref<math::ivec2> position_) const noexcept;

        [[nodiscard]] non_owning_rptr<scene::IRenderScene> resolveScene(cref<sptr<Window>> window_);

        void handleWindowResize(const ptr<BoundWindow> wnd_, cref<math::ivec2> nextSize_) const;

    private:
        Vector<wptr<Window>> _focusOrder;

    private:
        void interceptFocusEvent(cref<sptr<Window>> window_, cref<FocusEvent> event_) override;

    public:
        void destroyWindow(mref<sptr<Window>> window_);

        [[nodiscard]] sptr<Window> requestWindow(
            string_view title_,
            cref<math::ivec2> size_,
            const wptr<Window> parent_,
            const non_owning_rptr<scene::IRenderScene> scene_ = nullptr);
    };
}
