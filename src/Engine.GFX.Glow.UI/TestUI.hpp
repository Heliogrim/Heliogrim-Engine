#pragma once

#include "Widget/Panel.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx {
    class Device;
}

extern ember::sptr<ember::engine::gfx::Texture> testTexture;

extern ember::wptr<ember::engine::gfx::glow::ui::Widget> testFrameDisplay;
extern ember::wptr<ember::engine::gfx::glow::ui::Widget> testFrameTime;

void testLoad(ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);

ember::sptr<ember::engine::gfx::glow::ui::Panel> buildTestUI(
    ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);

ember::ptr<ember::engine::gfx::glow::ui::Font> getDefaultFont();
