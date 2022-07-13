#include "MaterialLoader.hpp"

#include <Engine.Assets/Database/AssetDatabase.hpp>
#include <Engine.Assets/Types/Texture.hpp>
#include <Engine.Common/Collection/CompactArray.hpp>
#include <Engine.Session/Session.hpp>

#include "../Resource/MaterialResource.hpp"
#include "../Resource/TextureResource.hpp"
#include "Engine.Resource/ResourceManager.hpp"

using namespace ember::engine::gfx;
using namespace ember;

MaterialLoader::MaterialLoader(const ptr<cache::GlobalCacheCtrl> cache_) :
    _cacheCtrl(cache_) {}

[[nodiscard]] ptr<TextureResource> resolveTextureResource(
    cref<asset_guid> guid_,
    const ptr<const engine::assets::AssetDatabase> database_,
    const ptr<engine::res::LoaderManager> loader_
) {
    #ifdef _DEBUG
    assert(not not guid_);
    #endif

    /**
     * Resolve texture assets
     */
    const auto query { database_->query(guid_) };
    assert(query.exists());

    auto* const textureAsset { query.get() };

    #ifdef _DEBUG
    if (!textureAsset->getClass()->isExactType<engine::assets::Texture>()) {
        __debugbreak();
    }
    #endif

    /**
     * Load texture assets -> Get resource handler
     */
    auto* const texture { static_cast<const ptr<TextureResource>>(loader_->loadImmediately(textureAsset)) };

    /**
     *
     */
    return texture;
}

MaterialLoader::result_type MaterialLoader::operator()(const ptr<assets::GfxMaterial> asset_, options_type options_) {

    const auto* const db { Session::get()->modules().assetDatabase() };
    auto& loader { Session::get()->modules().resourceManager()->loader() };

    /**
     *
     */
    auto* res { new MaterialResource() };

    /**
     *
     */
    if constexpr (true) {
        res->_payload.diffuse = resolveTextureResource(asset_->diffuse(), db, &loader);
        res->_payload.normal = resolveTextureResource(asset_->normal(), db, &loader);
        res->_payload.roughness = resolveTextureResource(asset_->roughness(), db, &loader);
        res->_payload.ao = resolveTextureResource(asset_->ao(), db, &loader);
    }

    /**
     *
     */

    return res;
}
