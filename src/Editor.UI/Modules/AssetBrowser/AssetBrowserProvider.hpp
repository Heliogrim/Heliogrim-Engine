#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Url.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "AssetBrowserEntry.hpp"

namespace hg::editor::ui {
    class AssetBrowserProvider {
    public:
        using this_type = AssetBrowserProvider;

    public:
        virtual ~AssetBrowserProvider() = default;

    public:
        _Success_(return != nullptr) virtual non_owning_rptr<this_type> createProvider(cref<Url> url_) = 0;

        virtual non_owning_rptr<this_type> destroyProvider(cref<Url> url_) = 0;

    public:
        virtual bool retrieve(cref<Url> url_, _Out_ ref<Vector<AssetBrowserEntry>> entries_);
    };
}
