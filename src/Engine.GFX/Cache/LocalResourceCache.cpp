#include "LocalResourceCache.hpp"

#include "CacheResult.hpp"
#include "ModelBatch.hpp"

using namespace hg::engine::gfx::cache;
using namespace hg::engine::gfx;
using namespace hg;

LocalResourceCache::LocalResourceCache(const nmpt<GlobalCacheCtrl> global_) :
    _global(global_),
    _caches(),
    _shifting({ &_caches.front(), &_caches.back() }) {}

LocalResourceCache::LocalResourceCache(mref<this_type> other_) noexcept :
    _global(std::move(other_._global)),
    _caches(std::move(other_._caches)),
    _shifting({ &_caches.front(), &_caches.back() }) {}

LocalResourceCache::~LocalResourceCache() = default;

ref<LocalResourceCache::this_type> LocalResourceCache::operator=(mref<this_type> other_) noexcept {

    if (std::addressof(other_) != this) {
        _caches = std::move(other_._caches);

        if (other_._shifting.front() > other_._shifting.back()) {
            _shifting.front() = &_caches.front();
            _shifting.back() = &_caches.back();
        } else {
            _shifting.front() = &_caches.back();
            _shifting.back() = &_caches.front();
        }
    }

    return *this;
}

void LocalResourceCache::tidy() {
    for (auto& cache : _caches) {
        cache.tidy();
    }
}

void LocalResourceCache::reset() {
    shift();
}

void LocalResourceCache::reset(const bool fully_) {
    if (fully_) {
        return tidy();
    }

    return reset();
}

CacheResult LocalResourceCache::fetch(cref<std::ptrdiff_t> key_, _Out_ ref<ptr<ModelBatch>> dst_) {

    /**
     * Check if current cache has object already stored
     */
    if (_shifting.front()->get(key_, dst_) == CacheResult::eHit) {
        return CacheResult::eHit;
    }

    /**
     * If current primary cache missed, check for transient secondary
     */
    if (_shifting.back()->get(key_, dst_) == CacheResult::eHit) {
        store(key_, ptr<ModelBatch> { dst_ });
        _shifting.back()->remove(key_);

        return CacheResult::eHit;
    }

    return CacheResult::eMiss;
}

void LocalResourceCache::store(cref<std::ptrdiff_t> key_, mref<ptr<ModelBatch>> obj_) {
    _shifting.front()->insert(key_, std::move(obj_));
}

void LocalResourceCache::shift() {

    /**
     * Swap primary and secondary
     */
    std::swap(_shifting.front(), _shifting.back());

    /**
     * Erase previous state of primary cache (new secondary can be used to fetch last invocation states)
     */
    _shifting.front()->tidy();
}
