#include "FontCache.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

FontCache::FontCache(const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_):
    Cache(),
    _cacheCtrl(cacheCtrl_) {}

FontCache::response_type::type FontCache::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(_STD move(request_), _STD move(options_));
}
