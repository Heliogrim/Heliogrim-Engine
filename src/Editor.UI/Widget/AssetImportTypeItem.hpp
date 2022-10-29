#pragma once
#include <Engine.GFX.Glow.UI/Widget/Button.hpp>

namespace ember::editor::ui {

    class AssetImportTypeItem :
        public engine::gfx::glow::ui::Button {
    public:
        AssetImportTypeItem() = default;

        ~AssetImportTypeItem() override = default;
    };
}
