#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Widget/Widget.hpp"

namespace hg::engine::reflow {
    class Children :
        public Vector<sptr<Widget>> {};
}
