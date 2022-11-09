#pragma once
#include <Engine.Reflow/Widget/Button.hpp>

namespace ember::editor::ui {

    class AssetImportTypeItem :
        public engine::reflow::Button {
    public:
        AssetImportTypeItem();

        ~AssetImportTypeItem() override = default;
    };
}
