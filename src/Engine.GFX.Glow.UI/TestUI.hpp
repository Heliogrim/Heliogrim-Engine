#pragma once

#include "Engine.Reflow/Widget/Panel.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Loader/Font/FontResource.hpp>

namespace ember {
    class Actor;
    class ActorComponent;
}

namespace ember::engine::gfx {
    class Device;
}

extern ember::smr<ember::engine::gfx::TextureResource> testTexture;
extern ember::smr<ember::engine::gfx::FontResource> testFont;

extern ember::wptr<ember::engine::reflow::Widget> testFrameDisplay;
extern ember::wptr<ember::engine::reflow::Widget> testFrameTime;

extern ember::ptr<void> editorSelectedTarget;

void storeEditorSelectedTarget(const ember::ptr<ember::Actor> target_);

void storeEditorSelectedTarget(const ember::ptr<ember::ActorComponent> target_);

void storeHierarchyActor(ember::cref<ember::Vector<ember::ptr<ember::Actor>>> targets_);

void testLoad(ember::cref<ember::sptr<ember::engine::gfx::Device>> device_);

void destroyLoaded();

void buildTestUI(
    ember::cref<ember::sptr<ember::engine::gfx::Device>> device_,
    const ember::non_owning_rptr<ember::engine::reflow::Window> window_
);

const ember::ptr<ember::engine::reflow::Font> getDefaultFont();
