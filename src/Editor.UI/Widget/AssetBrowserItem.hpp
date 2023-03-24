#pragma once
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include <Engine.Reflow/Widget/Button.hpp>
#include "../Modules/AssetBrowser/AssetBrowserEntry.hpp"

namespace hg::editor::ui {
    class AssetBrowserPanel;
}

namespace hg::editor::ui {
    class AssetBrowserItem :
        public engine::reflow::Button {
    protected:
        AssetBrowserItem();

    public:
        ~AssetBrowserItem() override = default;

    private:
        AssetBrowserEntry _value;

    public:
        [[nodiscard]] static sptr<AssetBrowserItem> make(
            cref<sptr<AssetBrowserPanel>> panel_,
            mref<AssetBrowserEntry> value_
        );
    };
}
