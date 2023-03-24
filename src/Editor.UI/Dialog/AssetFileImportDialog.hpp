#pragma once
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Reflow/Widget/Dialog.hpp>

namespace hg::editor::ui {
    class AssetBrowser;
}

namespace hg::editor::ui {
    class AssetFileImportDialog :
        public engine::reflow::Dialog {
    protected:
        AssetFileImportDialog(cref<fs::Url> source_, cref<fs::Url> target_);

    private:
        fs::Url _source;

        fs::Url _target;
        ptr<AssetBrowser> _assetBrowser;

    public:
        static sptr<Dialog> make(const ptr<AssetBrowser> browser_, cref<fs::Url> source_, cref<fs::Url> target_);
    };
}
