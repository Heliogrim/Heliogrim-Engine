#include "MaterialCache.hpp"

#include <Engine.GFX/Cache/GlobalResourceCache.hpp>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>

using namespace hg::engine::gfx::loader;
using namespace hg;

MaterialCache::MaterialCache(const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_):
    Cache(),
    _cacheCtrl(cacheCtrl_) {}

MaterialCache::response_type::type MaterialCache::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    const asset_guid guid = request_->get_guid();
    const auto query = _cacheCtrl->cache()->query(guid);
    if (query == cache::QueryResultType::eHit) {
        return (smr<resource::ResourceBase> { query.value() }).into<MaterialResource>();
    }

    auto prevResponse = next_(_STD move(request_), _STD move(options_));

    if (not prevResponse.empty()) {
        _cacheCtrl->cache()->store(guid, smr<MaterialResource> { prevResponse }.into<resource::ResourceBase>());
    }

    return prevResponse;
}
