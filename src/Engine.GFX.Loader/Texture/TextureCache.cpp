#include "TextureCache.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

TextureCache::TextureCache(const non_owning_rptr<cache::GlobalCacheCtrl> cache_) {}

TextureCache::traits::response_value_type TextureCache::operator()(
    mref<traits::request_value_type> request_,
    mref<traits::request_options_type> options_
) const {
    return {};
}

TextureCache::traits::response_value_type TextureCache::operator()(
    mref<traits::request_value_type> request_,
    mref<traits::request_options_type> options_,
    mref<traits::stream_options_type> streamOptions_
) const {
    return {};
}
