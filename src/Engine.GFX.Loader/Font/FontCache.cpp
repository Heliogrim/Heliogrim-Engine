#include "FontCache.hpp"

#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Cache/GlobalResourceCache.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::loader;
using namespace hg;

FontCache::FontCache(ref<cache::GlobalCacheCtrl> cacheCtrl_):
	Cache(),
	_cacheCtrl(std::addressof(cacheCtrl_)) {}

FontCache::response_type::type FontCache::operator()(
	mref<request_type::type> request_,
	mref<request_type::options> options_,
	cref<next_type> next_
) const {

	const auto guid = request_->get_guid();
	const auto query = _cacheCtrl->cache()->query(guid);
	if (query == cache::QueryResultType::eHit) {
		return query.value().into<resource::Resource<reflow::Font>>();
	}

	/**/

	auto response = next_(std::move(request_), std::move(options_));
	const auto store = _cacheCtrl->cache()->store(guid, clone(response));

	return std::move(response);
}
