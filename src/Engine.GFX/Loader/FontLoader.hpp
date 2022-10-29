#pragma once

#include <Engine.Assets/Types/Font.hpp>
#include <Engine.Resource/Loader/Loader.hpp>

#include "../Cache/__fwd.hpp"

namespace ember::engine::gfx {

    class FontLoader :
        public res::Loader<assets::Font> {
    public:
        using this_type = FontLoader;
        using loader_type = res::Loader<assets::Font>;

        using result_type = loader_type::result_type;
        using options_type = loader_type::options_type;

    public:
        FontLoader(const ptr<cache::GlobalCacheCtrl> cache_);

        ~FontLoader() override;

    private:
        // TODO: Check whether we want font at cache ctrl or just use it atomically
        ptr<cache::GlobalCacheCtrl> _cacheCtrl;

    public:
        [[nodiscard]] result_type operator()(const ptr<assets::Font> asset_, options_type options_) override;

    private:
        ptr<void> __test_load(ptr<::ember::engine::res::LoaderOptions<::ember::engine::assets::Font>> options_);
    };

}