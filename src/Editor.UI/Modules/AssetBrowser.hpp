#pragma once

#include <Engine.Common/Collection/StableUnorderedMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "AssetBrowser/AssetBrowserEntry.hpp"

namespace hg::editor::ui {
    class AssetBrowserPanel;
    class AssetBrowserProvider;
}

namespace hg::editor::ui {
    class AssetBrowser final {
    public:
        friend class ::hg::editor::ui::AssetBrowserPanel;

    public:
        using this_type = AssetBrowser;

    public:
        AssetBrowser();

        ~AssetBrowser();

    private:
        Vector<wptr<AssetBrowserPanel>> _panels;

    public:
        [[nodiscard]] sptr<AssetBrowserPanel> makePanel();

    private:
        Vector<uptr<AssetBrowserProvider>> _providers;

    public:
        void addProvider(mref<uptr<AssetBrowserProvider>> provider_);

    protected:
        [[nodiscard]] fs::Url getBrowserRoot() const noexcept;

        [[nodiscard]] fs::Url getImportRoot() const noexcept;

    protected:
        bool retrieveItems(cref<fs::Url> cwd_, _Inout_ ref<Vector<AssetBrowserEntry>> entries_) const;

        void retrieveFromProviders(cref<fs::Url> url_, _Inout_ ref<Vector<AssetBrowserEntry>> entries_) const;

        bool retrieveDirectories(cref<fs::Url> cwd_, _Inout_ ref<Vector<AssetBrowserEntry>> directories_) const;

        void retrieveProviderDirectories(cref<fs::Url> url_, _Inout_ ref<Vector<AssetBrowserEntry>> directories_) const;

        void retrieve(cref<fs::Url> cwd_, _Inout_ ref<Vector<AssetBrowserEntry>> entries_);
    };
}
