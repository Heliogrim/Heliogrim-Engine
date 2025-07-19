#include "EditorTextureCache.hpp"

#include <Engine.GFX/Cache/CacheTextureSubject.hpp>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Cache/GlobalResourceCache.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

EditorTextureCache::EditorTextureCache(ref<engine::gfx::cache::GlobalCacheCtrl> cacheCtrl_) :
	_cacheCtrl(std::addressof(cacheCtrl_)) {}

EditorTextureCache::~EditorTextureCache() = default;

bool EditorTextureCache::contains(nmpt<const engine::assets::TextureAsset> asset_) const noexcept {
	return _cacheCtrl->cache()->contains(asset_.get());
}

EditorTextureCache::cache_result_type EditorTextureCache::query(
	nmpt<const engine::assets::TextureAsset> asset_
) const noexcept {
	return cache_result_type { engine::gfx::cache::QueryResultType::eMiss, nullptr };
}

bool EditorTextureCache::store(
	nmpt<const engine::assets::TextureAsset> asset_,
	mref<cache_value_type> resource_
) const {
	_cacheCtrl->cache()->store(asset_->getAssetGuid(), std::move(resource_));
	return true;
}

bool EditorTextureCache::remove(nmpt<const engine::assets::TextureAsset> asset_) noexcept {
	return _cacheCtrl->cache()->remove(asset_->getAssetGuid());
}

bool EditorTextureCache::remove(
	nmpt<const engine::assets::TextureAsset> asset_,
	ref<cache_value_type> resource_
) noexcept {

	smr<engine::resource::ResourceBase> stored {};
	const auto result = _cacheCtrl->cache()->remove(asset_->getAssetGuid(), stored);

	if (result) {
		resource_ = stored.into<cache_value_type::value_type>();
	}

	return result;
}

EditorTextureCache::response_type::type EditorTextureCache::operator()(
	mref<request_type::type> request_,
	mref<request_type::options> options_,
	cref<next_type> next_
) const {

	const auto result = query(request_);

	if (result == engine::gfx::cache::QueryResultType::eHit) {
		return result.value();
	}

	/**/

	cache_key_type cacheKey = request_;
	auto response = next_(std::move(request_), std::move(options_));

	/**/

	store(cacheKey, clone(response));

	return response;
}

EditorTextureCache::stream_response_type::type EditorTextureCache::operator()(
	mref<stream_request_type::type> request_,
	mref<stream_request_type::options> options_,
	cref<next_type> next_
) const {

	engine::gfx::cache::TextureSubResource sub {};
	if (
		_cacheCtrl->markAsUsed(static_cast<ptr<const engine::gfx::TextureResource>>(request_.get()), std::move(sub)) ==
		engine::gfx::cache::StreamCacheResultType::eResidential
	) {
		// return result.value();
		return;
	}

	/**/

	return next_(std::move(request_), std::move(options_));
}
