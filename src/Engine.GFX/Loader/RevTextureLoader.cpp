#include "RevTextureLoader.hpp"

#include <Engine.Session/Session.hpp>

#include "Engine.Assets/Database/AssetDatabase.hpp"
#include "Engine.Assets/Types/Image.hpp"

#include "../Cache/GlobalCacheCtrl.hpp"
#include "../Cache/GlobalResourceCache.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevTextureLoader::RevTextureLoader(const ptr<cache::GlobalCacheCtrl> cache_) :
    TextureLoader(cache_) {}

RevTextureLoader::result_type RevTextureLoader::operator()(const ptr<assets::Texture> asset_, options_type options_) {

    const auto* const db { Session::get()->modules().assetDatabase() };

    // TODO: Rewrite
    const asset_guid baseImageGuid { asset_->baseImage() };

    const auto query { db->query(baseImageGuid) };
    assert(query.exists());

    const auto* const baseImage { static_cast<ptr<ember::engine::assets::Image>>(query.get()) };
    assert(!baseImage->sources().empty());

    /**
     *
     */
    auto* res { _cacheCtrl->cache()->request(asset_) };

    if (not res->hasStreamLoader()) {
        res->setStreamLoader(weak_from_this());
    }

    //
    assert(res != nullptr);

    /**/
    const auto isDefault { baseImage->sources().front().path().contains("default"sv) };
    /**/

    if (!res->_payload.__pseudo_stored) {

        if (isDefault) {

            constexpr loader::TextureLoaderFlags flags { loader::TextureLoaderFlagBits::eLockLoaded };

            res->_payload.view = TextureLoader::loadTo(baseImage->sources().front(), _STD move(res->_payload.view), flags);
            res->_payload.__pseudo_stored = true;
            
        } else {

            constexpr loader::TextureLoaderFlags flags { loader::TextureLoaderFlagBits::eLazyDataLoading };

            res->_payload.view = TextureLoader::loadTo(baseImage->sources().front(), _STD move(res->_payload.view), flags);
            res->_payload.__pseudo_stored = true;
        }

    }

    return res;
}

void RevTextureLoader::streamLoad(const ptr<res::partial::Streamable<res::Resource>> resource_,
    stream_options_type options_) {

    const ptr<TextureResource> texture { static_cast<const ptr<TextureResource>>(resource_) };
    const auto* const asset { static_cast<const non_owning_rptr<const assets::Texture>>(resource_->origin()) };
    const auto* const db { Session::get()->modules().assetDatabase() };

    /**/
    if (texture->_payload.view->width() == 1ui32) {
        // Warning: Drop stream load operations on default textures
        return;
    }
    /**/

    // TODO: Rewrite
    const asset_guid baseImageGuid { asset->baseImage() };

    const auto query { db->query(baseImageGuid) };
    assert(query.exists());

    const auto* const baseImage { static_cast<ptr<ember::engine::assets::Image>>(query.get()) };
    assert(!baseImage->sources().empty());

    /**
     *
     */
    TextureLoader::partialLoadTo(baseImage->sources().front(), options_, texture->_payload.view.get());
}

void RevTextureLoader::streamUnload(const ptr<res::partial::Streamable<res::Resource>> resource_,
    stream_options_type options_) {

    const ptr<TextureResource> texture { static_cast<const ptr<TextureResource>>(resource_) };

    /**
     *
     */
    //TextureLoader::partialUnload(options_, texture->_payload.view.get());
}
