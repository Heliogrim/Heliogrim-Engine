#include "MaterialCache.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

MaterialCache::MaterialCache(const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_):
    Cache(),
    _cacheCtrl(nullptr) {}

MaterialCache::response_type::type MaterialCache::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(_STD move(request_), _STD move(options_));
}
