#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Vector.hpp>

#include "__fwd.hpp"
#include "../Event/EventDispatcher.hpp"

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

    public:
        [[nodiscard]] sptr<Window> requestWindow(cref<math::uivec2> size_, const wptr<Window> parent_);
    };

}
