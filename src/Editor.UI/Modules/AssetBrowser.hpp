#pragma once

#include <Engine.Common/Collection/StableUnorderedMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::editor::ui {
    class AssetBrowserPanel;
}

namespace ember::editor::ui {
    class AssetBrowser final {
    public:
        friend class ::ember::editor::ui::AssetBrowserPanel;

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
        StableUnorderedMap<string, Vector<_STD pair<string, Url>>> _virtualMountPoints;

    protected:
        [[nodiscard]] Url getBrowserRoot() const noexcept;

        [[nodiscard]] Url getVirtProtoRoot(cref<string_view> schema_, const u64 virtRootId_);

        [[nodiscard]] Url getImportRoot() const noexcept;

    protected:
        bool retrieveEntriesFQUrl(cref<Url> fqurl_, _Out_ ref<Vector<_STD pair<string, Url>>> entries_) const;

        bool retrieveEntries(cref<Url> cwd_, _Out_ ref<Vector<_STD pair<string, Url>>> entries_) const;
    };
}
