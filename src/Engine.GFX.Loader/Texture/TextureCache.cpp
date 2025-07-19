#include "TextureCache.hpp"

#include <Engine.Common/Sal.hpp>

#include "Engine.GFX/Cache/GlobalCacheCtrl.hpp"
#include "Engine.GFX/Cache/GlobalResourceCache.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg::engine::gfx;
using namespace hg;

TextureCache::TextureCache(const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_) :
	_cacheCtrl(cacheCtrl_) {}

bool TextureCache::contains(const non_owning_rptr<const assets::TextureAsset> asset_) const noexcept {
	return _cacheCtrl->cache()->contains(asset_);
}

cache::Result<cache::QueryResultType, smr<TextureResource>> TextureCache::query(
	const non_owning_rptr<const assets::TextureAsset> asset_
) const noexcept {
	return cache_result_type { cache::QueryResultType::eMiss, {} };
}

bool TextureCache::store(
	const non_owning_rptr<const assets::TextureAsset> asset_,
	mref<smr<TextureResource>> resource_
) const noexcept {
	_cacheCtrl->cache()->store(asset_->getAssetGuid(), std::move(resource_));
	return true;
}

bool TextureCache::remove(const non_owning_rptr<const assets::TextureAsset> asset_) noexcept {
	return _cacheCtrl->cache()->remove(asset_->getAssetGuid());
}

bool TextureCache::remove(
	const non_owning_rptr<const assets::TextureAsset> asset_,
	ref<smr<TextureResource>> resource_
) noexcept {
	smr<resource::ResourceBase> stored {};
	const auto result = _cacheCtrl->cache()->remove(asset_->getAssetGuid(), stored);

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

	if (result == cache::QueryResultType::eHit) {
		return result.value();
	}

	/**
	 * On cache miss, we need to load the whole resource
	 *  Further we need to clone the cache key to store the responding resource object to the cache
	 */
	cache_key_type cacheKey = request_;
	auto response = next_(std::move(request_), std::move(options_));

	/**
	 * Clone responding object as cache value
	 */
	store(cacheKey, cache_value_type { response });

	return response;
}

TextureCache::stream_response_type::type TextureCache::operator()(
	_In_ mref<stream_request_type::type> request_,
	_In_ mref<stream_request_type::options> options_,
	_In_ cref<next_type> next_
) const {

	/**
	 * On cache hit, we need to check whether the required sub-resource (streaming) is alread present
	 *  If present, we just need to mark the required sub-resource ranges and return the cache response
	 */
	cache::TextureSubResource sub = {};
	if (
		_cacheCtrl->markAsUsed(
			static_cast<non_owning_rptr<const TextureResource>>(request_.get()),
			std::move(sub)
		) == cache::StreamCacheResultType::eResidential
	) {
		//return result.value();
		return;
	}

	/**
	 * If sub-resource is not residential, we need to stream the required data segments
	 *  Due to the fact, that the texture should query the resource itself to check for residential segments
	 *  we are not required to introduce another store invocation, cause the chain will implicitly manipulate the resource.
	 */
	return next_(std::move(request_), std::move(options_));
}

#if FALSE
TextureCache::response_type::type TextureCache::operator()(
    _In_ mref<request_type::type> request_,
    _In_ mref<stream_request_type::options> options_,
    _In_ cref<next_type> next_
) const {

    cache_key_type cacheKey = request_;
    auto result = query(cacheKey);

    if (result == cache::QueryResultType::eMiss) {

        /**
         * On cache miss, we need to loader the whole resource object with stream parameters
         *  Further we need to store the responding resource object to the cache
         */
        auto response = next_(std::move(request_), std::move(options_));

        /**
         * Clone responding object as cache value
         */
        store(cacheKey, cache_value_type { response });

        /**
         * After loading and storing the resource object, we fall back to cach hit routine
         */
        result = query(cacheKey);
    }

    /**
     * On cache hit, we need to check whether the required sub-resource (streaming) is alread present
     *  If present, we just need to mark the required sub-resource ranges and return the cache response
     */
    cache::TextureSubResource sub = {};
    if (_cacheCtrl->markAsUsed(result->get(), std::move(sub)) == cache::StreamCacheResultType::eResidential) {
        return result.value();
    }

    /**
     * If sub-resource is not residential, we need to stream the required data segments
     *  Due to the fact, that the texture should query the resource itself to check for residential segments
     *  we are not required to introduce another store invocation, cause the chain will implicitly manipulate the resource.
     */
    return next_(std::move(request_), std::move(options_));
}
#endif
