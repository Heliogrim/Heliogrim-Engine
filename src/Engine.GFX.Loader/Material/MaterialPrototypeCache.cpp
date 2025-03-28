#include "MaterialPrototypeCache.hpp"

#include <Engine.GFX/Cache/GlobalResourceCache.hpp>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>

using namespace hg::engine::gfx::loader;
using namespace hg;

MaterialPrototypeCache::MaterialPrototypeCache(const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_):
    Cache(),
    _cacheCtrl(cacheCtrl_) {}

MaterialPrototypeCache::response_type::type MaterialPrototypeCache::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {

    const asset_guid guid = request_->get_guid();
    const auto query = _cacheCtrl->cache()->query(guid);
    if (query == cache::QueryResultType::eHit) {
        return (smr<resource::ResourceBase> { query.value() }).into<MaterialPrototypeResource>();
    }

    auto prevResponse = next_(std::move(request_), std::move(options_));

    if (not prevResponse.empty()) {
        _cacheCtrl->cache()->store(
            guid,
            smr<MaterialPrototypeResource> { prevResponse }.into<resource::ResourceBase>()
        );
    }

    return prevResponse;
}
