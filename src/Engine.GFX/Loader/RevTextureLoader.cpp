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

    //
    assert(res != nullptr);
    res->_payload.view = TextureLoader::loadTo(baseImage->sources().front(), _STD move(res->_payload.view));

    return res;
}
