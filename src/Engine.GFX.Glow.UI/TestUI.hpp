#pragma once

#include "Engine.Reflow/Widget/Panel.hpp"
#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Loader/Font/FontResource.hpp>

namespace hg {
    class Actor;
    class ActorComponent;
}

namespace hg::engine::gfx {
    class Device;
}

extern hg::smr<hg::engine::gfx::TextureResource> testTexture;
extern hg::smr<hg::engine::gfx::FontResource> testFont;

extern hg::wptr<hg::engine::reflow::Widget> testFrameDisplay;
extern hg::wptr<hg::engine::reflow::Widget> testFrameTime;

extern hg::ptr<void> editorSelectedTarget;

void storeEditorSelectedTarget(const hg::ptr<hg::Actor> target_);

void storeEditorSelectedTarget(const hg::ptr<hg::ActorComponent> target_);

void storeHierarchyActor(hg::cref<hg::Vector<hg::ptr<hg::Actor>>> targets_);

void testLoad(hg::cref<hg::sptr<hg::engine::gfx::Device>> device_);

void destroyLoaded();

void buildTestUI(
    hg::cref<hg::sptr<hg::engine::gfx::Device>> device_,
    const hg::non_owning_rptr<hg::engine::reflow::Window> window_
);

const hg::ptr<hg::engine::reflow::Font> getDefaultFont();
