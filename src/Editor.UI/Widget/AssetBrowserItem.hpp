#pragma once
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include <Engine.Reflow/Widget/Button.hpp>

namespace ember::editor::ui {
    class AssetBrowserPanel;
}

namespace ember::editor::ui {
    class AssetBrowserItem :
        public engine::reflow::Button {
    protected:
        AssetBrowserItem();

    public:
        ~AssetBrowserItem() override = default;

    private:
        string _name;
        Url _virtUrl;
        Vector<Url> _fqUrls;

    public:
        [[nodiscard]] static sptr<AssetBrowserItem> make(
            cref<sptr<AssetBrowserPanel>> panel_,
            cref<string> name_,
            cref<Url> virtUrl_,
            mref<Vector<Url>> fqUrls_
        );
    };
}
