#include "TextureCache.hpp"

#include "Engine.GFX/Cache/GlobalCacheCtrl.hpp"
#include "Engine.GFX/Cache/GlobalResourceCache.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember::engine::gfx;
using namespace ember;

TextureCache::TextureCache(const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_) :
    _cacheCtrl(cacheCtrl_) {}

bool TextureCache::contains(const non_owning_rptr<const assets::Texture> asset_) const noexcept {
    return _cacheCtrl->cache()->contains(asset_);
}

cache::Result<cache::CacheResultType, smr<TextureResource>> TextureCache::query(
    const non_owning_rptr<const assets::Texture> asset_
) const noexcept {
    return cache_result_type { cache::CacheResultType::eMiss, {} };
}

bool TextureCache::store(
    const non_owning_rptr<const assets::Texture> asset_,
    mref<smr<TextureResource>> resource_
) noexcept {
    _cacheCtrl->cache()->store(asset_->get_guid(), _STD move(resource_));
    return true;
}

bool TextureCache::remove(const non_owning_rptr<const assets::Texture> asset_) noexcept {
    return _cacheCtrl->cache()->remove(asset_->get_guid());
}

bool TextureCache::remove(
    const non_owning_rptr<const assets::Texture> asset_,
    ref<smr<TextureResource>> resource_
) noexcept {
    smr<resource::ResourceBase> stored {};
    const auto result = _cacheCtrl->cache()->remove(asset_->get_guid(), stored);

    if (result) {
        resource_ = stored.into<TextureResource>();
    }

    return result;
}

void TextureCache::clear() {}

TextureCache::response_type::type TextureCache::operator()(
    _In_ mref<request_type::type> request_,
    _In_ mref<request_type::options> options_,
    _In_ cref<next_type> next_
) const {

    const auto result = query(request_);

    if (result == cache::CacheResultType::eHit) {
        return result.value();
    }

    /**/

    auto response = next_(_STD move(request_), _STD move(options_));
    const_cast<ptr<TextureCache>>(this)->store(request_, smr<TextureResource> { response });

    return response;
}

TextureCache::response_type::type TextureCache::operator()(
    _In_ mref<request_type::type> request_,
    _In_ mref<request_type::options> options_,
    _In_ mref<request_type::stream> streamOptions_,
    _In_ cref<next_type> next_
) const {
    return next_(_STD move(request_), _STD move(options_), _STD move(streamOptions_));
}
