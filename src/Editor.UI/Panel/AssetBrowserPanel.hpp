#pragma once
#include <filesystem>
#include <Engine.Common/Url.hpp>
#include <Engine.Reflow/Widget/__fwd.hpp>
#include <Engine.Reflow/Widget/Panel.hpp>
#include <Engine.Reflow/Widget/HBox.hpp>

namespace ember::editor::ui {
    class AssetBrowser;
}

namespace ember::editor::ui {

    class AssetBrowserPanel :
        public engine::reflow::Panel {
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
        sptr<Widget> _nav;
        sptr<Widget> _items;

    private:
        [[nodiscard]] sptr<engine::reflow::HBox> getNavContainer() const;

        [[nodiscard]] sptr<engine::reflow::HBox> getItemContainer() const;

        void dropNav();

        void buildNav();

        void dropItems();

        void buildItems();

    private:
        wptr<engine::reflow::Popup> _dialog;

    private:
        void closeImportDialog();

        void openImportDialog(cref<Url> fqUrlSource_);

    public:
        engine::reflow::EventResponse onMouseButtonDown(cref<engine::reflow::MouseEvent> event_) override;

        engine::reflow::EventResponse onDrop(cref<engine::reflow::DragDropEvent> event_) override;

    public:
        static sptr<AssetBrowserPanel> make(const non_owning_rptr<AssetBrowser> browser_, cref<Url> root_);
    };

}
