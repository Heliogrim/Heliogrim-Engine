#pragma once
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include <Engine.GFX.Glow.UI/Widget/Button.hpp>

namespace ember::editor::ui {

    class AssetBrowserItem :
        public engine::gfx::glow::ui::Button {
    protected:
        AssetBrowserItem();

    public:
        ~AssetBrowserItem() override = default;

    public:
        bool onMouseEnterEvent(cref<math::ivec2> pointer_, bool enter_) override;

    private:
        string _name;
        Url _virtUrl;
        Vector<Url> _fqUrls;

    public:
        [[nodiscard]] static sptr<AssetBrowserItem> make(
            cref<string> name_,
            cref<Url> virtUrl_,
            mref<Vector<Url>> fqUrls_
        );
    };
}
