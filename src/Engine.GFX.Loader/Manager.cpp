#include "Manager.hpp"

#include <Engine.Assets/Types/Texture/Texture.hpp>

#include "Font/FontLoader.hpp"
#include "Geometry/StaticGeometryLoader.hpp"
#include "Material/MaterialLoader.hpp"
#include "Texture/TextureLoader.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

void engine::gfx::loader::register_loader(
    ref<resource::LoaderManager> manager_,
    const non_owning_rptr<Graphics> graphics_,
    const non_owning_rptr<gfx::cache::GlobalCacheCtrl> cacheCtrl_,
    const non_owning_rptr<gfx::pool::GlobalResourcePool> pool_
) {
    {
        auto tl = make_sptr<TextureLoader>(cacheCtrl_, pool_, manager_.sharedSourceLoader());
        manager_.registerLoader<assets::Texture, TextureResource>(tl);
    }

    {
        auto sgl = make_sptr<StaticGeometryLoader>(cacheCtrl_, pool_, manager_.sharedSourceLoader());
        manager_.registerLoader<assets::StaticGeometry, StaticGeometryResource>(sgl);
    }

    {
        auto ml = make_sptr<MaterialLoader>(cacheCtrl_, pool_, manager_.sharedSourceLoader());
        manager_.registerLoader<assets::GfxMaterial, MaterialResource>(ml);
    }

    {
        auto fl = make_sptr<FontLoader>(cacheCtrl_, pool_, manager_.sharedSourceLoader());
        manager_.registerLoader<assets::Font, FontResource>(fl);
    }
}

void engine::gfx::loader::unregister_loader(
    ref<resource::LoaderManager> manager_,
    const non_owning_rptr<Graphics> graphics_,
    const non_owning_rptr<gfx::cache::GlobalCacheCtrl> cacheCtrl_,
    const non_owning_rptr<gfx::pool::GlobalResourcePool> pool_
) {
    manager_.unregisterLoader<assets::Texture>();
    manager_.unregisterLoader<assets::StaticGeometry>();
    manager_.unregisterLoader<assets::GfxMaterial>();
    manager_.unregisterLoader<assets::Font>();
}

void engine::gfx::loader::register_importer(
    ref<res::ImporterManager> manager_,
    const non_owning_rptr<Graphics> graphics_
) {}

void engine::gfx::loader::unregister_importer(
    ref<res::ImporterManager> manager_,
    const non_owning_rptr<Graphics> graphics_
) {}
