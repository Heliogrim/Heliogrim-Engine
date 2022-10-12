#pragma once

#include "Widget/Panel.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {
    class Device;
}

void testLoad(ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);

ember::sptr<ember::engine::gfx::glow::ui::Panel> buildTestUI(
    ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);
