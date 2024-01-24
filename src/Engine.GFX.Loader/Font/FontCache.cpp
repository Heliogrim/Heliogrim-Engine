#include "FontCache.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

FontCache::FontCache(const non_owning_rptr<cache::GlobalCacheCtrl> cacheCtrl_):
    Cache(),
    _cacheCtrl(cacheCtrl_) {}

FontCache::response_type::type FontCache::operator()(
    mref<request_type::type> request_,
    mref<request_type::options> options_,
    cref<next_type> next_
) const {
    return next_(std::move(request_), std::move(options_));
}
