#include "StaticGeometryCache.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

StaticGeometryCache::StaticGeometryCache(const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_) :
    Cache(),
    _cacheCtrl(cacheCtrl_) {}

StaticGeometryCache::response_type::type StaticGeometryCache::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(_STD move(request_), _STD move(options_));
}
