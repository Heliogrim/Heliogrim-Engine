#pragma once

#include <Engine.Assets/Types/GfxMaterial.hpp>
#include <Engine.Resource/Loader/Loader.hpp>

#include <Engine.GFX/Cache/__fwd.hpp>

namespace ember::engine::gfx {

    class MaterialLoader :
        public res::Loader<assets::GfxMaterial> {
    public:
        using this_type = MaterialLoader;
        using loader_type = res::Loader<assets::GfxMaterial>;

        using result_type = loader_type::result_type;
        using options_type = loader_type::options_type;

    public:
        MaterialLoader(const ptr<cache::GlobalCacheCtrl> cache_);

    private:
        ptr<cache::GlobalCacheCtrl> _cacheCtrl;

    public:
        [[nodiscard]] result_type operator()(const ptr<assets::GfxMaterial> asset_, options_type options_) override;
    };
}
