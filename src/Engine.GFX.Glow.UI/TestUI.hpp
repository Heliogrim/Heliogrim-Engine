#pragma once

#include "Engine.Reflow/Widget/Panel.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow/Window/Window.hpp>

namespace ember {
    class Actor;
}

namespace ember::engine::gfx {
    class Device;
}

extern ember::sptr<ember::engine::gfx::Texture> testTexture;

extern ember::wptr<ember::engine::reflow::Widget> testFrameDisplay;
extern ember::wptr<ember::engine::reflow::Widget> testFrameTime;

extern ember::ptr<ember::Actor> editorSelectedTarget;

void storeEditorSelectedTarget(const ember::ptr<ember::Actor> target_);

void storeHierarchyActor(const ember::ptr<ember::Actor> target_);

void testLoad(ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);

ember::sptr<ember::engine::reflow::Window> buildTestUI(
    ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);

ember::ptr<ember::engine::reflow::Font> getDefaultFont();
