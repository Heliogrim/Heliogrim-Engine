#pragma once
#include <filesystem>
#include <Engine.Common/Url.hpp>
#include <Engine.GFX.Glow.UI/Widget/Panel.hpp>

namespace ember::editor::ui {
    class AssetBrowser;
}

namespace ember::editor::ui {

    class AssetBrowserPanel :
        public engine::gfx::glow::ui::Panel {
    protected:
        AssetBrowserPanel();

    public:
        ~AssetBrowserPanel() override;

        //private:
    public:
        non_owning_rptr<AssetBrowser> _browser;

        Url _browserRoot;
        Url _browserCwd;

    public:
        void changeCwd(cref<Url> nextCwd_);

    private:
        [[nodiscard]] cref<sptr<Widget>> getNavContainer() const;

        [[nodiscard]] cref<sptr<Widget>> getItemContainer() const;

        void dropNav();

        void buildNav();

        void dropItems();

        void buildItems();

    private:
        wptr<Widget> _dialog;

    private:
        void closeImportDialog();

        void openImportDialog(cref<Url> fqUrlSource_);

    public:
        bool onDragDropEvent(cref<engine::input::event::DragDropEvent> event_) override;

    public:
        static sptr<AssetBrowserPanel> make(const non_owning_rptr<AssetBrowser> browser_, cref<Url> root_);
    };

}
