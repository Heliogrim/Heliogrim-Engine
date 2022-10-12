#pragma once
#include <Engine.GFX.Glow.UI/Widget/Panel.hpp>

namespace ember::editor::ui {

    class AssetBrowserPanel :
        public engine::gfx::glow::ui::Panel {
    protected:
        AssetBrowserPanel();

    public:
        static sptr<Panel> make();
    };

}
