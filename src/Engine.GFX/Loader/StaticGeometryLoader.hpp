#pragma once
#include <Engine.Assets/Types/StaticGeometry.hpp>
#include <Engine.Resource/Loader/Loader.hpp>

#include "GeometryLoader.hpp"
#include "../Cache/__fwd.hpp"
#include "../Resource/StaticGeometryResource.hpp"

namespace ember::engine::gfx {

    class StaticGeometryLoader :
        public loader::GeometryLoader,
        public res::Loader<assets::StaticGeometry> {
    public:
        using this_type = StaticGeometryLoader;
        using loader_type = res::Loader<assets::StaticGeometry>;

        using result_type = loader_type::result_type;
        using options_type = loader_type::options_type;

    public:
        StaticGeometryLoader(const ptr<cache::GlobalCacheCtrl> cache_);

        ~StaticGeometryLoader() override;

    private:
        ptr<cache::GlobalCacheCtrl> _cacheCtrl;

    public:
        [[nodiscard]] result_type operator()(const ptr<assets::StaticGeometry> asset_, options_type options_) override;

    private:
        void loadWithAssimp(const ptr<assets::StaticGeometry> asset_, const ptr<StaticGeometryResource> dst_);

    };
}
