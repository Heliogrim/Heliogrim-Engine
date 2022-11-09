#pragma once

#include "Engine.Reflow/Widget/Panel.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow/Window/Window.hpp>

namespace ember::engine::gfx {
    class Device;
}

extern ember::sptr<ember::engine::gfx::Texture> testTexture;

extern ember::wptr<ember::engine::reflow::Widget> testFrameDisplay;
extern ember::wptr<ember::engine::reflow::Widget> testFrameTime;

void testLoad(ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);

ember::sptr<ember::engine::reflow::Window> buildTestUI(
    ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);

ember::ptr<ember::engine::reflow::Font> getDefaultFont();
