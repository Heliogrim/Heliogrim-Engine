#include "Manager.hpp"

#include <Engine.Assets/Types/Texture/Texture.hpp>

#include "Texture/TextureLoader.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

void engine::gfx::loader::register_loader(
    ref<resource::LoaderManager> manager_,
    const non_owning_rptr<Graphics> graphics_,
    const non_owning_rptr<gfx::cache::GlobalCacheCtrl> cacheCtrl_
) {
    auto tl = make_sptr<TextureLoader>(cacheCtrl_);
    manager_.registerLoader<assets::Texture, TextureResource>(tl);
}

void engine::gfx::loader::unregister_loader(
    ref<resource::LoaderManager> manager_,
    const non_owning_rptr<Graphics> graphics_,
    const non_owning_rptr<gfx::cache::GlobalCacheCtrl> cacheCtrl_
) {
    manager_.unregisterLoader<assets::Texture>();
}

void engine::gfx::loader::register_importer(
    ref<res::ImporterManager> manager_,
    const non_owning_rptr<Graphics> graphics_
) {}

void engine::gfx::loader::unregister_importer(
    ref<res::ImporterManager> manager_,
    const non_owning_rptr<Graphics> graphics_
) {}
