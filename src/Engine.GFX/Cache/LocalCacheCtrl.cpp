#include "LocalCacheCtrl.hpp"

#include "LocalResourceCache.hpp"
#include "GlobalResourceCache.hpp"

using namespace ember::engine::gfx::cache;
using namespace ember;

LocalCacheCtrl::LocalCacheCtrl(mref<uptr<LocalResourceCache>> cache_) :
    _cache(_STD move(cache_)) {}

LocalCacheCtrl::~LocalCacheCtrl() {
    tidy();
}

void LocalCacheCtrl::tidy() {

    if (_cache.get()) {
        reset(true);
        _cache.reset();
    }
}

void LocalCacheCtrl::reset(const bool fully_) {

    auto* global { _cache->_global };

    /**
     * Reset all texture marks
     */
    for (auto&& entry : _textures) {
        global->unmark(entry.subject, entry.subResource);
    }
    _textures.clear();

    /**
     * Reset all geometry marks
     */
    #pragma warning(push)
    #pragma warning(disable: 4996)
    for (auto&& entry : _staticGeometries) {
        global->unmark(entry.subject, _STD move(entry.subResource));
    }
    _staticGeometries.clear();
    #pragma warning(pop)

    /**
     * Propagate reset
     */
    _cache->reset(fully_);
}

const non_owning_rptr<const LocalResourceCache> LocalCacheCtrl::cache() const noexcept {
    return _cache.get();
}

const non_owning_rptr<LocalResourceCache> LocalCacheCtrl::cache() noexcept {
    return _cache.get();
}

void LocalCacheCtrl::markAsUsed(const ptr<TextureResource> resource_, mref<TextureSubResource> subResource_) {
    markAsUsed(resource_, AssocKey<TextureSubResource>::from(_STD move(subResource_)));
}

void LocalCacheCtrl::markAsUsed(
    const ptr<TextureResource> resource_,
    cref<AssocKey<TextureSubResource>> subResource_
) {
    // TODO: Check for uniqueness
    _textures.push_back({ resource_, subResource_ });
    _cache->_global->markAsUsed(resource_, subResource_);
}

void LocalCacheCtrl::markAsUsed(const ptr<StaticGeometryResource> resource_,
    mref<StaticGeometrySubResource> subResource_) {

    // TODO: Check for uniqueness
    _staticGeometries.push_back({ resource_, subResource_ });
    #pragma warning(push)
    #pragma warning(disable: 4996)
    _cache->_global->markAsUsed(resource_, _STD move(subResource_));
    #pragma warning(pop)
}
