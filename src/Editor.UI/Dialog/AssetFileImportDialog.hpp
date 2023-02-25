#pragma once
#include <Engine.Common/Url.hpp>
#include <Engine.Reflow/Widget/Dialog.hpp>

namespace ember::editor::ui {
    class AssetBrowser;
}

namespace ember::editor::ui {
    class AssetFileImportDialog :
        public engine::reflow::Dialog {
    protected:
        AssetFileImportDialog(cref<Url> source_, cref<Url> target_);

    private:
        Url _source;

        Url _target;
        ptr<AssetBrowser> _assetBrowser;

    public:
        static sptr<Dialog> make(const ptr<AssetBrowser> browser_, cref<Url> source_, cref<Url> target_);
    };
}
